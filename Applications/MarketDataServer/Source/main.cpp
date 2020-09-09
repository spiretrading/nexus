#include <fstream>
#include <iostream>
#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Sql/MySqlConfig.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
#include <Viper/MySql/Connection.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/AsyncHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataFeedServlet.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"
#include "Nexus/MarketDataService/SessionCachedHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/SqlHistoricalDataStore.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
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
using namespace TCLAP;
using namespace Viper;

namespace {
  using SqlDataStore = SqlHistoricalDataStore<MySql::Connection>;
  using RegistryServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaMarketDataRegistryServlet<
    MarketDataRegistry*, SessionCachedHistoricalDataStore<
    AsyncHistoricalDataStore<SqlDataStore*>*>,
    ApplicationAdministrationClient::Client*>,
    ApplicationServiceLocatorClient::Client*, NativePointerPolicy>,
    TcpServerSocket, BinarySender<SharedBuffer>, NullEncoder,
    std::shared_ptr<LiveTimer>>;
  using BaseRegistryServlet = MarketDataRegistryServlet<
    RegistryServletContainer, MarketDataRegistry*,
    SessionCachedHistoricalDataStore<AsyncHistoricalDataStore<SqlDataStore*>*>,
    ApplicationAdministrationClient::Client*>;
  using FeedServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<
    MetaMarketDataFeedServlet<BaseRegistryServlet*>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, SizeDeclarativeEncoder<ZLibEncoder>,
    std::shared_ptr<LiveTimer>>;

  JsonValue ParseCountries(const YAML::Node& config,
      const CountryDatabase& countryDatabase) {
    auto countries = std::vector<JsonValue>();
    for(auto item : config) {
      auto country = ParseCountryCode(item.as<std::string>(), countryDatabase);
      countries.push_back(static_cast<double>(
        static_cast<std::uint16_t>(country)));
    }
    if(countries.empty()) {
      return JsonNull();
    }
    return countries;
  }

