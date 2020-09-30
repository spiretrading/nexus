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
#include <Beam/Sql/SqlConnection.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/UidService/ApplicationDefinitions.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
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
#include "SimulationOrderExecutionServer/SimulationOrderExecutionDriver.hpp"
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
using namespace TCLAP;
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
    ApplicationUidClient::Client*, ApplicationSimulationOrderExecutionDriver*>;
  using ApplicationOrderSubmissionCheckDriver = OrderSubmissionCheckDriver<
    ApplicationInternalMatchingOrderExecutionDriver*>;
  using ApplicationComplianceCheckOrderExecutionDriver =
    ComplianceCheckOrderExecutionDriver<ApplicationOrderSubmissionCheckDriver*,
    LiveNtpTimeClient*, ComplianceRuleSet<ApplicationComplianceClient::Client*,
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
  try {
    auto cmd = CmdLine("", ' ',
      "0.9-r" SIMULATION_ORDER_EXECUTION_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto configArg = ValueArg<std::string>("c", "config", "Configuration file",
      false, "config.yml", "path");
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() <<
      std::endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
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
    std::cerr << "Error connecting to the UID service: " << e.what() <<
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
    try {
      timeClient = MakeLiveNtpTimeClient(ntpPool);
    } catch(const std::exception&) {
      std::cerr << "NTP service unavailable." << std::endl;
      return -1;
    }
  } catch(const  std::exception& e) {
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
  auto marketDataClient = ApplicationMarketDataClient();
  try {
    marketDataClient.BuildSession(Ref(*serviceLocatorClient));
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the market data service." << std::endl;
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
  auto simulationOrderExecutionDriver =
    ApplicationSimulationOrderExecutionDriver(marketDataClient.Get(),
    timeClient.get());
  auto internalMatchingOrderExecutionDriver =
    ApplicationInternalMatchingOrderExecutionDriver(
    serviceLocatorClient->GetAccount(), Initialize(), marketDataClient.Get(),
    timeClient.get(), uidClient.Get(), &simulationOrderExecutionDriver);
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
    &internalMatchingOrderExecutionDriver, std::move(checks));
  auto complianceRuleSet = ComplianceRuleSet(complianceClient.Get(),
    serviceLocatorClient.Get(), [&] (const auto& entry) {
    return BuildComplianceRule(entry.GetSchema(), *marketDataClient,
      *definitionsClient, *timeClient);
  });
  auto complianceCheckOrderExecutionDriver =
    ApplicationComplianceCheckOrderExecutionDriver(&orderSubmissionCheckDriver,
    timeClient.get(), &complianceRuleSet);
  auto mySqlConfigs = std::vector<MySqlConfig>();
  try {
    mySqlConfigs = MySqlConfig::ParseReplication(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'data_store': " << e.what() <<
      std::endl;
    return -1;
  }
  auto sessionStartTime = ptime();
  try {
    sessionStartTime = Extract<ptime>(config, "session_start_time", pos_infin);
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'session_start_time': " << e.what() <<
      std::endl;
    return -1;
  }
  auto orderExecutionServerConnectionInitializer =
    OrderExecutionServerConnectionInitializer();
  try {
    orderExecutionServerConnectionInitializer.Initialize(
      GetNode(config, "server"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'server': " << e.what() << std::endl;
    return -1;
  }
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
  auto orderExecutionServer = optional<OrderExecutionServletContainer>();
  try {
    orderExecutionServer.emplace(Initialize(serviceLocatorClient.Get(),
      Initialize(sessionStartTime, definitionsClient->LoadMarketDatabase(),
      definitionsClient->LoadDestinationDatabase(), timeClient.get(),
      serviceLocatorClient.Get(), uidClient.Get(), administrationClient.Get(),
      &complianceCheckOrderExecutionDriver, dataStore.get())),
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
