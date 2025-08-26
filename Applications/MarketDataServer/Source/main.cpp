#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Sql/MySqlConfig.hpp>
#include <Beam/Sql/SqlConnection.hpp>
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
using namespace Viper;

namespace {
  using SqlDataStore = SqlHistoricalDataStore<SqlConnection<MySql::Connection>>;
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

  JsonValue parse_countries(
      const YAML::Node& config, const CountryDatabase& countries) {
    return TryOrNest([&] {
      auto country_nodes = std::vector<JsonValue>();
      for(auto item : config) {
        auto code = item.as<std::string>();
        auto country = parse_country_code(code, countries);
        if(!country) {
          throw std::runtime_error("Country code not found: " + code);
        }
        country_nodes.push_back(
          static_cast<double>(static_cast<std::uint16_t>(country)));
      }
      if(country_nodes.empty()) {
        return JsonValue(JsonNull());
      }
      return JsonValue(country_nodes);
    }, std::runtime_error("Error parsing countries."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "0.9-r" MARKET_DATA_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto service_locator_client =
      MakeApplicationServiceLocatorClient(GetNode(config, "service_locator"));
    auto definitions_client =
      ApplicationDefinitionsClient(service_locator_client.Get());
    auto administration_client =
      ApplicationAdministrationClient(service_locator_client.Get());
    auto countries = definitions_client->load_country_database();
    auto registry_service_config = TryOrNest([&] {
      return ServiceConfiguration::Parse(
        GetNode(config, "registry_server"), MARKET_DATA_REGISTRY_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'registry_server'."));
    auto feed_service_config = TryOrNest([&] {
      return ServiceConfiguration::Parse(
        GetNode(config, "feed_server"), MARKET_DATA_FEED_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'feed_server'."));
    auto countries_node = TryOrNest([&] {
      if(auto countries_node = config["countries"]) {
        return parse_countries(
          countries_node, definitions_client->load_country_database());
      }
      return JsonValue(JsonNull());
    }, std::runtime_error("Error parsing section 'countries'"));
    if(countries_node != JsonNull()) {
      registry_service_config.m_properties["countries"] = countries_node;
      feed_service_config.m_properties["countries"] = countries_node;
    }
    auto mysql_config = TryOrNest([&] {
      return MySqlConfig::Parse(GetNode(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto venues = definitions_client->load_venue_database();
    auto historical_data_store = SqlDataStore(venues, [=] {
      return SqlConnection(MySql::Connection(mysql_config.m_address.GetHost(),
        mysql_config.m_address.GetPort(), mysql_config.m_username,
        mysql_config.m_password, mysql_config.m_schema));
    });
    auto async_data_store = AsyncHistoricalDataStore(&historical_data_store);
    auto cache_block_size = Extract<int>(config, "cache_block_size", 1000);
    auto time_zone_database = definitions_client->load_time_zone_database();
    auto market_data_registry = MarketDataRegistry(venues, time_zone_database);
    auto base_registry_servlet = BaseRegistryServlet(&*administration_client,
      &market_data_registry, Initialize(&async_data_store, cache_block_size));
    auto registry_server = RegistryServletContainer(
      Initialize(service_locator_client.Get(), &base_registry_servlet),
      Initialize(registry_service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*service_locator_client, registry_service_config);
    auto feedServer = FeedServletContainer(
      Initialize(service_locator_client.Get(), &base_registry_servlet),
      Initialize(feed_service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*service_locator_client, feed_service_config);
    WaitForKillEvent();
    service_locator_client->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
