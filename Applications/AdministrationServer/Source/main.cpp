#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Sql/MySqlConfig.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <Beam/Threading/LiveTimer.hpp>
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
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Viper;

namespace {
  using AdministrationServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaAdministrationServlet<
      ApplicationServiceLocatorClient::Client*, CachedAdministrationDataStore<
        SqlAdministrationDataStore<SqlConnection<MySql::Connection>>>,
      LocalTimeClient>,
      ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
      BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  EntitlementDatabase parse_entitlements(
      const YAML::Node& config, const CurrencyDatabase& currencies,
      ApplicationServiceLocatorClient& service_locator_client) {
    auto entitlements_directory = LoadOrCreateDirectory(*service_locator_client,
      "entitlements", DirectoryEntry::GetStarDirectory());
    auto database = EntitlementDatabase();
    for(auto entitlement_config : config) {
      auto entry = EntitlementDatabase::Entry();
      entry.m_name = Extract<std::string>(entitlement_config, "name");
      entry.m_price =
        parse_money(Extract<std::string>(entitlement_config, "price"));
      entry.m_currency = currencies.from(
        Extract<std::string>(entitlement_config, "currency")).m_id;
      auto group_name = Extract<std::string>(entitlement_config, "group");
      entry.m_group_entry = LoadOrCreateDirectory(
        *service_locator_client, group_name, entitlements_directory);
      auto applicability = GetNode(entitlement_config, "applicability");
      for(auto applicability_config : applicability) {
        auto venue =
          Venue(Extract<std::string>(applicability_config, "venue", ""));
        auto source =
          Venue(Extract<std::string>(applicability_config, "source"));
        auto key = EntitlementKey(venue, source);
        auto messages = GetNode(applicability_config, "messages");
        for(auto message_config : messages) {
          auto message = message_config.as<std::string>();
          if(message == "BBO") {
            entry.m_applicability[key].Set(MarketDataType::BBO_QUOTE);
          } else if(message == "BOOK") {
            entry.m_applicability[key].Set(MarketDataType::BOOK_QUOTE);
          } else if(message == "TAS") {
            entry.m_applicability[key].Set(MarketDataType::TIME_AND_SALE);
          } else if(message == "IMB") {
            entry.m_applicability[key].Set(MarketDataType::ORDER_IMBALANCE);
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
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" ADMINISTRATION_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto mysql_config = TryOrNest([&] {
      return MySqlConfig::Parse(GetNode(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto service_config = TryOrNest([&] {
      return ServiceConfiguration::Parse(
        GetNode(config, "server"), ADMINISTRATION_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto service_locator_client =
      MakeApplicationServiceLocatorClient(GetNode(config, "service_locator"));
    auto definitions_client =
      ApplicationDefinitionsClient(service_locator_client.Get());
    auto entitlements = parse_entitlements(GetNode(config, "entitlements"),
      definitions_client->load_currency_database(), service_locator_client);
    auto account_source = [&] (unsigned int id) {
      return service_locator_client->LoadDirectoryEntry(id);
    };
    auto server = AdministrationServletContainer(Initialize(
      service_locator_client.Get(), Initialize(service_locator_client.Get(),
        entitlements, Initialize(Initialize(MakeSqlConnection(MySql::Connection(
          mysql_config.m_address.GetHost(), mysql_config.m_address.GetPort(),
          mysql_config.m_username, mysql_config.m_password,
          mysql_config.m_schema)), account_source)), Initialize())),
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
