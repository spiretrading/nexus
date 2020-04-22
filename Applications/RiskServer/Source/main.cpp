#include <fstream>
#include <iostream>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/Parsers/Parse.hpp>
#include <Beam/Queues/AggregateQueue.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"
#include "Nexus/RiskService/RiskServlet.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Parsers;
using namespace Beam::Queries;
using namespace Beam::Routines;
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
using namespace Nexus::Queries;
using namespace Nexus::RiskService;
using namespace std;
using namespace TCLAP;

namespace {
  using ApplicationPortfolio =
    Portfolio<TrueAverageBookkeeper<Inventory<Position<Security>>>>;
  using ApplicationRiskStateMonitor =
    RiskStateMonitor<RiskStateTracker<ApplicationPortfolio,
    LiveNtpTimeClient*>, ApplicationAdministrationClient::Client*,
    ApplicationMarketDataClient::Client*, LiveTimer, LiveNtpTimeClient*>;
  using RiskServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<
    MetaRiskServlet<ApplicationAdministrationClient::Client*,
    ApplicationOrderExecutionClient::Client*, ApplicationRiskStateMonitor>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  struct RiskServerConnectionInitializer {
    string m_serviceName;
    IpAddress m_interface;
    vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void RiskServerConnectionInitializer::Initialize(const YAML::Node& config) {
    m_serviceName = Extract<string>(config, "service",
      RiskService::SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    vector<IpAddress> addresses;
    addresses.push_back(m_interface);
    m_addresses = Extract<vector<IpAddress>>(config, "addresses", addresses);
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "0.9-r" RISK_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
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
  ApplicationDefinitionsClient definitionsClient;
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the definitions service." << endl;
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
  ApplicationMarketDataClient marketDataClient;
  try {
    marketDataClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    marketDataClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the market data service." << endl;
    return -1;
  }
  ApplicationOrderExecutionClient orderExecutionClient;
  try {
    orderExecutionClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    orderExecutionClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the order execution service." << endl;
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
    auto ntpPool = Parse<vector<IpAddress>>(get<string>(
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
  RiskServerConnectionInitializer riskServerConnectionInitializer;
  try {
    riskServerConnectionInitializer.Initialize(GetNode(config, "server"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'server': " << e.what() << endl;
    return -1;
  }
  boost::optional<RiskServletContainer> riskServer;
  try {
    auto sessionStartTime = ToUtcTime(Extract<ptime>(config,
      "session_start_time", pos_infin));
    Beam::Queries::Range timeRange;
    if(sessionStartTime == pos_infin) {
      timeRange = Beam::Queries::Range::RealTime();
    } else {
      timeRange = Beam::Queries::Range(sessionStartTime,
        Beam::Queries::Sequence::Last());
    }
    auto accounts = serviceLocatorClient->LoadAllAccounts();
    vector<std::shared_ptr<QueueReader<const Order*>>> accountServletQueues;
    vector<std::shared_ptr<QueueReader<const Order*>>> accountTransitionQueues;
    for(const auto& account : accounts) {
      AccountQuery query;
      query.SetIndex(account);
      query.SetRange(timeRange);
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto accountServletQueue = std::make_shared<Queue<const Order*>>();
      orderExecutionClient->QueryOrderSubmissions(query, accountServletQueue);
      accountServletQueues.push_back(accountServletQueue);
      auto accountTransitionQueue = std::make_shared<Queue<const Order*>>();
      orderExecutionClient->QueryOrderSubmissions(query,
        accountTransitionQueue);
      accountTransitionQueues.push_back(accountTransitionQueue);
    }
    auto orderServletQueue = std::make_shared<AggregateQueue<const Order*>>(
      std::move(accountServletQueues));
    auto orderTransitionQueue = std::make_shared<AggregateQueue<const Order*>>(
      std::move(accountTransitionQueues));
    riskServer.emplace(Initialize(serviceLocatorClient.Get(),
      Initialize(orderServletQueue, administrationClient.Get(),
      orderExecutionClient.Get(), Initialize(administrationClient.Get(),
      marketDataClient.Get(), orderTransitionQueue,
      Initialize(seconds{1}, Ref(timerThreadPool)), timeClient.get(),
      definitionsClient->LoadMarketDatabase(),
      definitionsClient->LoadExchangeRates()),
      definitionsClient->LoadDestinationDatabase(),
      definitionsClient->LoadMarketDatabase())),
      Initialize(riskServerConnectionInitializer.m_interface,
      Ref(socketThreadPool)),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10),
      Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    cerr << "Error initializing server: " << e.what() << endl;
    return -1;
  }
  try {
    riskServer->Open();
  } catch(const std::exception& e) {
    cerr << "Error opening server: " << e.what() << endl;
    return -1;
  }
  try {
    auto service = JsonObject();
    service["addresses"] = lexical_cast<std::string>(
      Stream(riskServerConnectionInitializer.m_addresses));
    serviceLocatorClient->Register(
      riskServerConnectionInitializer.m_serviceName, service);
  } catch(const std::exception& e) {
    cerr << "Error registering service: " << e.what() << endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
