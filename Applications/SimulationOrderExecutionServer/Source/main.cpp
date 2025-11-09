#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Sql/MySqlConfig.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/UidService/ApplicationDefinitions.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <Viper/MySql/Connection.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/Compliance/ApplicationDefinitions.hpp"
#include "Nexus/Compliance/ComplianceCheckOrderExecutionDriver.hpp"
#include "Nexus/Compliance/ComplianceRuleBuilder.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/BoardLotCheck.hpp"
#include "Nexus/OrderExecutionService/BuyingPowerCheck.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServlet.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckDriver.hpp"
#include "Nexus/OrderExecutionService/ReplicatedOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/RiskStateCheck.hpp"
#include "Nexus/OrderExecutionService/SqlOrderExecutionDataStore.hpp"
#include "Nexus/SimulationMatcher/SimulationOrderExecutionDriver.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Viper;

namespace {
  using DataStore =
    SqlOrderExecutionDataStore<SqlConnection<MySql::Connection>>;
  using ApplicationSimulationOrderExecutionDriver =
    SimulationOrderExecutionDriver<
      ApplicationMarketDataClient*, LiveNtpTimeClient*>;
  using ApplicationOrderSubmissionCheckDriver =
    OrderSubmissionCheckDriver<ApplicationSimulationOrderExecutionDriver*>;
  using ApplicationComplianceCheckOrderExecutionDriver =
    ComplianceCheckOrderExecutionDriver<ApplicationOrderSubmissionCheckDriver*,
      LiveNtpTimeClient*, ComplianceRuleSet<
        ApplicationComplianceClient*, ApplicationServiceLocatorClient*>*>;
  using ApplicationOrderExecutionDriver =
    ApplicationComplianceCheckOrderExecutionDriver;
  using OrderExecutionServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaOrderExecutionServlet<
      LiveNtpTimeClient*, ApplicationServiceLocatorClient*,
      ApplicationUidClient*, ApplicationAdministrationClient*,
      ApplicationOrderExecutionDriver*, ReplicatedOrderExecutionDataStore*>,
    ApplicationServiceLocatorClient*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" SIMULATION_ORDER_EXECUTION_SERVER_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
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
    auto market_data_client =
      ApplicationMarketDataClient(Ref(service_locator_client));
    auto definitions_client =
      ApplicationDefinitionsClient(Ref(service_locator_client));
    auto compliance_client =
      ApplicationComplianceClient(Ref(service_locator_client));
    auto simulation_driver = ApplicationSimulationOrderExecutionDriver(
      &market_data_client, time_client.get());
    auto checks = std::vector<std::unique_ptr<OrderSubmissionCheck>>();
    try_or_nest([&] {
      checks.emplace_back(make_board_lot_check(&market_data_client,
        definitions_client.load_venue_database(),
        definitions_client.load_time_zone_database()));
      checks.emplace_back(
        std::make_unique<BuyingPowerCheck<ApplicationAdministrationClient*,
          ApplicationMarketDataClient*>>(
            ExchangeRateTable(definitions_client.load_exchange_rates()),
            &administration_client, &market_data_client));
      checks.emplace_back(
        std::make_unique<RiskStateCheck<ApplicationAdministrationClient*>>(
          &administration_client));
    }, std::runtime_error("Unable to initialize order submission checks"));
    auto submission_check_driver = ApplicationOrderSubmissionCheckDriver(
      &simulation_driver, std::move(checks));
    auto rule_set = ComplianceRuleSet(
      &compliance_client, &service_locator_client, [&] (const auto& entry) {
        return make_compliance_rule(entry.get_schema(), market_data_client,
          definitions_client, *time_client);
      });
    auto compliance_check_driver =
      ApplicationComplianceCheckOrderExecutionDriver(
        &submission_check_driver, time_client.get(), &rule_set);
    auto mysql_configs = try_or_nest([&] {
      return MySqlConfig::parse_replication(get_node(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto session_start_time =
      extract<ptime>(config, "session_start_time", pos_infin);
    auto account_source = [&] (unsigned int id) {
      return service_locator_client.load_directory_entry(id);
    };
    auto connection_builders = std::vector<DataStore::ConnectionBuilder>();
    for(auto& mysql_config : mysql_configs) {
      connection_builders.emplace_back([=] {
        return SqlConnection(MySql::Connection(
          mysql_config.m_address.get_host(), mysql_config.m_address.get_port(),
          mysql_config.m_username, mysql_config.m_password,
          mysql_config.m_schema));
      });
    }
    auto data_store = make_replicated_sql_order_execution_data_store(
      connection_builders, account_source);
    auto order_execution_server = OrderExecutionServletContainer(
      init(&service_locator_client, init(
        session_start_time , definitions_client.load_venue_database(),
        definitions_client.load_destination_database(), time_client.get(),
        &service_locator_client, &uid_client, &administration_client,
        &compliance_check_driver, data_store.get())),
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
