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

namespace {
  using IncomingMarketDataClientSessionBuilder =
    AuthenticatedServiceProtocolClientBuilder<
      ApplicationServiceLocatorClient::Client*, MessageProtocol<
        std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>,
        NullEncoder>, LiveTimer>;
  using IncomingMarketDataClient = std::shared_ptr<MarketDataClient>;
  using MarketDataRelayServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaMarketDataRelayServlet<
      IncomingMarketDataClient, ApplicationAdministrationClient::Client*>,
      ApplicationServiceLocatorClient::Client*, NativePointerPolicy>,
    TcpServerSocket, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>, std::shared_ptr<LiveTimer>>;
  using BaseMarketDataRelayServlet = MarketDataRelayServlet<
    MarketDataRelayServletContainer, IncomingMarketDataClient,
    ApplicationAdministrationClient::Client*>;

  Region parse_region(
      const JsonObject& node, const CountryDatabase& countries) {
    auto region = Region();
    if(auto countries_node = node.Get("countries")) {
      if(auto country_list = get<std::vector<JsonValue>>(&*countries_node)) {
        for(auto& country_value : *country_list) {
          if(auto country = get<double>(&country_value)) {
            region += CountryCode(static_cast<std::uint16_t>(*country));
          }
        }
      }
    } else {
      region = Region::GLOBAL;
    }
    return region;
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" MARKET_DATA_RELAY_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto service_config = TryOrNest([&] {
      return ServiceConfiguration::Parse(
        GetNode(config, "server"), MARKET_DATA_RELAY_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto service_locator_client = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto definitions_client =
      ApplicationDefinitionsClient(service_locator_client.Get());
    auto administration_client =
      ApplicationAdministrationClient(service_locator_client.Get());
    auto countries = definitions_client->load_country_database();
    auto market_data_client_builder = [&] {
      auto entries = service_locator_client->Locate(
        MARKET_DATA_REGISTRY_SERVICE_NAME);
      if(entries.empty()) {
        BOOST_THROW_EXCEPTION(ConnectException(
          "No " + MARKET_DATA_REGISTRY_SERVICE_NAME + " services available."));
      }
      auto clients = RegionMap<std::shared_ptr<MarketDataClient>>(nullptr);
      for(auto& entry : entries) {
        auto region = parse_region(entry.GetProperties(), countries);
        auto market_data_client =
          std::make_shared<MarketDataClient>(std::in_place_type<
            ServiceMarketDataClient<IncomingMarketDataClientSessionBuilder>>,
          make_basic_market_data_client_session_builder<
            IncomingMarketDataClientSessionBuilder>(
              service_locator_client.Get(), [=] (const auto& candidate_entry) {
                auto candidate_region =
                  parse_region(candidate_entry.GetProperties(), countries);
                return region <= candidate_region;
              }, MARKET_DATA_REGISTRY_SERVICE_NAME));
      }
      return std::make_unique<MarketDataClient>(
        std::in_place_type<DistributedMarketDataClient>, std::move(clients));
    };
    auto client_timeout =
      Extract<time_duration>(config, "connection_timeout", milliseconds(500));
    auto min_connections = static_cast<std::size_t>(
      Extract<int>(config, "min_connections", thread::hardware_concurrency()));
    auto max_connections = static_cast<std::size_t>(
      Extract<int>(config, "max_connections", 10 * min_connections));
    auto base_registry_servlet = BaseMarketDataRelayServlet(client_timeout,
      market_data_client_builder, min_connections, max_connections,
      administration_client.Get());
    auto server = MarketDataRelayServletContainer(
      Initialize(service_locator_client.Get(), &base_registry_servlet),
      Initialize(service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*service_locator_client, service_config);
    WaitForKillEvent();
    service_locator_client->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
