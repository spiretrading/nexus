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
using namespace Nexus::Accounting;
using namespace Nexus::AdministrationService;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace Viper;

namespace {
  using ApplicationDataStore = SqlRiskDataStore<
    SqlConnection<MySql::Connection>>;
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
    auto serviceConfig = TryOrNest([&] {
      return ServiceConfiguration::Parse(GetNode(config, "server"),
        RiskService::SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto definitionsClient = ApplicationDefinitionsClient(
      serviceLocatorClient.Get());
    auto administrationClient = ApplicationAdministrationClient(
      serviceLocatorClient.Get());
    auto marketDataClient = ApplicationMarketDataClient(
      serviceLocatorClient.Get());
    auto orderExecutionClient = ApplicationOrderExecutionClient(
      serviceLocatorClient.Get());
    auto timeClient = MakeLiveNtpTimeClientFromServiceLocator(
      *serviceLocatorClient);
    auto mySqlConfig = TryOrNest([&] {
      return MySqlConfig::Parse(GetNode(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto dataStore = ApplicationDataStore(MakeSqlConnection(MySql::Connection(
      mySqlConfig.m_address.GetHost(), mySqlConfig.m_address.GetPort(),
      mySqlConfig.m_username, mySqlConfig.m_password, mySqlConfig.m_schema)));
    auto exchangeRates = definitionsClient->LoadExchangeRates();
    auto markets = definitionsClient->LoadMarketDatabase();
    auto destinations = definitionsClient->LoadDestinationDatabase();
    auto accounts = std::make_shared<Queue<AccountUpdate>>();
    serviceLocatorClient->MonitorAccounts(accounts);
    auto riskServer = RiskServletContainer(Initialize(
      serviceLocatorClient.Get(), Initialize(
        MakeConverterQueueReader(MakeFilteredQueueReader(std::move(accounts),
          [] (const auto& update) {
            return update.m_type == AccountUpdate::Type::ADDED;
          }),
          [] (const auto& update) {
            return update.m_account;
          }), administrationClient.Get(), marketDataClient.Get(),
        orderExecutionClient.Get(),
        [] {
          return std::make_unique<LiveTimer>(seconds(1));
        }, std::move(timeClient), &dataStore, std::move(exchangeRates),
        std::move(markets), std::move(destinations))),
      Initialize(serviceConfig.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*serviceLocatorClient, serviceConfig);
    WaitForKillEvent();
    orderExecutionClient->Close();
    marketDataClient->Close();
    administrationClient->Close();
    serviceLocatorClient->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
