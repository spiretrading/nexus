#include <cstdlib>
#include <fstream>
#include <iostream>
#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/Parsers/Parse.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "SimulationMarketDataFeedClient/SimulationMarketDataFeedClient.hpp"
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
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace std;
using namespace TCLAP;

namespace {
  using BaseMarketDataFeedClient = MarketDataFeedClient<string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  using ApplicationMarketDataFeedClient = SimulationMarketDataFeedClient<
    BaseMarketDataFeedClient, LiveNtpTimeClient*, LiveTimer, LiveTimer,
    LiveTimer>;

  vector<Security> ParseSecurities(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    vector<Security> securities;
    for(auto& item : config) {
      auto security = ParseSecurity(item.as<string>(), marketDatabase);
      securities.push_back(security);
    }
    return securities;
  }

  vector<unique_ptr<ApplicationMarketDataFeedClient>> BuildMockFeedClients(
      const YAML::Node& config, const MarketDatabase& marketDatabase,
      const vector<IpAddress>& addresses,
      Ref<ApplicationServiceLocatorClient> serviceLocatorClient,
      LiveNtpTimeClient* timeClient, Ref<SocketThreadPool> socketThreadPool,
      Ref<TimerThreadPool> timerThreadPool) {
    vector<unique_ptr<ApplicationMarketDataFeedClient>> feedClients;
    auto securities = ParseSecurities(GetNode(config, "symbols"),
      marketDatabase);
    auto feedCount = min<int>(Extract<int>(config, "feeds"), securities.size());
    auto securitiesPerFeed = securities.size() / feedCount;
    auto bboPeriod = Extract<time_duration>(config, "bbo_period");
    auto marketQuotePeriod = Extract<time_duration>(config,
      "market_quote_period");
    auto timeAndSalesPeriod = Extract<time_duration>(config,
      "time_and_sales_period");
    auto sampling = Extract<time_duration>(config, "sampling");
    for(auto i = 0; i < feedCount; ++i) {
      vector<Security> feedSecurities;
      if(i < feedCount - 1) {
        feedSecurities.insert(feedSecurities.end(), securities.begin() +
          i * securitiesPerFeed, securities.begin() +
          (i + 1) * securitiesPerFeed);
      } else {
        feedSecurities.insert(feedSecurities.end(), securities.begin() +
          i * securitiesPerFeed, securities.end());
      }
      auto applicationMarketDataFeed =
        std::make_unique<ApplicationMarketDataFeedClient>(feedSecurities,
        marketDatabase, Initialize(Initialize(addresses, Ref(socketThreadPool)),
        SessionAuthenticator<ApplicationServiceLocatorClient::Client>(
          Ref(**serviceLocatorClient)),
        Initialize(sampling, Ref(timerThreadPool)),
        Initialize(seconds(10), Ref(timerThreadPool))), timeClient,
        Initialize(bboPeriod, Ref(timerThreadPool)),
        Initialize(marketQuotePeriod, Ref(timerThreadPool)),
        Initialize(timeAndSalesPeriod, Ref(timerThreadPool)));
      feedClients.push_back(std::move(applicationMarketDataFeed));
    }
    return feedClients;
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "1.0-r" SIMULATION_MARKET_DATA_FEED_CLIENT_VERSION
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
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize NTP client: " << e.what() << endl;
    return -1;
  }
  try {
    timeClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the time service." << endl;
    return -1;
  }
  vector<unique_ptr<ApplicationMarketDataFeedClient>> feedClients;
  try {
    auto marketDataServices = serviceLocatorClient->Locate(
      MarketDataService::FEED_SERVICE_NAME);
    if(marketDataServices.empty()) {
      cerr << "No market data services available." << endl;
      return -1;
    }
    auto& marketDataService = marketDataServices.front();
    auto marketDataAddresses = Parse<vector<IpAddress>>(
      get<string>(marketDataService.GetProperties().At("addresses")));
    feedClients = BuildMockFeedClients(config,
      definitionsClient->LoadMarketDatabase(), marketDataAddresses,
      Ref(serviceLocatorClient), timeClient.get(), Ref(socketThreadPool),
      Ref(timerThreadPool));
  } catch(const YAML::ParserException& e) {
    cerr << "Invalid YAML at line " << (e.mark.line + 1) << ", " << "column " <<
      (e.mark.column + 1) << ": " << e.msg << endl;
    return -1;
  } catch(const std::exception& e) {
    cerr << "Exception caught: " << diagnostic_information(e) << flush;
    return -1;
  }
  try {
    for(auto& client : feedClients) {
      client->Open();
    }
  } catch(const std::exception& e) {
    cerr << "Exception caught: " << diagnostic_information(e) << flush;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
