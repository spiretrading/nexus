#include <fstream>
#include <iostream>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/Parsers/Parse.hpp>
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
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
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
#include "OasisOrderExecutionServer/AequitasFixApplication.hpp"
#include "OasisOrderExecutionServer/AsxFixApplication.hpp"
#include "OasisOrderExecutionServer/ChixFixApplication.hpp"
#include "OasisOrderExecutionServer/CnsxFixApplication.hpp"
#include "OasisOrderExecutionServer/FeeCalculatorOrderExecutionDriver.hpp"
#include "OasisOrderExecutionServer/LekFixApplication.hpp"
#include "OasisOrderExecutionServer/MatchNowFixApplication.hpp"
#include "OasisOrderExecutionServer/MonexBoomFixApplication.hpp"
#include "OasisOrderExecutionServer/OmegaFixApplication.hpp"
#include "OasisOrderExecutionServer/TsxSorFixApplication.hpp"
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
  using ApplicationOrderSubmissionCheckDriver = OrderSubmissionCheckDriver<
    ApplicationFeesCalculatorOrderExecutionDriver*>;
  using ApplicationComplianceCheckOrderExecutionDriver =
    ComplianceCheckOrderExecutionDriver<ApplicationOrderSubmissionCheckDriver*,
    LiveNtpTimeClient*, ComplianceRuleSet<ApplicationComplianceClient::Client*,
    ApplicationServiceLocatorClient::Client*>*>;
  using ApplicationManualOrderEntryDriver = ManualOrderEntryDriver<
    ApplicationComplianceCheckOrderExecutionDriver*,
    ApplicationAdministrationClient::Client*>;
  using ApplicationOrderExecutionDriver = ApplicationManualOrderEntryDriver;
  using OrderExecutionServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaOrderExecutionServlet<
    LiveNtpTimeClient*, ApplicationServiceLocatorClient::Client*,
    ApplicationUidClient::Client*, ApplicationAdministrationClient::Client*,
    ApplicationOrderExecutionDriver*, ReplicatedOrderExecutionDataStore*>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  struct OrderExecutionServerConnectionInitializer {
    std::string m_serviceName;
    IpAddress m_interface;
    std::vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void OrderExecutionServerConnectionInitializer::Initialize(
      const YAML::Node& config) {
    m_serviceName = Extract<std::string>(config, "service",
      OrderExecutionService::SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    auto addresses = std::vector<IpAddress>();
    addresses.push_back(m_interface);
    m_addresses = Extract<std::vector<IpAddress>>(config, "addresses",
      addresses);
  }
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  auto feeTableFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "0.9-r" OASIS_ORDER_EXECUTION_SERVER_VERSION
      "\nCopyright (C) 2009 Eidolon Systems Ltd.");
    auto configArg = ValueArg<std::string>("c", "config", "Configuration file",
      false, "config.yml", "path");
    cmd.add(configArg);
    auto feeTableArg = ValueArg<std::string>("f", "fee_table", "Fee table file",
      false, "fee_table.yml", "path");
    cmd.add(feeTableArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
    feeTableFile = feeTableArg.getValue();
  } catch(const ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() <<
      std::endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  auto feeTableConfig = Require(LoadFile, feeTableFile);
  auto serviceLocatorClientConfig = ServiceLocatorClientConfig();
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      GetNode(config, "service_locator"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'service_locator': " << e.what() <<
      std::endl;
    return -1;
  }
  auto serviceLocatorClient = ApplicationServiceLocatorClient();
  try {
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password,
      serviceLocatorClientConfig.m_address);
  } catch(const std::exception& e) {
    std::cerr << "Error logging in: " << e.what() << std::endl;
    return -1;
  }
  auto uidClient = ApplicationUidClient();
  try {
    uidClient.BuildSession(Ref(*serviceLocatorClient));
  } catch(const std::exception& e) {
    std::cerr << "Error connecting to the uid service: " << e.what() <<
      std::endl;
    return -1;
  }
  auto timeClient = std::unique_ptr<LiveNtpTimeClient>();
  try {
    auto timeServices = serviceLocatorClient->Locate(TimeService::SERVICE_NAME);
    if(timeServices.empty()) {
      std::cerr << "No time services available." << std::endl;
      return -1;
    }
    auto& timeService = timeServices.front();
    auto ntpPool = Parse<std::vector<IpAddress>>(get<std::string>(
      timeService.GetProperties().At("addresses")));
    timeClient = MakeLiveNtpTimeClient(ntpPool);
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize NTP client: " << e.what() << std::endl;
    return -1;
  }
  auto administrationClient = ApplicationAdministrationClient();
  try {
    administrationClient.BuildSession(Ref(*serviceLocatorClient));
  } catch(const std::exception& e) {
    std::cerr << "Error connecting to the administration service: " <<
      e.what() << std::endl;
    return -1;
  }
  auto definitionsClient = ApplicationDefinitionsClient();
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient));
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the definitions service." << std::endl;
    return -1;
  }
  auto complianceClient = ApplicationComplianceClient();
  try {
    complianceClient.BuildSession(Ref(*serviceLocatorClient));
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the compliance service." << std::endl;
    return -1;
  }
  auto marketDataClient = ApplicationMarketDataClient();
  try {
    marketDataClient.BuildSession(Ref(*serviceLocatorClient));
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the market data service." << std::endl;
    return -1;
  }
  auto fixApplicationEntries = std::vector<FixApplicationEntry>();
  try {
    auto asxEntry = FixApplicationEntry();
    asxEntry.m_configPath = "asx.cfg";
    asxEntry.m_application = std::make_shared<AsxFixApplication>(
      Ref(*timeClient));
    asxEntry.m_destinations.push_back(DefaultDestinations::ASXT());
    asxEntry.m_destinations.push_back(DefaultDestinations::CXA());
    fixApplicationEntries.push_back(asxEntry);
    auto chixEntry = FixApplicationEntry();
    chixEntry.m_configPath = "chix.cfg";
    chixEntry.m_application = std::make_shared<ChixFixApplication>(
      Ref(*timeClient));
    chixEntry.m_destinations.push_back(DefaultDestinations::CHIX());
    chixEntry.m_destinations.push_back(DefaultDestinations::CX2());
    chixEntry.m_destinations.push_back(DefaultDestinations::TSX());
    fixApplicationEntries.push_back(chixEntry);
    auto tsxEntry = FixApplicationEntry();
    tsxEntry.m_configPath = "tsxsor.cfg";
    tsxEntry.m_application = std::make_shared<TsxSorFixApplication>(
      Ref(*timeClient));
    tsxEntry.m_destinations.push_back(DefaultDestinations::ALPHA());
    fixApplicationEntries.push_back(tsxEntry);
    auto matchNowLiquidityProviderEntry = FixApplicationEntry();
    matchNowLiquidityProviderEntry.m_configPath = "matnlp.cfg";
    matchNowLiquidityProviderEntry.m_application =
      std::make_shared<MatchNowFixApplication>(Ref(*timeClient));
    matchNowLiquidityProviderEntry.m_destinations.push_back(
      DefaultDestinations::MATNLP());
    fixApplicationEntries.push_back(matchNowLiquidityProviderEntry);
    auto matchNowMarketFlowEntry = FixApplicationEntry();
    matchNowMarketFlowEntry.m_configPath = "matnmf.cfg";
    matchNowMarketFlowEntry.m_application =
      std::make_shared<MatchNowFixApplication>(Ref(*timeClient));
    matchNowMarketFlowEntry.m_destinations.push_back(
      DefaultDestinations::MATNMF());
    fixApplicationEntries.push_back(matchNowMarketFlowEntry);
    auto omegaEntry = FixApplicationEntry();
    omegaEntry.m_configPath = "omega.cfg";
    omegaEntry.m_application = std::make_shared<OmegaFixApplication>(
      Ref(*timeClient), Ref(*marketDataClient));
    omegaEntry.m_destinations.push_back(DefaultDestinations::LYNX());
    omegaEntry.m_destinations.push_back(DefaultDestinations::OMEGA());
    fixApplicationEntries.push_back(omegaEntry);
    auto pureEntry = FixApplicationEntry();
    pureEntry.m_configPath = "pure.cfg";
    pureEntry.m_application = std::make_shared<CnsxFixApplication>(
      Ref(*timeClient));
    pureEntry.m_destinations.push_back(DefaultDestinations::PURE());
    pureEntry.m_destinations.push_back(DefaultDestinations::CSE());
    fixApplicationEntries.push_back(pureEntry);
    auto neoeEntry = FixApplicationEntry();
    neoeEntry.m_configPath = "neoe.cfg";
    neoeEntry.m_application = std::make_shared<AequitasFixApplication>(
      Ref(*timeClient));
    neoeEntry.m_destinations.push_back(DefaultDestinations::NEOE());
    fixApplicationEntries.push_back(neoeEntry);
    auto lekEntry = FixApplicationEntry();
    lekEntry.m_configPath = "lek.cfg";
    lekEntry.m_application = std::make_shared<LekFixApplication>(
      Ref(*timeClient));
    lekEntry.m_destinations.push_back(DefaultDestinations::AMEX());
    lekEntry.m_destinations.push_back(DefaultDestinations::ARCA());
    lekEntry.m_destinations.push_back(DefaultDestinations::BATS());
    lekEntry.m_destinations.push_back(DefaultDestinations::BATY());
    lekEntry.m_destinations.push_back(DefaultDestinations::CBSX());
    lekEntry.m_destinations.push_back(DefaultDestinations::EDGA());
    lekEntry.m_destinations.push_back(DefaultDestinations::EDGX());
    lekEntry.m_destinations.push_back(DefaultDestinations::NYSE());
    lekEntry.m_destinations.push_back(DefaultDestinations::NASDAQ());
    fixApplicationEntries.push_back(lekEntry);
    auto boomEntry = FixApplicationEntry();
    boomEntry.m_configPath = "boom.cfg";
    boomEntry.m_application = std::make_shared<MonexBoomFixApplication>(
      Ref(*timeClient), Ref(*marketDataClient));
    boomEntry.m_destinations.push_back(DefaultDestinations::HKEX());
    boomEntry.m_destinations.push_back(DefaultDestinations::OSE());
    boomEntry.m_destinations.push_back(DefaultDestinations::TSE());
    fixApplicationEntries.push_back(boomEntry);
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize FIX entry: " << e.what() << std::endl;
    return -1;
  }
  auto fixOrderExecutionDriver = ApplicationFixOrderExecutionDriver(
    fixApplicationEntries);
  auto marketDatabase = MarketDatabase();
  try {
    marketDatabase = definitionsClient->LoadMarketDatabase();
  } catch(const std::exception& e) {
    std::cerr << "Unable to load market database: " << e.what() << std::endl;
    return -1;
  }
  auto asxtFeeTable = AsxtFeeTable();
  try {
    asxtFeeTable = ParseAsxFeeTable(GetNode(feeTableConfig, "au_equities"));
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize ASX fee table: " << e.what() <<
      std::endl;
    return -1;
  }
  auto hkexFeeTable = HkexFeeTable();
  try {
    hkexFeeTable = ParseHkexFeeTable(GetNode(feeTableConfig, "hk_equities"),
      marketDatabase);
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize HKEX fee table: " << e.what() <<
      std::endl;
    return -1;
  }
  auto jpxFeeTable = JpxFeeTable();
  try {
    jpxFeeTable = ParseJpxFeeTable(GetNode(feeTableConfig, "jp_equities"),
      marketDatabase);
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize JPX fee table: " << e.what() <<
      std::endl;
    return -1;
  }
  auto tmxFeeTable = ConsolidatedTmxFeeTable();
  try {
    tmxFeeTable = ParseConsolidatedTmxFeeTable(
      GetNode(feeTableConfig, "ca_equities"), marketDatabase);
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize TMX fee table: " << e.what() <<
      std::endl;
    return -1;
  }
  auto usFeeTable = ConsolidatedUsFeeTable();
  try {
    usFeeTable = ParseConsolidatedUsFeeTable(
      GetNode(feeTableConfig, "us_equities"), marketDatabase);
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize US fee table: " << e.what() << std::endl;
    return -1;
  }
  auto feesCalculator = ApplicationFeesCalculatorOrderExecutionDriver(
    &fixOrderExecutionDriver, asxtFeeTable, hkexFeeTable, jpxFeeTable,
    tmxFeeTable, usFeeTable);
  auto checks = std::vector<std::unique_ptr<OrderSubmissionCheck>>();
  try {
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
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize order submission checks: " << e.what() <<
      std::endl;
    return -1;
  }
  auto orderSubmissionCheckDriver = ApplicationOrderSubmissionCheckDriver(
    &feesCalculator, std::move(checks));
  auto complianceRuleSet = ComplianceRuleSet(complianceClient.Get(),
    serviceLocatorClient.Get(),
    [&] (const auto& entry) {
      return BuildComplianceRule(entry.GetSchema(), *marketDataClient,
        *definitionsClient, *timeClient);
    });
  auto complianceCheckOrderExecutionDriver =
    ApplicationComplianceCheckOrderExecutionDriver(&orderSubmissionCheckDriver,
    timeClient.get(), &complianceRuleSet);
  auto manualOrderExecutionDriver = ApplicationManualOrderEntryDriver(
    DefaultDestinations::MOE(), &complianceCheckOrderExecutionDriver,
    administrationClient.Get());
  auto sessionStartTime = ToUtcTime(Extract<ptime>(config, "session_start_time",
    pos_infin));
  auto orderExecutionServerConnectionInitializer =
    OrderExecutionServerConnectionInitializer();
  try {
    orderExecutionServerConnectionInitializer.Initialize(
      GetNode(config, "server"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'server': " << e.what() << std::endl;
    return -1;
  }
  auto mySqlConfigs = std::vector<MySqlConfig>();
  try {
    mySqlConfigs = MySqlConfig::ParseReplication(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'data_store': " << e.what() <<
      std::endl;
    return -1;
  }
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
  auto orderExecutionServer = optional<OrderExecutionServletContainer>();
  try {
    orderExecutionServer.emplace(Initialize(serviceLocatorClient.Get(),
      Initialize(sessionStartTime, definitionsClient->LoadMarketDatabase(),
      definitionsClient->LoadDestinationDatabase(), timeClient.get(),
      serviceLocatorClient.Get(), uidClient.Get(), administrationClient.Get(),
      &manualOrderExecutionDriver, dataStore.get())),
      Initialize(orderExecutionServerConnectionInitializer.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
  } catch(const std::exception& e) {
    std::cerr << "Error opening order server: " << e.what() << std::endl;
    return -1;
  }
  try {
    auto orderExecutionService = JsonObject();
    orderExecutionService["addresses"] = lexical_cast<std::string>(
      Stream(orderExecutionServerConnectionInitializer.m_addresses));
    serviceLocatorClient->Register(
      orderExecutionServerConnectionInitializer.m_serviceName,
      orderExecutionService);
  } catch(const std::exception& e) {
    std::cerr << "Error registering service: " << e.what() << std::endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
