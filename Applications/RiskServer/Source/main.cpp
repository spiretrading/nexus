#include <fstream>
#include <iostream>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/Parsers/Parse.hpp>
#include <Beam/Queues/ConverterQueueReader.hpp>
#include <Beam/Queues/FilteredQueueReader.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Sql/MySqlConfig.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
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
using namespace Beam::Parsers;
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
using namespace TCLAP;
using namespace Viper;

namespace {
  using ApplicationDataStore = SqlRiskDataStore<MySql::Connection>;
  using RiskServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<
    MetaRiskServlet<ApplicationAdministrationClient::Client*,
    ApplicationMarketDataClient::Client*,
    ApplicationOrderExecutionClient::Client*, LiveTimer,
    std::unique_ptr<LiveNtpTimeClient>, ApplicationDataStore*>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  struct RiskServerConnectionInitializer {
    std::string m_serviceName;
    IpAddress m_interface;
    std::vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void RiskServerConnectionInitializer::Initialize(const YAML::Node& config) {
    m_serviceName = Extract<std::string>(config, "service",
      RiskService::SERVICE_NAME);
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
    auto cmd = CmdLine("", ' ', "0.9-r" RISK_SERVER_VERSION
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
  auto socketThreadPool = SocketThreadPool();
  auto timerThreadPool = TimerThreadPool();
  auto serviceLocatorClient = ApplicationServiceLocatorClient();
  try {
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password,
      serviceLocatorClientConfig.m_address, Ref(socketThreadPool),
      Ref(timerThreadPool));
  } catch(const std::exception& e) {
    std::cerr << "Error logging in: " << e.what() << std::endl;
    return -1;
  }
  auto definitionsClient = ApplicationDefinitionsClient();
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the definitions service." << std::endl;
    return -1;
  }
  auto administrationClient = ApplicationAdministrationClient();
  try {
    administrationClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
  } catch(const std::exception& e) {
    std::cerr << "Error connecting to the administration service: " <<
      e.what() << std::endl;
    return -1;
  }
  auto marketDataClient = ApplicationMarketDataClient();
  try {
    marketDataClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the market data service." << std::endl;
    return -1;
  }
  auto orderExecutionClient = ApplicationOrderExecutionClient();
  try {
    orderExecutionClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the order execution service." <<
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
      timeClient = MakeLiveNtpTimeClient(ntpPool, Ref(socketThreadPool),
        Ref(timerThreadPool));
    } catch(const std::exception&) {
      std::cerr << "NTP service unavailable." << std::endl;
      return -1;
    }
  } catch(const  std::exception& e) {
    std::cerr << "Unable to initialize NTP client: " << e.what() << std::endl;
    return -1;
  }
  auto mySqlConfig = MySqlConfig();
  try {
    mySqlConfig = MySqlConfig::Parse(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'data_store': " << e.what() <<
      std::endl;
    return -1;
  }
  auto dataStore = ApplicationDataStore(std::make_unique<MySql::Connection>(
    mySqlConfig.m_address.GetHost(), mySqlConfig.m_address.GetPort(),
    mySqlConfig.m_username, mySqlConfig.m_password, mySqlConfig.m_schema));
  auto exchangeRates = std::vector<ExchangeRate>();
  try {
    exchangeRates = definitionsClient->LoadExchangeRates();
  } catch(const std::exception& e) {
    std::cerr << "Error loading exchange rates: " << e.what() << std::endl;
    return -1;
  }
  auto markets = optional<MarketDatabase>();
  try {
    markets = definitionsClient->LoadMarketDatabase();
  } catch(const std::exception& e) {
    std::cerr << "Error loading markets database: " << e.what() << std::endl;
    return -1;
  }
  auto destinations = optional<DestinationDatabase>();
  try {
    destinations = definitionsClient->LoadDestinationDatabase();
  } catch(const std::exception& e) {
    std::cerr << "Error loading destination database: " << e.what() <<
      std::endl;
    return -1;
  }
  auto riskServerConnectionInitializer = RiskServerConnectionInitializer();
  try {
    riskServerConnectionInitializer.Initialize(GetNode(config, "server"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'server': " << e.what() << std::endl;
    return -1;
  }
  auto accounts = std::make_shared<Queue<AccountUpdate>>();
  serviceLocatorClient->MonitorAccounts(accounts);
  auto riskServer = optional<RiskServletContainer>();
  try {
    riskServer.emplace(Initialize(serviceLocatorClient.Get(),
      Initialize(MakeConverterQueueReader(MakeFilteredQueueReader(
      std::move(accounts), [] (const auto& update) {
        return update.m_type == AccountUpdate::Type::ADDED;
      }),
      [] (const auto& update) {
        return update.m_account;
      }), administrationClient.Get(), marketDataClient.Get(),
      orderExecutionClient.Get(),
      [&] {
        return std::make_unique<LiveTimer>(seconds(1), Ref(timerThreadPool));
      }, std::move(timeClient), &dataStore, std::move(exchangeRates),
      std::move(*markets), std::move(*destinations))), Initialize(
      riskServerConnectionInitializer.m_interface, Ref(socketThreadPool)),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10),
      Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    std::cerr << "Error opening server: " << e.what() << std::endl;
    return -1;
  }
  try {
    auto service = JsonObject();
    service["addresses"] = lexical_cast<std::string>(
      Stream(riskServerConnectionInitializer.m_addresses));
    serviceLocatorClient->Register(
      riskServerConnectionInitializer.m_serviceName, service);
  } catch(const std::exception& e) {
    std::cerr << "Error registering service: " << e.what() << std::endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
