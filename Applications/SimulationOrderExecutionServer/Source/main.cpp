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
#include <Beam/Threading/LiveTimer.hpp>
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
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Parsers;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::UidService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Viper;

namespace {
  using SqlDataStore =
    SqlOrderExecutionDataStore<SqlConnection<MySql::Connection>>;
  using ApplicationSimulationOrderExecutionDriver =
    SimulationOrderExecutionDriver<
      ApplicationMarketDataClient::Client*, LiveNtpTimeClient*>;
  using ApplicationOrderSubmissionCheckDriver =
    OrderSubmissionCheckDriver<ApplicationSimulationOrderExecutionDriver*>;
  using ApplicationComplianceCheckOrderExecutionDriver =
    ComplianceCheckOrderExecutionDriver<ApplicationOrderSubmissionCheckDriver*,
      LiveNtpTimeClient*,
      ComplianceRuleSet<ApplicationComplianceClient::Client*,
        ApplicationServiceLocatorClient::Client*>*>;
  using ApplicationOrderExecutionDriver =
    ApplicationComplianceCheckOrderExecutionDriver;
  using OrderExecutionServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaOrderExecutionServlet<
      LiveNtpTimeClient*, ApplicationServiceLocatorClient::Client*,
      ApplicationUidClient::Client*, ApplicationAdministrationClient::Client*,
      ApplicationOrderExecutionDriver*, ReplicatedOrderExecutionDataStore*>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "0.9-r" SIMULATION_ORDER_EXECUTION_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto service_config = TryOrNest([&] {
      return ServiceConfiguration::Parse(
        GetNode(config, "server"), ORDER_EXECUTION_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto service_locator_client =
      MakeApplicationServiceLocatorClient(GetNode(config, "service_locator"));
    auto uid_client = ApplicationUidClient(service_locator_client.Get());
    auto time_client =
      MakeLiveNtpTimeClientFromServiceLocator(*service_locator_client);
    auto administration_client =
      ApplicationAdministrationClient(service_locator_client.Get());
    auto market_data_client =
      ApplicationMarketDataClient(service_locator_client.Get());
    auto definitions_client =
      ApplicationDefinitionsClient(service_locator_client.Get());
    auto compliance_client =
      ApplicationComplianceClient(service_locator_client.Get());
    auto simulation_driver = ApplicationSimulationOrderExecutionDriver(
      market_data_client.Get(), time_client.get());
    auto checks = std::vector<std::unique_ptr<OrderSubmissionCheck>>();
    TryOrNest([&] {
      checks.emplace_back(make_board_lot_check(market_data_client.Get(),
        definitions_client->load_venue_database(),
        definitions_client->load_time_zone_database()));
      checks.emplace_back(std::make_unique<
        BuyingPowerCheck<ApplicationAdministrationClient::Client*,
          ApplicationMarketDataClient::Client*>>(
            ExchangeRateTable(definitions_client->load_exchange_rates()),
            administration_client.Get(), market_data_client.Get()));
      checks.emplace_back(std::make_unique<
        RiskStateCheck<ApplicationAdministrationClient::Client*>>(
          administration_client.Get()));
    }, std::runtime_error("Unable to initialize order submission checks"));
    auto submission_check_driver = ApplicationOrderSubmissionCheckDriver(
      &simulation_driver, std::move(checks));
    auto rule_set = ComplianceRuleSet(compliance_client.Get(),
      service_locator_client.Get(), [&] (const auto& entry) {
        return make_compliance_rule(entry.get_schema(), *market_data_client,
          *definitions_client, *time_client);
      });
    auto compliance_check_driver =
      ApplicationComplianceCheckOrderExecutionDriver(
        &submission_check_driver, time_client.get(), &rule_set);
    auto mysql_configs = TryOrNest([&] {
      return MySqlConfig::ParseReplication(GetNode(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto session_start_time =
      Extract<ptime>(config, "session_start_time", pos_infin);
    auto account_source = [&] (unsigned int id) {
      return service_locator_client->LoadDirectoryEntry(id);
    };
    auto connection_builders = std::vector<SqlDataStore::ConnectionBuilder>();
    for(auto& mysql_config : mysql_configs) {
      connection_builders.emplace_back([=] {
        return SqlConnection(MySql::Connection(mysql_config.m_address.GetHost(),
          mysql_config.m_address.GetPort(), mysql_config.m_username,
          mysql_config.m_password, mysql_config.m_schema));
      });
    }
    auto data_store = make_replicated_sql_order_execution_data_store(
      connection_builders, account_source);
    auto order_execution_server = OrderExecutionServletContainer(
      Initialize(service_locator_client.Get(), Initialize(
        session_start_time , definitions_client->load_venue_database(),
        definitions_client->load_destination_database(), time_client.get(),
        service_locator_client.Get(), uid_client.Get(),
        administration_client.Get(), &compliance_check_driver,
        data_store.get())),
      Initialize(service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*service_locator_client, service_config);
    WaitForKillEvent();
    service_locator_client->Close();
    compliance_client->close();
    market_data_client->close();
    administration_client->close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
