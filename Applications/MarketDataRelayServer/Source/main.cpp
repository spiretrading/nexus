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
  using IncomingMarketDataClient = std::shared_ptr<MarketDataClientBox>;
  using MarketDataRelayServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaMarketDataRelayServlet<
      IncomingMarketDataClient, ApplicationAdministrationClient::Client*>,
      ApplicationServiceLocatorClient::Client*, NativePointerPolicy>,
    TcpServerSocket, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>, std::shared_ptr<LiveTimer>>;
  using BaseMarketDataRelayServlet = MarketDataRelayServlet<
    MarketDataRelayServletContainer, IncomingMarketDataClient,
    ApplicationAdministrationClient::Client*>;

  std::unordered_set<CountryCode> ExtractCountries(const JsonObject& node,
      const CountryDatabase& countryDatabase) {
    auto countries = std::unordered_set<CountryCode>();
    if(auto countriesNode = node.Get("countries")) {
      if(auto countryList = get<std::vector<JsonValue>>(&*countriesNode)) {
        for(auto& countryValue : *countryList) {
          if(auto country = get<double>(&countryValue)) {
            countries.insert(CountryCode(static_cast<std::uint16_t>(*country)));
          }
        }
      }
    } else {
      for(auto entry : countryDatabase.GetEntries()) {
        countries.insert(entry.m_code);
      }
    }
    return countries;
  }
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
    auto countryDatabase = definitionsClient->LoadCountryDatabase();
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto marketDataClientBuilder = [&] {
      auto entries = serviceLocatorClient->Locate(REGISTRY_SERVICE_NAME);
      if(entries.empty()) {
        BOOST_THROW_EXCEPTION(ConnectException(
          "No " + REGISTRY_SERVICE_NAME + " services available."));
      }
      auto countryToMarketDataClients = std::unordered_map<
        CountryCode, std::shared_ptr<MarketDataClientBox>>();
      auto marketToMarketDataClients = std::unordered_map<
        MarketCode, std::shared_ptr<MarketDataClientBox>>();
      for(auto& entry : entries) {
        auto countries = ExtractCountries(entry.GetProperties(),
          countryDatabase);
        auto marketDataClient = std::make_shared<MarketDataClientBox>(
          std::in_place_type<
            MarketDataClient<IncomingMarketDataClientSessionBuilder>>,
          BuildBasicMarketDataClientSessionBuilder<
            IncomingMarketDataClientSessionBuilder>(serviceLocatorClient.Get(),
              [=] (const auto& candidateEntry) {
                auto candidateCountries = ExtractCountries(
                  candidateEntry.GetProperties(), countryDatabase);
                return countries.size() <= candidateCountries.size() &&
                  std::all_of(countries.begin(), countries.end(),
                    [&] (auto country) {
                      return candidateCountries.count(country) == 1;
                    });
              }, REGISTRY_SERVICE_NAME));
        for(auto country : countries) {
          countryToMarketDataClients.insert(std::pair(country,
            marketDataClient));
          for(auto& market : marketDatabase.FromCountry(country)) {
            marketToMarketDataClients.insert(std::pair(market.m_code,
              marketDataClient));
          }
        }
      }
      return std::make_unique<MarketDataClientBox>(
        std::in_place_type<DistributedMarketDataClient>,
        std::move(countryToMarketDataClients),
        std::move(marketToMarketDataClients));
    };
    auto clientTimeout = Extract<time_duration>(config, "connection_timeout",
      milliseconds(500));
    auto minConnections = static_cast<std::size_t>(Extract<int>(config,
      "min_connections", thread::hardware_concurrency()));
    auto maxConnections = static_cast<std::size_t>(Extract<int>(config,
      "max_connections", 10 * minConnections));
    auto baseRegistryServlet = BaseMarketDataRelayServlet(clientTimeout,
      marketDataClientBuilder, minConnections, maxConnections,
      administrationClient.Get());
    auto server = MarketDataRelayServletContainer(Initialize(
      serviceLocatorClient.Get(), &baseRegistryServlet),
      Initialize(serviceConfig.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*serviceLocatorClient, serviceConfig);
    WaitForKillEvent();
    serviceLocatorClient->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
