#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Sql/MySqlConfig.hpp>
#include <Beam/Threading/LiveTimer.hpp>
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
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::UidService;
using namespace boost;
using namespace boost::local_time;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::Compliance;
using namespace Nexus::DefinitionsService;
using namespace Nexus::FixUtilities;
using namespace Nexus::MarketDataService;
using namespace Nexus::OasisOrderExecutionService;
using namespace Nexus::OrderExecutionService;
using namespace TCLAP;
using namespace Viper;

namespace {
  using SqlDataStore = SqlOrderExecutionDataStore<MySql::Connection>;
  using ApplicationFixOrderExecutionDriver = FixOrderExecutionDriver;
  using ApplicationFeesCalculatorOrderExecutionDriver =
    FeesCalculatorOrderExecutionDriver<ApplicationFixOrderExecutionDriver*>;
  using ApplicationOrderSubmissionCheckDriver =
    OrderSubmissionCheckDriver<ApplicationFeesCalculatorOrderExecutionDriver*>;
  using ApplicationComplianceCheckOrderExecutionDriver =
    ComplianceCheckOrderExecutionDriver<
      ApplicationOrderSubmissionCheckDriver*, LiveNtpTimeClient*,
      ComplianceRuleSet<ApplicationComplianceClient::Client*,
        ApplicationServiceLocatorClient::Client*>*>;
  using ApplicationManualOrderEntryDriver =
    ManualOrderEntryDriver<ApplicationComplianceCheckOrderExecutionDriver*,
      ApplicationAdministrationClient::Client*>;
  using ApplicationOrderExecutionDriver = ApplicationManualOrderEntryDriver;
  using OrderExecutionServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaOrderExecutionServlet<
      LiveNtpTimeClient*, ApplicationServiceLocatorClient::Client*,
      ApplicationUidClient::Client*, ApplicationAdministrationClient::Client*,
      ApplicationOrderExecutionDriver*, ReplicatedOrderExecutionDataStore*>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  std::vector<FixApplicationEntry> LoadFixApplications(
      Ref<LiveNtpTimeClient> timeClient,
      Ref<ApplicationMarketDataClient> marketDataClient) {
    return TryOrNest([&] {
      auto entries = std::vector<FixApplicationEntry>();
      auto asxEntry = FixApplicationEntry();
      asxEntry.m_configPath = "asx.cfg";
      asxEntry.m_application =
        std::make_shared<AsxFixApplication>(Ref(timeClient));
      asxEntry.m_destinations.push_back(DefaultDestinations::ASXT());
      asxEntry.m_destinations.push_back(DefaultDestinations::CXA());
      entries.push_back(asxEntry);
      auto serenityEntry = FixApplicationEntry();
      serenityEntry.m_configPath = "serenity.cfg";
      serenityEntry.m_application = std::make_shared<SerenityFixApplication>(
        Ref(timeClient), Ref(**marketDataClient));
      serenityEntry.m_destinations.push_back(DefaultDestinations::ALPHA());
      serenityEntry.m_destinations.push_back(DefaultDestinations::CHIX());
      serenityEntry.m_destinations.push_back(DefaultDestinations::CSE());
      serenityEntry.m_destinations.push_back(DefaultDestinations::CSE2());
      serenityEntry.m_destinations.push_back(DefaultDestinations::CX2());
      serenityEntry.m_destinations.push_back(DefaultDestinations::MATNLP());
      serenityEntry.m_destinations.push_back(DefaultDestinations::MATNMF());
      serenityEntry.m_destinations.push_back(DefaultDestinations::NEOE());
      serenityEntry.m_destinations.push_back(DefaultDestinations::LYNX());
      serenityEntry.m_destinations.push_back(DefaultDestinations::OMEGA());
      serenityEntry.m_destinations.push_back(DefaultDestinations::PURE());
      serenityEntry.m_destinations.push_back(DefaultDestinations::TSX());
      entries.push_back(serenityEntry);
      return entries;
    }, std::runtime_error("Unable to initialize FIX application."));
  }

