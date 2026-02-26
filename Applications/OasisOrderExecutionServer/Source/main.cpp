#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Sql/MySqlConfig.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/UidService/ApplicationDefinitions.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <Viper/MySql/Connection.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/Compliance/ApplicationDefinitions.hpp"
#include "Nexus/Compliance/ComplianceCheckOrderExecutionDriver.hpp"
#include "Nexus/Compliance/ComplianceRuleBuilder.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/FixUtilities/FixOrderExecutionDriver.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/BoardLotCheck.hpp"
#include "Nexus/OrderExecutionService/BuyingPowerCheck.hpp"
#include "Nexus/OrderExecutionService/ManualOrderEntryDriver.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServlet.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckDriver.hpp"
#include "Nexus/OrderExecutionService/ReplicatedOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/RiskStateCheck.hpp"
#include "Nexus/OrderExecutionService/SqlOrderExecutionDataStore.hpp"
#include "OasisOrderExecutionServer/AsxFixApplication.hpp"
#include "OasisOrderExecutionServer/FeeCalculatorOrderExecutionDriver.hpp"
#include "OasisOrderExecutionServer/SerenityFixApplication.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::local_time;
using namespace boost::posix_time;
using namespace Nexus;
using namespace TCLAP;
using namespace Viper;

namespace {
  using ApplicationSqlDataStore = SqlOrderExecutionDataStore<MySql::Connection>;
  using ApplicationFixOrderExecutionDriver = FixOrderExecutionDriver;
  using ApplicationFeesCalculatorOrderExecutionDriver =
    FeesCalculatorOrderExecutionDriver<ApplicationFixOrderExecutionDriver*>;
  using ApplicationOrderSubmissionCheckDriver =
    OrderSubmissionCheckDriver<ApplicationFeesCalculatorOrderExecutionDriver*>;
  using ApplicationComplianceCheckOrderExecutionDriver =
    ComplianceCheckOrderExecutionDriver<ApplicationOrderSubmissionCheckDriver*,
      LiveNtpTimeClient*, ComplianceRuleSet<
        ApplicationComplianceClient*, ApplicationServiceLocatorClient*>*>;
  using ApplicationManualOrderEntryDriver =
    ManualOrderEntryDriver<ApplicationComplianceCheckOrderExecutionDriver*,
      ApplicationAdministrationClient*>;
  using ApplicationOrderExecutionDriver = ApplicationManualOrderEntryDriver;
  using OrderExecutionServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaOrderExecutionServlet<
      LiveNtpTimeClient*, ApplicationServiceLocatorClient*,
      ApplicationUidClient*, ApplicationAdministrationClient*,
      ApplicationOrderExecutionDriver*, ReplicatedOrderExecutionDataStore*>,
    ApplicationServiceLocatorClient*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  std::vector<FixApplicationEntry> load_fix_applications(
      Ref<LiveNtpTimeClient> time_client,
      Ref<ApplicationMarketDataClient> market_data_client) {
    return try_or_nest([&] {
      auto entries = std::vector<FixApplicationEntry>();
      auto asx_entry = FixApplicationEntry();
      asx_entry.m_settings = FIX::SessionSettings("asx.cfg");
      asx_entry.m_application =
        std::make_shared<AsxFixApplication>(Ref(time_client));
      asx_entry.m_destinations.push_back(DefaultDestinations::ASXT);
      asx_entry.m_destinations.push_back(DefaultDestinations::CXA);
      entries.push_back(asx_entry);
      auto serenity_entry = FixApplicationEntry();
      serenity_entry.m_settings = FIX::SessionSettings("serenity.cfg");
      serenity_entry.m_application = std::make_shared<SerenityFixApplication>(
        Ref(time_client), Ref(market_data_client));
      serenity_entry.m_destinations.push_back(DefaultDestinations::ALPHA);
      serenity_entry.m_destinations.push_back(DefaultDestinations::CHIX);
      serenity_entry.m_destinations.push_back(DefaultDestinations::CSE);
      serenity_entry.m_destinations.push_back(DefaultDestinations::CSE2);
      serenity_entry.m_destinations.push_back(DefaultDestinations::CX2);
      serenity_entry.m_destinations.push_back(DefaultDestinations::MATNLP);
      serenity_entry.m_destinations.push_back(DefaultDestinations::MATNMF);
      serenity_entry.m_destinations.push_back(DefaultDestinations::NEOE);
      serenity_entry.m_destinations.push_back(DefaultDestinations::LYNX);
      serenity_entry.m_destinations.push_back(DefaultDestinations::OMEGA);
      serenity_entry.m_destinations.push_back(DefaultDestinations::PURE);
      serenity_entry.m_destinations.push_back(DefaultDestinations::TSX);
      serenity_entry.m_destinations.push_back(DefaultDestinations::OTCM);
      entries.push_back(serenity_entry);
      return entries;
    }, std::runtime_error("Unable to initialize FIX application."));
  }

