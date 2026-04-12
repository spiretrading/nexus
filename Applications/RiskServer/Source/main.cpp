#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/Queues/ConverterQueueReader.hpp>
#include <Beam/Queues/FilteredQueueReader.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Sql/MySqlConfig.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <Viper/MySql/Connection.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"
#include "Nexus/RiskService/RiskServlet.hpp"
#include "Nexus/RiskService/SqlRiskDataStore.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Viper;

namespace {
  using ApplicationDataStore =
    SqlRiskDataStore<SqlConnection<MySql::Connection>>;
  using RiskServletContainer =
    ServiceProtocolServletContainer<MetaAuthenticationServletAdapter<
      MetaRiskServlet<ApplicationAdministrationClient*,
        ApplicationMarketDataClient*, ApplicationOrderExecutionClient*,
        LiveTimer, std::unique_ptr<LiveNtpTimeClient>, ApplicationDataStore*>,
      ApplicationServiceLocatorClient*>, TcpServerSocket,
      BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv, "1.0-r" RISK_SERVER_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_config = try_or_nest([&] {
      return ServiceConfiguration::parse(
        get_node(config, "server"), RISK_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto definitions_client =
      ApplicationDefinitionsClient(Ref(service_locator_client));
    auto administration_client =
      ApplicationAdministrationClient(Ref(service_locator_client));
    auto market_data_client =
      ApplicationMarketDataClient(Ref(service_locator_client));
    auto order_execution_client =
      ApplicationOrderExecutionClient(Ref(service_locator_client));
    auto time_client = make_live_ntp_time_client(service_locator_client);
    auto mysql_config = try_or_nest([&] {
      return MySqlConfig::parse(get_node(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto data_store = ApplicationDataStore(
      std::make_unique<SqlConnection<MySql::Connection>>(MySql::Connection(
        mysql_config.m_address.get_host(), mysql_config.m_address.get_port(),
        mysql_config.m_username, mysql_config.m_password,
        mysql_config.m_schema)));
    auto exchange_rates =
      ExchangeRateTable(definitions_client.load_exchange_rates());
    auto destinations = definitions_client.load_destination_database();
    auto accounts = std::make_shared<Queue<AccountUpdate>>();
    service_locator_client.monitor(accounts);
    auto risk_server =
      RiskServletContainer(init(&service_locator_client, init(
        convert(filter(std::move(accounts),
          [] (const auto& update) {
            return update.m_type == AccountUpdate::Type::ADDED;
          }),
          [] (const auto& update) {
            return update.m_account;
          }), &administration_client, &market_data_client,
          &order_execution_client,
          [] {
            return std::make_unique<LiveTimer>(seconds(1));
          }, std::move(time_client), &data_store, std::move(exchange_rates),
        std::move(destinations))),
      init(service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    add(service_locator_client, service_config);
    wait_for_kill_event();
    service_locator_client.close();
    order_execution_client.close();
    market_data_client.close();
    administration_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
