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
#include <Beam/Utilities/Streamable.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
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
using namespace TCLAP;

namespace {
  using IncomingMarketDataClientSessionBuilder =
    AuthenticatedServiceProtocolClientBuilder<
    ApplicationServiceLocatorClient::Client, MessageProtocol<
    std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>, NullEncoder>,
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
    std::string m_serviceName;
    IpAddress m_interface;
    std::vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void MarketDataRelayServerConnectionInitializer::Initialize(
      const YAML::Node& config) {
    m_serviceName = Extract<std::string>(config, "service",
      MarketDataService::RELAY_SERVICE_NAME);
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
    auto cmd = CmdLine("", ' ', "0.9-r" MARKET_DATA_RELAY_SERVER_VERSION
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
  auto marketDatabase = definitionsClient->LoadMarketDatabase();
  auto marketDataClientBuilder = [&] {
    const auto SENTINEL = CountryCode::NONE;
    auto availableCountries = std::unordered_set<CountryCode>();
    auto lastCountries = std::vector<CountryCode>();
    auto servicePredicate = [&] (const auto& entry) {
      if(availableCountries.find(SENTINEL) != availableCountries.end() ||
          !lastCountries.empty()) {
        return false;
      }
      auto countriesNode = entry.GetProperties().Get("countries");
      if(!countriesNode) {
        availableCountries.insert(SENTINEL);
        return true;
      }
      if(auto countriesList = get<std::vector<JsonValue>>(&*countriesNode)) {
        auto countries = std::vector<CountryCode>();
        for(auto& countryValue : *countriesList) {
          if(auto country = get<double>(&countryValue)) {
            return false;
          } else {
            countries.emplace_back(static_cast<std::uint16_t>(*country));
          }
        }
        for(auto& country : countries) {
          if(availableCountries.find(country) != availableCountries.end()) {
            return false;
          }
        }
        lastCountries = std::move(countries);
        availableCountries.insert(lastCountries.begin(), lastCountries.end());
        return true;
      } else {
        return false;
      }
    };
    auto countryToMarketDataClients = std::unordered_map<
      CountryCode, std::shared_ptr<VirtualMarketDataClient>>();
    auto marketToMarketDataClients = std::unordered_map<
      MarketCode, std::shared_ptr<VirtualMarketDataClient>>();
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
          return incomingMarketDataClient;
        }
        auto client = std::shared_ptr<VirtualMarketDataClient>(
          std::move(incomingMarketDataClient));
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
    return MakeVirtualMarketDataClient(
      std::make_unique<DistributedMarketDataClient>(
      std::move(countryToMarketDataClients),
      std::move(marketToMarketDataClients)));
  };
  auto baseRegistryServlet = optional<BaseMarketDataRelayServlet>();
  try {
    auto entitlements = administrationClient->LoadEntitlements();
    auto clientTimeout = Extract<time_duration>(config, "connection_timeout",
      milliseconds{500});
    auto minConnections = static_cast<std::size_t>(Extract<int>(config,
      "min_connections", thread::hardware_concurrency()));
    auto maxConnections = static_cast<std::size_t>(Extract<int>(config,
      "max_connections", 10 * minConnections));
    baseRegistryServlet.emplace(entitlements, clientTimeout,
      marketDataClientBuilder, minConnections, maxConnections,
      &*administrationClient, Ref(timerThreadPool));
  } catch(const std::exception& e) {
    std::cerr << "Error initializing registry servlet: " << e.what() <<
      std::endl;
    return -1;
  }
  auto serverConnectionInitializer =
    MarketDataRelayServerConnectionInitializer();
  try {
    serverConnectionInitializer.Initialize(GetNode(config, "server"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'server': " << e.what() << std::endl;
    return -1;
  }
  auto server = optional<MarketDataRelayServletContainer>();
  try {
    server.emplace(Initialize(serviceLocatorClient.Get(),
      baseRegistryServlet.get_ptr()), Initialize(
      serverConnectionInitializer.m_interface, Ref(socketThreadPool)),
      std::bind(factory<std::shared_ptr<LiveTimer>>{}, seconds{10},
      Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    std::cerr << "Error opening server: " << e.what() << std::endl;
    return -1;
  }
  try {
    auto service = JsonObject();
    service["addresses"] = lexical_cast<std::string>(Stream(
      serverConnectionInitializer.m_addresses));
    serviceLocatorClient->Register(serverConnectionInitializer.m_serviceName,
      service);
  } catch(const std::exception& e) {
    std::cerr << "Error registering service: " << e.what() << std::endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
