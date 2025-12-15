#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Sql/MySqlConfig.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <Viper/MySql/Connection.hpp>
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/CachedAdministrationDataStore.hpp"
#include "Nexus/AdministrationService/SqlAdministrationDataStore.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Viper;

namespace {
  using AdministrationServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaAdministrationServlet<
      ApplicationServiceLocatorClient*, CachedAdministrationDataStore<
        SqlAdministrationDataStore<SqlConnection<MySql::Connection>>>,
      LocalTimeClient>, ApplicationServiceLocatorClient*>, TcpServerSocket,
      BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  EntitlementDatabase parse_entitlements(
      const YAML::Node& config, const CurrencyDatabase& currencies,
      ApplicationServiceLocatorClient& service_locator_client) {
    auto entitlements_directory = load_or_create_directory(
      service_locator_client, "entitlements", DirectoryEntry::STAR_DIRECTORY);
    auto database = EntitlementDatabase();
    for(auto entitlement_config : config) {
      auto entry = EntitlementDatabase::Entry();
      entry.m_name = extract<std::string>(entitlement_config, "name");
      entry.m_price =
        parse_money(extract<std::string>(entitlement_config, "price"));
      entry.m_currency = currencies.from(
        extract<std::string>(entitlement_config, "currency")).m_id;
      auto group_name = extract<std::string>(entitlement_config, "group");
      entry.m_group_entry = load_or_create_directory(
        service_locator_client, group_name, entitlements_directory);
      auto applicability = get_node(entitlement_config, "applicability");
      for(auto applicability_config : applicability) {
        auto venue =
          Venue(extract<std::string>(applicability_config, "venue", ""));
        auto source =
          Venue(extract<std::string>(applicability_config, "source"));
        auto key = EntitlementKey(venue, source);
        auto messages = get_node(applicability_config, "messages");
        for(auto message_config : messages) {
          auto message = message_config.as<std::string>();
          if(message == "BBO") {
            entry.m_applicability[key].set(MarketDataType::BBO_QUOTE);
          } else if(message == "BOOK") {
            entry.m_applicability[key].set(MarketDataType::BOOK_QUOTE);
          } else if(message == "TAS") {
            entry.m_applicability[key].set(MarketDataType::TIME_AND_SALE);
          } else if(message == "IMB") {
            entry.m_applicability[key].set(MarketDataType::ORDER_IMBALANCE);
          }
        }
      }
      database.add(entry);
    }
    return database;
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" ADMINISTRATION_SERVER_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto mysql_config = try_or_nest([&] {
      return MySqlConfig::parse(get_node(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto service_config = try_or_nest([&] {
      return ServiceConfiguration::parse(
        get_node(config, "server"), ADMINISTRATION_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto definitions_client =
      ApplicationDefinitionsClient(Ref(service_locator_client));
    auto entitlements = parse_entitlements(get_node(config, "entitlements"),
      definitions_client.load_currency_database(), service_locator_client);
    auto account_source = [&] (unsigned int id) {
      return service_locator_client.load_directory_entry(id);
    };
    auto server = AdministrationServletContainer(
      init(&service_locator_client, init(&service_locator_client, entitlements,
        init(init(std::make_unique<SqlConnection<MySql::Connection>>(
          MySql::Connection(mysql_config.m_address.get_host(),
            mysql_config.m_address.get_port(), mysql_config.m_username,
            mysql_config.m_password, mysql_config.m_schema)), account_source)),
        init())), init(service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    add(service_locator_client, service_config);
    wait_for_kill_event();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
