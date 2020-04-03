#include <fstream>
#include <iostream>
#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/DistributedMarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataRelayServlet.hpp"
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
using namespace std;
using namespace TCLAP;

namespace {
  using IncomingMarketDataClientSessionBuilder =
    AuthenticatedServiceProtocolClientBuilder<
    ApplicationServiceLocatorClient::Client, MessageProtocol<
    unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>, NullEncoder>,
    LiveTimer>;
  using IncomingMarketDataClient = std::shared_ptr<VirtualMarketDataClient>;
  using MarketDataRelayServletContainer =
    ServiceProtocolServletContainer<MetaAuthenticationServletAdapter<
    MetaMarketDataRelayServlet<IncomingMarketDataClient,
    ApplicationAdministrationClient::Client*>,
    ApplicationServiceLocatorClient::Client*, NativePointerPolicy>,
    TcpServerSocket, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>, std::shared_ptr<LiveTimer>>;
  using BaseMarketDataRelayServlet = MarketDataRelayServlet<
    MarketDataRelayServletContainer, IncomingMarketDataClient,
    ApplicationAdministrationClient::Client*>;

  struct MarketDataRelayServerConnectionInitializer {
    string m_serviceName;
    IpAddress m_interface;
    vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void MarketDataRelayServerConnectionInitializer::Initialize(
      const YAML::Node& config) {
    m_serviceName = Extract<string>(config, "service",
      MarketDataService::RELAY_SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    vector<IpAddress> addresses;
    addresses.push_back(m_interface);
    m_addresses = Extract<vector<IpAddress>>(config, "addresses", addresses);
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "0.9-r" MARKET_DATA_RELAY_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
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
  auto marketDatabase = definitionsClient->LoadMarketDatabase();
  auto marketDataClientBuilder =
    [&] {
      const auto SENTINEL = CountryDatabase::NONE;
      std::unordered_set<CountryCode> availableCountries;
      std::vector<CountryCode> lastCountries;
      auto servicePredicate =
        [&] (const ServiceEntry& entry) {
          if(availableCountries.find(SENTINEL) != availableCountries.end() ||
              !lastCountries.empty()) {
            return false;
          }
          auto countriesNode = entry.GetProperties().Get("countries");
          if(!countriesNode.is_initialized()) {
            availableCountries.insert(SENTINEL);
            return true;
          }
          if(auto countriesList =
              boost::get<std::vector<JsonValue>>(&*countriesNode)) {
            std::vector<CountryCode> countries;
            for(auto& countryValue : *countriesList) {
              auto country = boost::get<double>(&countryValue);
              if(country == nullptr) {
                return false;
              } else {
                countries.push_back(static_cast<CountryCode>(*country));
              }
            }
            for(auto& country : countries) {
              if(availableCountries.find(country) !=
                  availableCountries.end()) {
                return false;
              }
            }
            lastCountries = std::move(countries);
            availableCountries.insert(lastCountries.begin(),
              lastCountries.end());
            return true;
          } else {
            return false;
          }
        };
      std::unordered_map<CountryCode, std::shared_ptr<VirtualMarketDataClient>>
        countryToMarketDataClients;
      std::unordered_map<MarketCode, std::shared_ptr<VirtualMarketDataClient>>
        marketToMarketDataClients;
      while(availableCountries.find(SENTINEL) == availableCountries.end()) {
        try {
          auto incomingMarketDataClient = MakeVirtualMarketDataClient(
            std::make_unique<MarketDataClient<
            IncomingMarketDataClientSessionBuilder>>(
            BuildBasicMarketDataClientSessionBuilder<
            IncomingMarketDataClientSessionBuilder>(Ref(*serviceLocatorClient),
            Ref(socketThreadPool), Ref(timerThreadPool), servicePredicate,
            REGISTRY_SERVICE_NAME)));
          if(lastCountries.empty()) {
            incomingMarketDataClient->Open();
            return incomingMarketDataClient;
          }
          std::shared_ptr<VirtualMarketDataClient> client =
            std::move(incomingMarketDataClient);
          for(auto& country : lastCountries) {
            countryToMarketDataClients[country] = client;
            for(auto& market : marketDatabase.FromCountry(country)) {
              marketToMarketDataClients[market.m_code] = client;
            }
          }
          lastCountries.clear();
        } catch(const std::exception&) {
          if(countryToMarketDataClients.empty()) {
            throw;
          }
          break;
        }
      }
      auto distributedClient = MakeVirtualMarketDataClient(
        std::make_unique<DistributedMarketDataClient>(
        std::move(countryToMarketDataClients),
        std::move(marketToMarketDataClients)));
      distributedClient->Open();
      return distributedClient;
    };
  boost::optional<BaseMarketDataRelayServlet> baseRegistryServlet;
  try {
    auto entitlements = administrationClient->LoadEntitlements();
    auto clientTimeout = Extract<time_duration>(config, "connection_timeout",
      milliseconds{500});
    auto minConnections = static_cast<size_t>(Extract<int>(config,
      "min_connections", boost::thread::hardware_concurrency()));
    auto maxConnections = static_cast<size_t>(Extract<int>(config,
      "max_connections", 10 * minConnections));
    baseRegistryServlet.emplace(entitlements, clientTimeout,
      marketDataClientBuilder, minConnections, maxConnections,
      &*administrationClient, Ref(timerThreadPool));
  } catch(const std::exception& e) {
    cerr << "Error initializing registry servlet: " << e.what() << endl;
    return -1;
  }
  MarketDataRelayServerConnectionInitializer serverConnectionInitializer;
  try {
    serverConnectionInitializer.Initialize(GetNode(config, "server"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'server': " << e.what() << endl;
    return -1;
  }
  MarketDataRelayServletContainer server(Initialize(serviceLocatorClient.Get(),
    baseRegistryServlet.get_ptr()),
    Initialize(serverConnectionInitializer.m_interface, Ref(socketThreadPool)),
    std::bind(factory<std::shared_ptr<LiveTimer>>{}, seconds{10},
    Ref(timerThreadPool)));
  try {
    server.Open();
  } catch(const std::exception& e) {
    cerr << "Error opening server: " << e.what() << endl;
    return -1;
  }
  try {
    JsonObject service;
    service["addresses"] = ToString(serverConnectionInitializer.m_addresses);
    serviceLocatorClient->Register(serverConnectionInitializer.m_serviceName,
      service);
  } catch(const std::exception& e) {
    cerr << "Error registering service: " << e.what() << endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