  std::vector<std::unique_ptr<OrderSubmissionCheck>> LoadOrderSubmissionChecks(
      ApplicationMarketDataClient::Client& marketDataClient,
      ApplicationAdministrationClient::Client& administrationClient,
      const MarketDatabase& marketDatabase,
      const tz_database& timeZoneDatabase,
      const std::vector<ExchangeRate>& exchangeRates) {
    return TryOrNest([&] {
      auto checks = std::vector<std::unique_ptr<OrderSubmissionCheck>>();
      checks.emplace_back(
        MakeBoardLotCheck(&marketDataClient, marketDatabase, timeZoneDatabase));
      checks.emplace_back(std::make_unique<
        BuyingPowerCheck<ApplicationAdministrationClient::Client*,
          ApplicationMarketDataClient::Client*>>(exchangeRates,
            &administrationClient, &marketDataClient));
      checks.emplace_back(std::make_unique<
        RiskStateCheck<ApplicationAdministrationClient::Client*>>(
          &administrationClient));
      return checks;
    }, std::runtime_error("Unable to initialize order submission checks."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" OASIS_ORDER_EXECUTION_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto feeTableConfig = Require(LoadFile, "fee_table.yml");
    auto serviceConfig = TryOrNest([&] {
      return ServiceConfiguration::Parse(GetNode(config, "server"),
        OrderExecutionService::SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto uidClient = ApplicationUidClient(serviceLocatorClient.Get());
    auto timeClient =
      MakeLiveNtpTimeClientFromServiceLocator(*serviceLocatorClient);
    auto administrationClient =
      ApplicationAdministrationClient(serviceLocatorClient.Get());
    auto definitionsClient =
      ApplicationDefinitionsClient(serviceLocatorClient.Get());
    auto complianceClient =
      ApplicationComplianceClient(serviceLocatorClient.Get());
    auto marketDataClient =
      ApplicationMarketDataClient(serviceLocatorClient.Get());
    auto fixApplicationEntries =
      LoadFixApplications(Ref(*timeClient), Ref(marketDataClient));
    auto fixOrderExecutionDriver =
      ApplicationFixOrderExecutionDriver(fixApplicationEntries);
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto asxtFeeTable = TryOrNest([&] {
      return ParseAsxFeeTable(GetNode(feeTableConfig, "au_equities"));
    }, std::runtime_error("Failed parse section 'au_equities'."));
    auto hkexFeeTable = TryOrNest([&] {
      return ParseHkexFeeTable(
        GetNode(feeTableConfig, "hk_equities"), marketDatabase);
    }, std::runtime_error("Failed parse section 'hk_equities'."));
    auto jpxFeeTable = TryOrNest([&] {
      return ParseJpxFeeTable(
        GetNode(feeTableConfig, "jp_equities"), marketDatabase);
    }, std::runtime_error("Failed parse section 'jp_equities'."));
    auto tmxFeeTable = TryOrNest([&] {
      return ParseConsolidatedTmxFeeTable(
        GetNode(feeTableConfig, "ca_equities"), marketDatabase);
    }, std::runtime_error("Failed parse section 'ca_equities'."));
    auto usFeeTable = TryOrNest([&] {
      return ParseConsolidatedUsFeeTable(
        GetNode(feeTableConfig, "us_equities"), marketDatabase);
    }, std::runtime_error("Failed parse section 'us_equities'."));
    auto feesCalculator = ApplicationFeesCalculatorOrderExecutionDriver(
      &fixOrderExecutionDriver, asxtFeeTable, hkexFeeTable, jpxFeeTable,
      tmxFeeTable, usFeeTable);
    auto timeZoneDatabase = definitionsClient->LoadTimeZoneDatabase();
    auto exchangeRates = definitionsClient->LoadExchangeRates();
    auto checks = LoadOrderSubmissionChecks(*marketDataClient,
      *administrationClient, marketDatabase, timeZoneDatabase, exchangeRates);
    auto orderSubmissionCheckDriver = ApplicationOrderSubmissionCheckDriver(
      &feesCalculator, std::move(checks));
    auto complianceRuleSet = ComplianceRuleSet(complianceClient.Get(),
        serviceLocatorClient.Get(), [&] (const auto& entry) {
      return MakeComplianceRule(
        entry.GetSchema(), *marketDataClient, *definitionsClient, *timeClient);
    });
    auto complianceCheckOrderExecutionDriver =
      ApplicationComplianceCheckOrderExecutionDriver(
        &orderSubmissionCheckDriver, timeClient.get(), &complianceRuleSet);
    auto manualOrderExecutionDriver = ApplicationManualOrderEntryDriver(
      DefaultDestinations::MOE(), &complianceCheckOrderExecutionDriver,
      administrationClient.Get());
    auto sessionStartTime =
      ToUtcTime(Extract<ptime>(config, "session_start_time", pos_infin));
    auto mySqlConfigs = TryOrNest([&] {
      return MySqlConfig::ParseReplication(GetNode(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto accountSource = [&] (unsigned int id) {
      return serviceLocatorClient->LoadDirectoryEntry(id);
    };
    auto connectionBuilders = std::vector<SqlDataStore::ConnectionBuilder>();
    for(auto& mySqlConfig : mySqlConfigs) {
      connectionBuilders.emplace_back([=] {
        return MySql::Connection(mySqlConfig.m_address.GetHost(),
          mySqlConfig.m_address.GetPort(), mySqlConfig.m_username,
          mySqlConfig.m_password, mySqlConfig.m_schema);
      });
    }
    auto dataStore = MakeReplicatedMySqlOrderExecutionDataStore(
      connectionBuilders, accountSource);
    auto destinationDatabase = definitionsClient->LoadDestinationDatabase();
    auto server = OrderExecutionServletContainer(Initialize(
      serviceLocatorClient.Get(), Initialize(sessionStartTime,
        marketDatabase, destinationDatabase, timeClient.get(),
        serviceLocatorClient.Get(), uidClient.Get(), administrationClient.Get(),
        &manualOrderExecutionDriver, dataStore.get())),
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