  std::vector<std::unique_ptr<OrderSubmissionCheck>>
      load_order_submission_checks(
        ApplicationMarketDataClient& market_data_client,
        ApplicationAdministrationClient& administration_client,
        const ExchangeRateTable& exchange_rates) {
    return try_or_nest([&] {
      auto checks = std::vector<std::unique_ptr<OrderSubmissionCheck>>();
      checks.emplace_back(make_board_lot_check(
        &market_data_client, DEFAULT_VENUES, get_default_time_zone_database()));
      checks.emplace_back(std::make_unique<
        BuyingPowerCheck<ApplicationAdministrationClient*,
          ApplicationMarketDataClient*>>(exchange_rates,
            &administration_client, &market_data_client));
      checks.emplace_back(std::make_unique<
        RiskStateCheck<ApplicationAdministrationClient*>>(
          &administration_client));
      return checks;
    }, std::runtime_error("Unable to initialize order submission checks."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" OASIS_ORDER_EXECUTION_SERVER_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto fee_table_config = load_file("fee_table.yml");
    auto service_config = try_or_nest([&] {
      return ServiceConfiguration::parse(
        get_node(config, "server"), ORDER_EXECUTION_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto uid_client = ApplicationUidClient(Ref(service_locator_client));
    auto time_client = make_live_ntp_time_client(service_locator_client);
    auto administration_client =
      ApplicationAdministrationClient(Ref(service_locator_client));
    auto definitions_client =
      ApplicationDefinitionsClient(Ref(service_locator_client));
    auto compliance_client =
      ApplicationComplianceClient(Ref(service_locator_client));
    auto market_data_client =
      ApplicationMarketDataClient(Ref(service_locator_client));
    auto fix_application_entries =
      load_fix_applications(Ref(*time_client), Ref(market_data_client));
    auto fix_order_execution_driver =
      ApplicationFixOrderExecutionDriver(fix_application_entries);
    auto asx_trade_match_fee_table = try_or_nest([&] {
      return parse_asx_trade_match_fee_table(
        get_node(fee_table_config, "au_equities"));
    }, std::runtime_error("Failed parse section 'au_equities'."));
    auto tmx_fee_table = try_or_nest([&] {
      return parse_consolidated_tmx_fee_table(
        get_node(fee_table_config, "ca_equities"), DEFAULT_VENUES);
    }, std::runtime_error("Failed parse section 'ca_equities'."));
    auto fees_calculator = ApplicationFeesCalculatorOrderExecutionDriver(
      &fix_order_execution_driver, asx_trade_match_fee_table, tmx_fee_table);
    auto exchange_rates =
      ExchangeRateTable(definitions_client.load_exchange_rates());
    auto checks = load_order_submission_checks(
      market_data_client, administration_client, exchange_rates);
    auto order_submission_check_driver = ApplicationOrderSubmissionCheckDriver(
      &fees_calculator, std::move(checks));
    auto compliance_rule_set = ComplianceRuleSet(&compliance_client,
      &service_locator_client, [&] (const auto& entry) {
        return make_compliance_rule(entry.get_schema(), market_data_client,
          definitions_client, *time_client);
      });
    auto compliance_check_order_execution_driver =
      ApplicationComplianceCheckOrderExecutionDriver(
        &order_submission_check_driver, time_client.get(),
        &compliance_rule_set);
    auto manual_order_execution_driver = ApplicationManualOrderEntryDriver(
      DefaultDestinations::MOE, &compliance_check_order_execution_driver,
      &administration_client);
    auto session_start_time =
      to_utc_time(extract<ptime>(config, "session_start_time", pos_infin));
    auto mysql_configs = try_or_nest([&] {
      return MySqlConfig::parse_replication(get_node(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto account_source = [&] (unsigned int id) {
      return service_locator_client.load_directory_entry(id);
    };
    auto connection_builders =
      std::vector<ApplicationSqlDataStore::ConnectionBuilder>();
    for(auto& mysql_config : mysql_configs) {
      connection_builders.emplace_back([=] {
        return MySql::Connection(mysql_config.m_address.get_host(),
          mysql_config.m_address.get_port(), mysql_config.m_username,
          mysql_config.m_password, mysql_config.m_schema);
      });
    }
    auto data_store = make_replicated_sql_order_execution_data_store(
      connection_builders, account_source);
    auto server = OrderExecutionServletContainer(init(
      &service_locator_client, init(session_start_time,
        DEFAULT_VENUES, DEFAULT_DESTINATIONS, time_client.get(),
        &service_locator_client, &uid_client, &administration_client,
        &manual_order_execution_driver, data_store.get())),
      init(service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    add(service_locator_client, service_config);
    wait_for_kill_event();
    service_locator_client.close();
    compliance_client.close();
    market_data_client.close();
    administration_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
