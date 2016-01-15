#include <fstream>
#include <iostream>
#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/MySql/MySqlConfig.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <tclap/CmdLine.h>
#include "MarketDataServer/Version.hpp"
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/BufferedHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataFeedServlet.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"
#include "Nexus/MarketDataService/MySqlHistoricalDataStore.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::MySql;
using namespace Beam::Network;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace std;
using namespace TCLAP;

namespace {
  using RegistryServletContainer =
    ServiceProtocolServletContainer<MetaAuthenticationServletAdapter<
    MetaMarketDataRegistryServlet<MarketDataRegistry*,
    BufferedHistoricalDataStore<MySqlHistoricalDataStore*>,
    ApplicationServiceLocatorClient::Client>,
    ApplicationServiceLocatorClient::Client*, NativePointerPolicy>,
    TcpServerSocket, BinarySender<SharedBuffer>, NullEncoder,
    std::shared_ptr<LiveTimer>>;
  using BaseRegistryServlet =
    MarketDataRegistryServlet<RegistryServletContainer, MarketDataRegistry*,
    BufferedHistoricalDataStore<MySqlHistoricalDataStore*>,
    ApplicationServiceLocatorClient::Client>;
  using FeedServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<
    MetaMarketDataFeedServlet<BaseRegistryServlet*>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, SizeDeclarativeEncoder<ZLibEncoder>,
    std::shared_ptr<LiveTimer>>;