  struct RegistryServerConnectionInitializer {
    std::string m_serviceName;
    IpAddress m_interface;
    std::vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config,
      const CountryDatabase& countryDatabase);
  };

  struct FeedServerConnectionInitializer {
    std::string m_serviceName;
    IpAddress m_interface;
    std::vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void RegistryServerConnectionInitializer::Initialize(const YAML::Node& config,
      const CountryDatabase& countryDatabase) {
    m_serviceName = Extract<std::string>(config, "service",
      MarketDataService::REGISTRY_SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    auto addresses = std::vector<IpAddress>();
    addresses.push_back(m_interface);
    m_addresses = Extract<std::vector<IpAddress>>(config, "addresses",
      addresses);
  }

  void FeedServerConnectionInitializer::Initialize(const YAML::Node& config) {
    m_serviceName = Extract<std::string>(config, "service",
      MarketDataService::FEED_SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    auto addresses = std::vector<IpAddress>();
    addresses.push_back(m_interface);
    m_addresses = Extract<std::vector<IpAddress>>(config, "addresses",
      addresses);
  }
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "0.9-r" MARKET_DATA_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto configArg = ValueArg<std::string>("c", "config", "Configuration file",
      false, "config.yml", "path");
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() <<
      std::endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  auto serviceLocatorClientConfig = ServiceLocatorClientConfig();
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      GetNode(config, "service_locator"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'service_locator': " << e.what() <<
      std::endl;
    return -1;
  }
  auto socketThreadPool = SocketThreadPool();
  auto timerThreadPool = TimerThreadPool();
  auto threadPool = ThreadPool();
  auto serviceLocatorClient = ApplicationServiceLocatorClient();
  try {
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password,
      serviceLocatorClientConfig.m_address, Ref(socketThreadPool),
      Ref(timerThreadPool));
  } catch(const std::exception& e) {
    std::cerr << "Error logging in: " << e.what() << std::endl;
    return -1;
  }
  auto definitionsClient = ApplicationDefinitionsClient();
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the definitions service." << std::endl;
    return -1;
  }
  auto administrationClient = ApplicationAdministrationClient();
  try {
    administrationClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the administration service." <<
      std::endl;
    return -1;
  }
  auto registryServerConnectionInitializer =
    RegistryServerConnectionInitializer();
  try {
    auto countryDatabase = definitionsClient->LoadCountryDatabase();
    registryServerConnectionInitializer.Initialize(
      GetNode(config, "registry_server"), countryDatabase);
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'registry_server': " << e.what() <<
      std::endl;
    return -1;
  }
  auto feedServerConnectionInitializer = FeedServerConnectionInitializer();
  try {
    feedServerConnectionInitializer.Initialize(GetNode(config, "feed_server"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'feed_server': " << e.what() <<
      std::endl;
    return -1;
  }
  auto countries = JsonValue(JsonNull());
  try {
    auto countriesNode = config["countries"];
    if(countriesNode) {
      countries = ParseCountries(countriesNode,
        definitionsClient->LoadCountryDatabase());
    }
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'countries': " << e.what() << std::endl;
    return -1;
  }
  auto mySqlConfig = MySqlConfig();
  try {
    mySqlConfig = MySqlConfig::Parse(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'data_store': " << e.what() <<
      std::endl;
    return -1;
  }
  auto historicalDataStore = SqlDataStore(
    [=] {
      return MySql::Connection(mySqlConfig.m_address.GetHost(),
        mySqlConfig.m_address.GetPort(), mySqlConfig.m_username,
        mySqlConfig.m_password, mySqlConfig.m_schema);
    });
  auto asyncDataStore = optional<AsyncHistoricalDataStore<SqlDataStore*>>();
  auto marketDataRegistry = MarketDataRegistry();
  auto baseRegistryServlet = optional<BaseRegistryServlet>();
  try {
    auto cacheBlockSize = Extract<int>(config, "cache_block_size", 1000);
    asyncDataStore.emplace(&historicalDataStore);
    baseRegistryServlet.emplace(&*administrationClient, &marketDataRegistry,
      Initialize(&*asyncDataStore, cacheBlockSize));
  } catch(const std::exception& e) {
    std::cerr << "Error initializing server: " << e.what() << std::endl;
    return -1;
  }
  auto registryServer = optional<RegistryServletContainer>();
  try {
    registryServer.emplace(Initialize(
      serviceLocatorClient.Get(), baseRegistryServlet.get_ptr()), Initialize(
      registryServerConnectionInitializer.m_interface, Ref(socketThreadPool)),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10),
      Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    std::cerr << "Error opening server: " << e.what() << std::endl;
    return -1;
  }
  try {
    auto registryService = JsonObject();
    registryService["addresses"] = lexical_cast<std::string>(Stream(
      registryServerConnectionInitializer.m_addresses));
    if(countries != JsonNull{}) {
      registryService["countries"] = countries;
    }
    serviceLocatorClient->Register(
      registryServerConnectionInitializer.m_serviceName, registryService);
  } catch(const std::exception& e) {
    std::cerr << "Error registering service: " << e.what() << std::endl;
    return -1;
  }
  auto feedServer = optional<FeedServletContainer>();
  try {
    feedServer.emplace(Initialize(serviceLocatorClient.Get(),
      baseRegistryServlet.get_ptr()), Initialize(
      feedServerConnectionInitializer.m_interface, Ref(socketThreadPool)),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10),
      Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    std::cerr << "Error opening server: " << e.what() << std::endl;
    return -1;
  }
  try {
    auto feedService = JsonObject();
    feedService["addresses"] = lexical_cast<std::string>(
      Stream(feedServerConnectionInitializer.m_addresses));
    if(countries != JsonNull()) {
      feedService["countries"] = countries;
    }
    serviceLocatorClient->Register(
      feedServerConnectionInitializer.m_serviceName, feedService);
  } catch(const std::exception& e) {
    std::cerr << "Error registering service: " << e.what() << std::endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
