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
#include <boost/lexical_cast.hpp>
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

namespace {
  using IncomingMarketDataClientSessionBuilder =
    AuthenticatedServiceProtocolClientBuilder<
      ApplicationServiceLocatorClient::Client*, MessageProtocol<
        std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>,
        NullEncoder>, LiveTimer>;
  using IncomingMarketDataClient = std::shared_ptr<VirtualMarketDataClient>;
  using MarketDataRelayServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaMarketDataRelayServlet<
      IncomingMarketDataClient, ApplicationAdministrationClient::Client*>,
      ApplicationServiceLocatorClient::Client*, NativePointerPolicy>,
    TcpServerSocket, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>, std::shared_ptr<LiveTimer>>;
  using BaseMarketDataRelayServlet = MarketDataRelayServlet<
    MarketDataRelayServletContainer, IncomingMarketDataClient,
    ApplicationAdministrationClient::Client*>;
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" MARKET_DATA_RELAY_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto serviceConfig = TryOrNest([&] {
      return ServiceConfiguration::Parse(GetNode(config, "server"),
        MarketDataService::RELAY_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto definitionsClient = ApplicationDefinitionsClient(
      serviceLocatorClient.Get());
    auto administrationClient = ApplicationAdministrationClient(
      serviceLocatorClient.Get());
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
              countries.emplace_back(static_cast<std::uint16_t>(*country));
            } else {
              return false;
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
                  IncomingMarketDataClientSessionBuilder>(
                    serviceLocatorClient.Get(), servicePredicate,
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
    auto entitlements = administrationClient->LoadEntitlements();
    auto clientTimeout = Extract<time_duration>(config, "connection_timeout",
      milliseconds(500));
    auto minConnections = static_cast<std::size_t>(Extract<int>(config,
      "min_connections", thread::hardware_concurrency()));
    auto maxConnections = static_cast<std::size_t>(Extract<int>(config,
      "max_connections", 10 * minConnections));
    auto baseRegistryServlet = BaseMarketDataRelayServlet(entitlements,
      clientTimeout, marketDataClientBuilder, minConnections, maxConnections,
      &*administrationClient);
    auto server = MarketDataRelayServletContainer(Initialize(
      serviceLocatorClient.Get(), &baseRegistryServlet),
      Initialize(serviceConfig.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*serviceLocatorClient, serviceConfig);
    WaitForKillEvent();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