  struct RegistryServerConnectionInitializer {
    string m_serviceName;
    IpAddress m_interface;
    vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  struct FeedServerConnectionInitializer {
    string m_serviceName;
    IpAddress m_interface;
    vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void RegistryServerConnectionInitializer::Initialize(
      const YAML::Node& config) {
    m_serviceName = Extract<string>(config, "service",
      MarketDataService::REGISTRY_SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    vector<IpAddress> addresses;
    addresses.push_back(m_interface);
    m_addresses = Extract<vector<IpAddress>>(config, "addresses", addresses);
  }

  void FeedServerConnectionInitializer::Initialize(
      const YAML::Node& config) {
    m_serviceName = Extract<string>(config, "service",
      MarketDataService::FEED_SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    vector<IpAddress> addresses;
    addresses.push_back(m_interface);
    m_addresses = Extract<vector<IpAddress>>(config, "addresses", addresses);
  }

  void PopulateRegistrySecurityInfo(Out<MarketDataRegistry> registry,
      const MarketDatabase& marketDatabase) {
    YAML::Node config;
    try {
      ifstream symbolStream{"symbols"};
      if(!symbolStream.good()) {
        cerr << "symbols not found" << endl;
        return;
      }
      YAML::Parser configParser{symbolStream};
      configParser.GetNextDocument(config);
    } catch(const YAML::ParserException& e) {
      cerr << "Invalid symbol at line " << (e.mark.line + 1) << ", " <<
        "column " << (e.mark.column + 1) << ": " << e.msg << endl;
      return;
    }
    auto symbols = config.FindValue("symbols");
    if(symbols == nullptr) {
      return;
    }
    for(auto& entry : *symbols) {
      auto symbol = Extract<string>(entry, "symbol");
      auto name = Extract<string>(entry, "name");
      auto security = ParseSecurity(symbol, marketDatabase);
      SecurityInfo securityInfo{security, name, ""};
      registry->Add(securityInfo);
    }
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "0.9-r" MARKET_DATA_SERVER_VERSION
      "\nCopyright (C) 2009 Eidolon Systems Ltd."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  YAML::Node config;
  try {
    ifstream configStream{configFile.c_str()};
    if(!configStream.good()) {
      cerr << configFile << " not found." << endl;
      return -1;
    }
    YAML::Parser configParser{configStream};
    configParser.GetNextDocument(config);
  } catch(const YAML::ParserException& e) {
    cerr << "Invalid YAML at line " << (e.mark.line + 1) << ", " << "column " <<
      (e.mark.column + 1) << ": " << e.msg << endl;
    return -1;
  }
  RegistryServerConnectionInitializer registryServerConnectionInitializer;
  try {
    registryServerConnectionInitializer.Initialize(
      GetNode(config, "registry_server"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'registry_server': " << e.what() << endl;
    return -1;
  }
  FeedServerConnectionInitializer feedServerConnectionInitializer;
  try {
    feedServerConnectionInitializer.Initialize(GetNode(config, "feed_server"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'feed_server': " << e.what() << endl;
    return -1;
  }
  ServiceLocatorClientConfig serviceLocatorClientConfig;
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      GetNode(config, "service_locator"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'service_locator': " << e.what() << endl;
    return -1;
  }
  SocketThreadPool socketThreadPool;
  TimerThreadPool timerThreadPool;
  ThreadPool threadPool;
  ApplicationServiceLocatorClient serviceLocatorClient;
  try {
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_address,
      Ref(socketThreadPool), Ref(timerThreadPool));
    serviceLocatorClient->SetCredentials(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password);
    serviceLocatorClient->Open();
  } catch(const std::exception& e) {
    cerr << "Error logging in: " << e.what() << endl;
    return -1;
  }
  ApplicationDefinitionsClient definitionsClient;
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the definitions service." << endl;
    return -1;
  }
  ApplicationAdministrationClient administrationClient;
  try {
    administrationClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    administrationClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the administration service." << endl;
    return -1;
  }
  MySqlConfig mySqlConfig;
  try {
    mySqlConfig = MySqlConfig::Parse(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'data_store': " << e.what() << endl;
    return -1;
  }
  MySqlHistoricalDataStore historicalDataStore{mySqlConfig.m_address,
    mySqlConfig.m_schema, mySqlConfig.m_username, mySqlConfig.m_password};
  MarketDataRegistry marketDataRegistry;
  optional<BaseRegistryServlet> baseRegistryServlet;
  try {
    auto entitlements = administrationClient->LoadEntitlements();
    PopulateRegistrySecurityInfo(Store(marketDataRegistry),
      definitionsClient->LoadMarketDatabase());
    auto cacheBlockSize = Extract<int>(config, "cache_block_size", 10000);
    auto historicalBufferSize = static_cast<size_t>(Extract<int>(config,
      "historical_buffer_size", 100000));
    auto threadCount = static_cast<size_t>(Extract<int>(config,
      "database_threads", boost::thread::hardware_concurrency()));
    baseRegistryServlet.emplace(entitlements, Ref(*serviceLocatorClient),
      &marketDataRegistry, Initialize(&historicalDataStore,
      historicalBufferSize, Ref(threadPool)));
  } catch(const std::exception& e) {
    cerr << "Error initializing server: " << e.what() << endl;
    return -1;
  }
  RegistryServletContainer registryServer{Initialize(serviceLocatorClient.Get(),
    baseRegistryServlet.get_ptr()), Initialize(
    registryServerConnectionInitializer.m_interface, Ref(socketThreadPool)),
    std::bind(factory<std::shared_ptr<LiveTimer>>{}, seconds{10},
    Ref(timerThreadPool))};
  try {
    registryServer.Open();
  } catch(const std::exception& e) {
    cerr << "Error opening server: " << e.what() << endl;
    return -1;
  }
  try {
    JsonObject registryService;
    registryService["addresses"] =
      ToString(registryServerConnectionInitializer.m_addresses);
    serviceLocatorClient->Register(
      registryServerConnectionInitializer.m_serviceName, registryService);
  } catch(const std::exception& e) {
    cerr << "Error registering service: " << e.what() << endl;
    return -1;
  }
  FeedServletContainer feedServer{Initialize(serviceLocatorClient.Get(),
    baseRegistryServlet.get_ptr()), Initialize(
    feedServerConnectionInitializer.m_interface, Ref(socketThreadPool)),
    std::bind(factory<std::shared_ptr<LiveTimer>>{}, seconds{10},
    Ref(timerThreadPool))};
  try {
    feedServer.Open();
  } catch(const std::exception& e) {
    cerr << "Error opening server: " << e.what() << endl;
    return -1;
  }
  try {
    JsonObject feedService;
    feedService["addresses"] =
      ToString(feedServerConnectionInitializer.m_addresses);
    serviceLocatorClient->Register(
      feedServerConnectionInitializer.m_serviceName, feedService);
  } catch(const std::exception& e) {
    cerr << "Error registering service: " << e.what() << endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
