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
#include <Beam/Threading/LiveTimer.hpp>
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
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
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
  using ApplicationDataStore =
    SqlRiskDataStore<SqlConnection<MySql::Connection>>;
  using RiskServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<
      MetaRiskServlet<ApplicationAdministrationClient::Client*,
        ApplicationMarketDataClient::Client*,
        ApplicationOrderExecutionClient::Client*, LiveTimer,
        std::unique_ptr<LiveNtpTimeClient>, ApplicationDataStore*>,
      ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv, "0.9-r" RISK_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto service_config = TryOrNest([&] {
      return ServiceConfiguration::Parse(
        GetNode(config, "server"), RISK_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto service_locator_client =
      MakeApplicationServiceLocatorClient(GetNode(config, "service_locator"));
    auto definitions_client =
      ApplicationDefinitionsClient(service_locator_client.Get());
    auto administration_client =
      ApplicationAdministrationClient(service_locator_client.Get());
    auto market_data_client =
      ApplicationMarketDataClient(service_locator_client.Get());
    auto order_execution_client =
      ApplicationOrderExecutionClient(service_locator_client.Get());
    auto time_client =
      MakeLiveNtpTimeClientFromServiceLocator(*service_locator_client);
    auto mysql_config = TryOrNest([&] {
      return MySqlConfig::Parse(GetNode(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto data_store = ApplicationDataStore(MakeSqlConnection(MySql::Connection(
      mysql_config.m_address.GetHost(), mysql_config.m_address.GetPort(),
      mysql_config.m_username, mysql_config.m_password,
      mysql_config.m_schema)));
    auto exchange_rates =
      ExchangeRateTable(definitions_client->load_exchange_rates());
    auto venues = definitions_client->load_venue_database();
    auto destinations = definitions_client->load_destination_database();
    auto accounts = std::make_shared<Queue<AccountUpdate>>();
    service_locator_client->MonitorAccounts(accounts);
    auto risk_server =
      RiskServletContainer(Initialize(service_locator_client.Get(), Initialize(
        MakeConverterQueueReader(
          MakeFilteredQueueReader(std::move(accounts), [] (const auto& update) {
            return update.m_type == AccountUpdate::Type::ADDED;
          }),
          [] (const auto& update) {
            return update.m_account;
          }), administration_client.Get(), market_data_client.Get(),
        order_execution_client.Get(),
        [] {
          return std::make_unique<LiveTimer>(seconds(1));
        }, std::move(time_client), &data_store, std::move(exchange_rates),
        std::move(venues), std::move(destinations))),
      Initialize(service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*service_locator_client, service_config);
    WaitForKillEvent();
    service_locator_client->Close();
    order_execution_client->close();
    market_data_client->close();
    administration_client->close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
