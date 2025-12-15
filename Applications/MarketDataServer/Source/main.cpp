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
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
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
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Viper;

namespace {
  using DataStore = SqlHistoricalDataStore<SqlConnection<MySql::Connection>>;
  using RegistryServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaMarketDataRegistryServlet<
      MarketDataRegistry*, SessionCachedHistoricalDataStore<
        AsyncHistoricalDataStore<DataStore*>*>,
      ApplicationAdministrationClient*>,
      ApplicationServiceLocatorClient*, NativePointerPolicy>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;
  using BaseRegistryServlet = MarketDataRegistryServlet<
    RegistryServletContainer, MarketDataRegistry*,
    SessionCachedHistoricalDataStore<AsyncHistoricalDataStore<DataStore*>*>,
    ApplicationAdministrationClient*>;
  using FeedServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<
      MetaMarketDataFeedServlet<BaseRegistryServlet*>,
      ApplicationServiceLocatorClient*>, TcpServerSocket,
    BinarySender<SharedBuffer>, SizeDeclarativeEncoder<ZLibEncoder>,
    std::shared_ptr<LiveTimer>>;

  JsonValue parse_countries(
      const YAML::Node& config, const CountryDatabase& countries) {
    return try_or_nest([&] {
      auto country_nodes = std::vector<JsonValue>();
      for(auto item : config) {
        auto code = item.as<std::string>();
        auto country = parse_country_code(code, countries);
        if(!country) {
          throw_with_location(
            std::runtime_error("Country code not found: " + code));
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
    auto config = parse_command_line(argc, argv,
      "1.0-r" MARKET_DATA_SERVER_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto definitions_client =
      ApplicationDefinitionsClient(Ref(service_locator_client));
    auto administration_client =
      ApplicationAdministrationClient(Ref(service_locator_client));
    auto countries = definitions_client.load_country_database();
    auto registry_service_config = try_or_nest([&] {
      return ServiceConfiguration::parse(
        get_node(config, "registry_server"), MARKET_DATA_REGISTRY_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'registry_server'."));
    auto feed_service_config = try_or_nest([&] {
      return ServiceConfiguration::parse(
        get_node(config, "feed_server"), MARKET_DATA_FEED_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'feed_server'."));
    auto countries_node = try_or_nest([&] {
      if(auto countries_node = config["countries"]) {
        return parse_countries(
          countries_node, definitions_client.load_country_database());
      }
      return JsonValue(JsonNull());
    }, std::runtime_error("Error parsing section 'countries'"));
    if(countries_node != JsonNull()) {
      registry_service_config.m_properties["countries"] = countries_node;
      feed_service_config.m_properties["countries"] = countries_node;
    }
    auto mysql_config = try_or_nest([&] {
      return MySqlConfig::parse(get_node(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto venues = definitions_client.load_venue_database();
    auto historical_data_store = DataStore(venues, [=] {
      return SqlConnection(MySql::Connection(mysql_config.m_address.get_host(),
        mysql_config.m_address.get_port(), mysql_config.m_username,
        mysql_config.m_password, mysql_config.m_schema));
    });
    auto async_data_store = AsyncHistoricalDataStore(&historical_data_store);
    auto cache_block_size = extract<int>(config, "cache_block_size", 1000);
    auto time_zone_database = definitions_client.load_time_zone_database();
    auto market_data_registry = MarketDataRegistry(venues, time_zone_database);
    auto base_registry_servlet = BaseRegistryServlet(&administration_client,
      &market_data_registry, init(&async_data_store, cache_block_size));
    auto registry_server = RegistryServletContainer(
      init(&service_locator_client, &base_registry_servlet),
      init(registry_service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    add(service_locator_client, registry_service_config);
    auto feed_server = FeedServletContainer(
      init(&service_locator_client, &base_registry_servlet),
      init(feed_service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    add(service_locator_client, feed_service_config);
    wait_for_kill_event();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
