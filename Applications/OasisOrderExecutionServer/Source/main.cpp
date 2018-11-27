#include <fstream>
#include <iostream>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
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
#include "OasisOrderExecutionServer/OmegaFixApplication.hpp"
#include "OasisOrderExecutionServer/TsxSorFixApplication.hpp"
#include "OasisOrderExecutionServer/Version.hpp"

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
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::Compliance;
using namespace Nexus::DefinitionsService;
using namespace Nexus::FixUtilities;
using namespace Nexus::MarketDataService;
using namespace Nexus::OasisOrderExecutionService;
using namespace Nexus::OrderExecutionService;
using namespace std;
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
    string m_serviceName;
    IpAddress m_interface;
    vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void OrderExecutionServerConnectionInitializer::Initialize(
      const YAML::Node& config) {
    m_serviceName = Extract<string>(config, "service",
      OrderExecutionService::SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    vector<IpAddress> addresses;
    addresses.push_back(m_interface);
    m_addresses = Extract<vector<IpAddress>>(config, "addresses", addresses);
  }
}

int main(int argc, const char** argv) {
  string configFile;
  string feeTableFile;
  try {
    CmdLine cmd{"", ' ', "0.9-r" OASIS_ORDER_EXECUTION_SERVER_VERSION
      "\nCopyright (C) 2009 Eidolon Systems Ltd."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    ValueArg<string> feeTableArg{"f", "fee_table", "Fee table file", false,
      "fee_table.yml", "path"};
    cmd.add(feeTableArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
    feeTableFile = feeTableArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  auto feeTableConfig = Require(LoadFile, feeTableFile);
  ServiceLocatorClientConfig serviceLocatorClientConfig;
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      GetNode(config, "service_locator"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'service_locator': " << e.what() << endl;
    return -1;
  }
  SocketThreadPool socketThreadPool;
  TimerThreadPool timerThreadPool;
  ApplicationServiceLocatorClient serviceLocatorClient;
  try {
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_address,
      Ref(socketThreadPool), Ref(timerThreadPool));
    serviceLocatorClient->SetCredentials(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password);
    serviceLocatorClient->Open();
  } catch(const std::exception& e) {
    cerr << "Error logging in: " << e.what() << endl;
    return -1;
  }
  ApplicationUidClient uidClient;
  try {
    uidClient.BuildSession(Ref(*serviceLocatorClient), Ref(socketThreadPool),
      Ref(timerThreadPool));
    uidClient->Open();
  } catch(const std::exception& e) {
    cerr << "Error connecting to the uid service: " << e.what() << endl;
    return -1;
  }
  unique_ptr<LiveNtpTimeClient> timeClient;
  try {
    auto timeServices = serviceLocatorClient->Locate(TimeService::SERVICE_NAME);
    if(timeServices.empty()) {
      cerr << "No time services available." << endl;
      return -1;
    }
    auto& timeService = timeServices.front();
    auto ntpPool = FromString<vector<IpAddress>>(get<string>(
      timeService.GetProperties().At("addresses")));
    timeClient = MakeLiveNtpTimeClient(ntpPool, Ref(socketThreadPool),
      Ref(timerThreadPool));
  } catch(const  std::exception& e) {
    cerr << "Unable to initialize NTP client: " << e.what() << endl;
    return -1;
  }
  try {
    timeClient->Open();
  } catch(const std::exception&) {
    cerr << "NTP service unavailable." << endl;
    return -1;
  }
  ApplicationAdministrationClient administrationClient;
  try {
    administrationClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    administrationClient->Open();
  } catch(const std::exception& e) {
    cerr << "Error connecting to the administration service: " << e.what() <<
      endl;
    return -1;
  }
  ApplicationDefinitionsClient definitionsClient;
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the definitions service." << endl;
    return -1;
  }
  ApplicationComplianceClient complianceClient;
  try {
    complianceClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    complianceClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the compliance service." << endl;
    return -1;
  }
  ApplicationMarketDataClient marketDataClient;
  try {
    marketDataClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    marketDataClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the market data service." << endl;
    return -1;
  }
  vector<FixApplicationEntry> fixApplicationEntries;
  try {
    FixApplicationEntry asxEntry;
    asxEntry.m_configPath = "asx.cfg";
    asxEntry.m_application = std::make_shared<AsxFixApplication>(
      Ref(*timeClient));
    asxEntry.m_destinations.push_back(DefaultDestinations::ASXT());
    asxEntry.m_destinations.push_back(DefaultDestinations::CXA());
    fixApplicationEntries.push_back(asxEntry);
    FixApplicationEntry chixEntry;
    chixEntry.m_configPath = "chix.cfg";
    chixEntry.m_application = std::make_shared<ChixFixApplication>(
      Ref(*timeClient));
    chixEntry.m_destinations.push_back(DefaultDestinations::CHIX());
    chixEntry.m_destinations.push_back(DefaultDestinations::CX2());
    chixEntry.m_destinations.push_back(DefaultDestinations::TSX());
    fixApplicationEntries.push_back(chixEntry);
    FixApplicationEntry matchNowLiquidityProviderEntry;
    matchNowLiquidityProviderEntry.m_configPath = "matnlp.cfg";
    matchNowLiquidityProviderEntry.m_application =
      std::make_shared<MatchNowFixApplication>(Ref(*timeClient));
    matchNowLiquidityProviderEntry.m_destinations.push_back(
      DefaultDestinations::MATNLP());
    fixApplicationEntries.push_back(matchNowLiquidityProviderEntry);
    FixApplicationEntry matchNowMarketFlowEntry;
    matchNowMarketFlowEntry.m_configPath = "matnmf.cfg";
    matchNowMarketFlowEntry.m_application =
      std::make_shared<MatchNowFixApplication>(Ref(*timeClient));
    matchNowMarketFlowEntry.m_destinations.push_back(
      DefaultDestinations::MATNMF());
    fixApplicationEntries.push_back(matchNowMarketFlowEntry);
    FixApplicationEntry omegaEntry;
    omegaEntry.m_configPath = "omega.cfg";
    omegaEntry.m_application = std::make_shared<OmegaFixApplication>(
      Ref(*timeClient), Ref(*marketDataClient));
    omegaEntry.m_destinations.push_back(DefaultDestinations::LYNX());
    omegaEntry.m_destinations.push_back(DefaultDestinations::OMEGA());
    fixApplicationEntries.push_back(omegaEntry);
    FixApplicationEntry pureEntry;
    pureEntry.m_configPath = "pure.cfg";
    pureEntry.m_application = std::make_shared<CnsxFixApplication>(
      Ref(*timeClient));
    pureEntry.m_destinations.push_back(DefaultDestinations::PURE());
    pureEntry.m_destinations.push_back(DefaultDestinations::CSE());
    fixApplicationEntries.push_back(pureEntry);
    FixApplicationEntry neoeEntry;
    neoeEntry.m_configPath = "neoe.cfg";
    neoeEntry.m_application = std::make_shared<AequitasFixApplication>(
      Ref(*timeClient));
    neoeEntry.m_destinations.push_back(DefaultDestinations::NEOE());
    fixApplicationEntries.push_back(neoeEntry);
    FixApplicationEntry lekEntry;
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
  } catch(const std::exception& e) {
    cerr << "Unable to initialize FIX entry: " << e.what() << endl;
    return -1;
  }
  ApplicationFixOrderExecutionDriver fixOrderExecutionDriver{
    fixApplicationEntries};
  AsxtFeeTable asxtFeeTable;
  try {
    asxtFeeTable = ParseAsxFeeTable(GetNode(feeTableConfig, "au_equities"));
  } catch(const std::exception& e) {
    cerr << "Unable to initialize ASX fee table: " << e.what() << endl;
    return -1;
  }
  ConsolidatedTmxFeeTable tmxFeeTable;
  try {
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    tmxFeeTable = ParseConsolidatedTmxFeeTable(
      GetNode(feeTableConfig, "ca_equities"), marketDatabase);
  } catch(const std::exception& e) {
    cerr << "Unable to initialize TMX fee table: " << e.what() << endl;
    return -1;
  }
  ConsolidatedUsFeeTable usFeeTable;
  try {
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    usFeeTable = ParseConsolidatedUsFeeTable(
      GetNode(feeTableConfig, "us_equities"), marketDatabase);
  } catch(const std::exception& e) {
    cerr << "Unable to initialize US fee table: " << e.what() << endl;
    return -1;
  }
  ApplicationFeesCalculatorOrderExecutionDriver feesCalculator{
    &fixOrderExecutionDriver, asxtFeeTable, tmxFeeTable, usFeeTable};
  vector<unique_ptr<OrderSubmissionCheck>> checks;
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
    cerr << "Unable to initialize order submission checks: " << e.what() <<
      endl;
    return -1;
  }
  ApplicationOrderSubmissionCheckDriver orderSubmissionCheckDriver{
    &feesCalculator, std::move(checks)};
  ComplianceRuleSet<ApplicationComplianceClient::Client*,
    ApplicationServiceLocatorClient::Client*> complianceRuleSet{
    complianceClient.Get(), serviceLocatorClient.Get(),
    [&] (const ComplianceRuleEntry& entry) {
      return BuildComplianceRule(entry.GetSchema(), *marketDataClient,
        *definitionsClient, *timeClient);
    }};
  ApplicationComplianceCheckOrderExecutionDriver
    complianceCheckOrderExecutionDriver{&orderSubmissionCheckDriver,
    timeClient.get(), &complianceRuleSet};
  ApplicationManualOrderEntryDriver manualOrderExecutionDriver{
    DefaultDestinations::MOE(), &complianceCheckOrderExecutionDriver,
    administrationClient.Get()};
  auto sessionStartTime = ToUtcTime(Extract<ptime>(config, "session_start_time",
    pos_infin));
  OrderExecutionServerConnectionInitializer
    orderExecutionServerConnectionInitializer;
  try {
    orderExecutionServerConnectionInitializer.Initialize(
      GetNode(config, "server"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'server': " << e.what() << endl;
    return -1;
  }
  vector<MySqlConfig> mySqlConfigs;
  try {
    mySqlConfigs = MySqlConfig::ParseReplication(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'data_store': " << e.what() << endl;
    return -1;
  }
  auto accountSource =
    [&] (unsigned int id) {
      return serviceLocatorClient->LoadDirectoryEntry(id);
    };
  auto connectionBuilders = std::vector<SqlDataStore::ConnectionBuilder>();
  for(auto& mySqlConfig : mySqlConfigs) {
    connectionBuilders.emplace_back(
      [=] {
        return MySql::Connection(mySqlConfig.m_address.GetHost(),
          mySqlConfig.m_address.GetPort(), mySqlConfig.m_username,
          mySqlConfig.m_password, mySqlConfig.m_schema);
      });
  }
  auto dataStore = MakeReplicatedMySqlOrderExecutionDataStore(
    connectionBuilders, accountSource);
  OrderExecutionServletContainer orderExecutionServer{
    Initialize(serviceLocatorClient.Get(), Initialize(sessionStartTime,
    definitionsClient->LoadMarketDatabase(),
    definitionsClient->LoadDestinationDatabase(), timeClient.get(),
    serviceLocatorClient.Get(), uidClient.Get(), administrationClient.Get(),
    &manualOrderExecutionDriver, dataStore.get())),
    Initialize(orderExecutionServerConnectionInitializer.m_interface,
    Ref(socketThreadPool)),
    std::bind(factory<std::shared_ptr<LiveTimer>>{}, seconds{10},
    Ref(timerThreadPool))};
  try {
    orderExecutionServer.Open();
  } catch(const std::exception& e) {
    cerr << "Error opening server: " << e.what() << endl;
    return -1;
  }
  try {
    JsonObject orderExecutionService;
    orderExecutionService["addresses"] =
      ToString(orderExecutionServerConnectionInitializer.m_addresses);
    serviceLocatorClient->Register(
      orderExecutionServerConnectionInitializer.m_serviceName,
      orderExecutionService);
  } catch(const std::exception& e) {
    cerr << "Error registering service: " << e.what() << endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
