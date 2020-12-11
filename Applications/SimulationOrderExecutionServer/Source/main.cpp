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
#include "Nexus/InternalMatcher/InternalMatchingOrderExecutionDriver.hpp"
#include "Nexus/InternalMatcher/NullMatchReportBuilder.hpp"
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
using namespace Nexus::AdministrationService;
using namespace Nexus::Compliance;
using namespace Nexus::DefinitionsService;
using namespace Nexus::InternalMatcher;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Viper;

namespace {
  using SqlDataStore = SqlOrderExecutionDataStore<
    SqlConnection<MySql::Connection>>;
  using ApplicationSimulationOrderExecutionDriver =
    SimulationOrderExecutionDriver<ApplicationMarketDataClient::Client*,
      LiveNtpTimeClient*>;
  using ApplicationInternalMatchingOrderExecutionDriver =
    InternalMatchingOrderExecutionDriver<NullMatchReportBuilder,
      ApplicationMarketDataClient::Client*, LiveNtpTimeClient*,
      ApplicationUidClient::Client*,
      ApplicationSimulationOrderExecutionDriver*>;
  using ApplicationOrderSubmissionCheckDriver = OrderSubmissionCheckDriver<
    ApplicationInternalMatchingOrderExecutionDriver*>;
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
    auto serviceConfig = TryOrNest([&] {
      return ServiceConfiguration::Parse(GetNode(config, "server"),
        OrderExecutionService::SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto uidClient = ApplicationUidClient(serviceLocatorClient.Get());
    auto timeClient = MakeLiveNtpTimeClientFromServiceLocator(
      *serviceLocatorClient);
    auto administrationClient = ApplicationAdministrationClient(
      serviceLocatorClient.Get());
    auto marketDataClient = ApplicationMarketDataClient(
      serviceLocatorClient.Get());
    auto definitionsClient = ApplicationDefinitionsClient(
      serviceLocatorClient.Get());
    auto complianceClient = ApplicationComplianceClient(
      serviceLocatorClient.Get());
    auto simulationOrderExecutionDriver =
      ApplicationSimulationOrderExecutionDriver(marketDataClient.Get(),
        timeClient.get());
    auto internalMatchingOrderExecutionDriver =
      ApplicationInternalMatchingOrderExecutionDriver(
        serviceLocatorClient->GetAccount(), Initialize(),
        marketDataClient.Get(), timeClient.get(), uidClient.Get(),
        &simulationOrderExecutionDriver);
    auto checks = std::vector<std::unique_ptr<OrderSubmissionCheck>>();
    TryOrNest([&] {
      checks.emplace_back(MakeBoardLotCheck(marketDataClient.Get(),
        definitionsClient->LoadMarketDatabase(),
        definitionsClient->LoadTimeZoneDatabase()));
      checks.emplace_back(std::make_unique<
        BuyingPowerCheck<ApplicationAdministrationClient::Client*,
          ApplicationMarketDataClient::Client*>>(
            definitionsClient->LoadExchangeRates(), administrationClient.Get(),
            marketDataClient.Get()));
      checks.emplace_back(std::make_unique<
        RiskStateCheck<ApplicationAdministrationClient::Client*>>(
          administrationClient.Get()));
    }, std::runtime_error("Unable to initialize order submission checks"));
    auto orderSubmissionCheckDriver = ApplicationOrderSubmissionCheckDriver(
      &internalMatchingOrderExecutionDriver, std::move(checks));
    auto complianceRuleSet = ComplianceRuleSet(complianceClient.Get(),
      serviceLocatorClient.Get(), [&] (const auto& entry) {
        return BuildComplianceRule(entry.GetSchema(), *marketDataClient,
          *definitionsClient, *timeClient);
      });
    auto complianceCheckOrderExecutionDriver =
      ApplicationComplianceCheckOrderExecutionDriver(
        &orderSubmissionCheckDriver, timeClient.get(), &complianceRuleSet);
    auto mySqlConfigs = TryOrNest([&] {
      return MySqlConfig::ParseReplication(GetNode(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto sessionStartTime = Extract<ptime>(config, "session_start_time",
      pos_infin);
    auto accountSource = [&] (unsigned int id) {
      return serviceLocatorClient->LoadDirectoryEntry(id);
    };
    auto connectionBuilders = std::vector<SqlDataStore::ConnectionBuilder>();
    for(auto& mySqlConfig : mySqlConfigs) {
      connectionBuilders.emplace_back([=] {
        return SqlConnection(MySql::Connection(mySqlConfig.m_address.GetHost(),
          mySqlConfig.m_address.GetPort(), mySqlConfig.m_username,
          mySqlConfig.m_password, mySqlConfig.m_schema));
      });
    }
    auto dataStore = MakeReplicatedMySqlOrderExecutionDataStore(
      connectionBuilders, accountSource);
    auto orderExecutionServer = OrderExecutionServletContainer(
      Initialize(serviceLocatorClient.Get(), Initialize(
        sessionStartTime, definitionsClient->LoadMarketDatabase(),
        definitionsClient->LoadDestinationDatabase(), timeClient.get(),
        serviceLocatorClient.Get(), uidClient.Get(), administrationClient.Get(),
        &complianceCheckOrderExecutionDriver, dataStore.get())),
      Initialize(serviceConfig.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*serviceLocatorClient, serviceConfig);
    WaitForKillEvent();
    serviceLocatorClient->Close();
    complianceClient->Close();
    marketDataClient->Close();
    administrationClient->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
