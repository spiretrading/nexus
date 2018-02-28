#include <cstdlib>
#include <fstream>
#include <iostream>
#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/MarketDataService/MarketDataClientUtilities.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"
#include "ReplayMarketDataFeedClient/ReplayMarketDataFeedClient.hpp"
#include "ReplayMarketDataFeedClient/Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
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
using namespace Nexus::MarketDataService;
using namespace std;
using namespace TCLAP;

namespace {
  using BaseMarketDataFeedClient = MarketDataFeedClient<string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  using ApplicationMarketDataFeedClient = ReplayMarketDataFeedClient<
    BaseMarketDataFeedClient, LiveNtpTimeClient*, std::unique_ptr<LiveTimer>>;

  struct ReplayConfig {
    int m_bboPartitions;
    int m_bookQuotePartitions;
    int m_marketQuotePartitions;
    int m_timeAndSalePartitions;
    ptime m_startTime;
    ptime m_endTime;
    time_duration m_sampling;
    std::vector<Security> m_securities;

    static ReplayConfig Parse(const YAML::Node& config) {
      ReplayConfig replayConfig;
      replayConfig.m_bboPartitions = Extract<int>(config, "bbo_partitions",
        boost::thread::hardware_concurrency());
      replayConfig.m_bookQuotePartitions = Extract<int>(config,
        "book_quote_partitions", boost::thread::hardware_concurrency());
      replayConfig.m_marketQuotePartitions = Extract<int>(config,
        "market_quote_partitions", boost::thread::hardware_concurrency());
      replayConfig.m_timeAndSalePartitions = Extract<int>(config,
        "time_and_sale_partitions", boost::thread::hardware_concurrency());
      replayConfig.m_sampling = Extract<time_duration>(config,
        "sampling", boost::posix_time::milliseconds(100));
      replayConfig.m_startTime = ToUtcTime(Extract<ptime>(config,
        "start_time"));
      replayConfig.m_endTime = ToUtcTime(Extract<ptime>(config, "end_time"));
      replayConfig.m_securities = Extract<vector<Security>>(config, "symbols");
      return replayConfig;
    }
  };

  bool MarketDataComparator(const MarketDataFeedMessage& lhs,
      const MarketDataFeedMessage& rhs) {
    auto visitor = [] (const auto& value) {
      return value->m_timestamp;
    };
    auto lhsTimestamp = boost::apply_visitor(visitor, lhs);
    auto rhsTimestamp = boost::apply_visitor(visitor, rhs);
    return lhsTimestamp < rhsTimestamp;
  }

  template<typename MarketDataType>
  void BuildReplayClients(const ReplayConfig& replayConfig, int partitions,
      const std::vector<IpAddress>& marketDataAddresses,
      ApplicationServiceClients& sourceServiceClients,
      ApplicationServiceClients& targetServiceClients,
      RefType<SocketThreadPool> socketThreadPool,
      RefType<TimerThreadPool> timerThreadPool,
      std::vector<std::unique_ptr<ApplicationMarketDataFeedClient>>&
      replayClients) {
    deque<MarketDataFeedMessage> data;
    auto securitiesPerPartition = replayConfig.m_securities.size() / partitions;
    for(std::size_t i = 0; i <= replayConfig.m_securities.size(); ++i) {
      if(i != 0 && i % securitiesPerPartition == 0) {
        std::sort(data.begin(), data.end(), MarketDataComparator);
        auto replayClient = std::make_unique<ApplicationMarketDataFeedClient>(
          Initialize(Initialize(marketDataAddresses, Ref(socketThreadPool)),
          SessionAuthenticator<ApplicationServiceLocatorClient::Client>(
          Ref(targetServiceClients.GetServiceLocatorClient())),
          Initialize(replayConfig.m_sampling, Ref(timerThreadPool)),
          Initialize(seconds(10), Ref(timerThreadPool))),
          &targetServiceClients.GetTimeClient(),
          targetServiceClients.BuildTimer(replayConfig.m_sampling),
          replayConfig.m_startTime, std::move(data));
        replayClients.push_back(std::move(replayClient));
        data.clear();
        if(i == replayConfig.m_securities.size()) {
          break;
        }
      }
      auto& security = replayConfig.m_securities[i];
      std::cout << security << std::endl;
      auto query = SecurityMarketDataQuery();
      query.SetIndex(security);
      query.SetRange(replayConfig.m_startTime, replayConfig.m_endTime);
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto queue = std::make_shared<Queue<MarketDataType>>();
      QueryMarketDataClient(sourceServiceClients.GetMarketDataClient(), query,
        queue);
      vector<MarketDataType> marketData;
      FlushQueue(queue, std::back_inserter(marketData));
      std::transform(marketData.begin(), marketData.end(),
        std::back_inserter(data),
        [&] (auto& marketData) {
          return MarketDataFeedMessage(IndexedValue<MarketDataType, Security>(
            std::move(marketData), security));
        });
    }
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "1.0-r" REPLAY_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2011 Eidolon Systems Inc."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  YAML::Node config;
  try {
    ifstream configStream{configFile.c_str()};
    if(!configStream.good()) {
      cerr << configFile << " not found." << endl;
      return -1;
    }
    YAML::Parser configParser{configStream};
    configParser.GetNextDocument(config);
  } catch(const YAML::ParserException& e) {
    cerr << "Invalid YAML at line " << (e.mark.line + 1) << ", " << "column " <<
      (e.mark.column + 1) << ": " << e.msg << endl;
    return -1;
  }
  ServiceLocatorClientConfig sourceConfig;
  try {
    sourceConfig = ServiceLocatorClientConfig::Parse(
      *config.FindValue("source"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'source': " << e.what() << endl;
    return -1;
  }
  ServiceLocatorClientConfig targetConfig;
  try {
    targetConfig = ServiceLocatorClientConfig::Parse(
      *config.FindValue("target"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'target': " << e.what() << endl;
    return -1;
  }
  ReplayConfig replayConfig;
  try {
    replayConfig = ReplayConfig::Parse(config);
  } catch(const std::exception& e) {
    cerr << "Error parsing replay parameters: " << e.what() << endl;
    return -1;
  }
  SocketThreadPool socketThreadPool;
  TimerThreadPool timerThreadPool;
  ApplicationServiceClients sourceServiceClients(sourceConfig.m_address,
    sourceConfig.m_username, sourceConfig.m_password, Ref(socketThreadPool),
    Ref(timerThreadPool));
  ApplicationServiceClients targetServiceClients(targetConfig.m_address,
    targetConfig.m_username, targetConfig.m_password, Ref(socketThreadPool),
    Ref(timerThreadPool));
  try {
    sourceServiceClients.Open();
    targetServiceClients.Open();
  } catch(const std::exception& e) {
    cerr << "Error logging in: " << e.what() << endl;
    return -1;
  }
  std::vector<IpAddress> marketDataAddresses;
  try {
    auto marketDataServices =
      targetServiceClients.GetServiceLocatorClient().Locate(
      MarketDataService::FEED_SERVICE_NAME);
    if(marketDataServices.empty()) {
      cerr << "No market data services available." << endl;
      return -1;
    }
    auto& marketDataService = marketDataServices.front();
    marketDataAddresses = FromString<vector<IpAddress>>(
      get<string>(marketDataService.GetProperties().At("addresses")));
  } catch(const std::exception& e) {
    cerr << "Exception caught: " << diagnostic_information(e) << flush;
    return -1;
  }
  std::vector<std::unique_ptr<ApplicationMarketDataFeedClient>> replayClients;
  BuildReplayClients<BboQuote>(replayConfig, replayConfig.m_bboPartitions,
    marketDataAddresses, sourceServiceClients, targetServiceClients,
    Ref(socketThreadPool), Ref(timerThreadPool), replayClients);
  BuildReplayClients<TimeAndSale>(replayConfig,
    replayConfig.m_timeAndSalePartitions, marketDataAddresses,
    sourceServiceClients, targetServiceClients, Ref(socketThreadPool),
    Ref(timerThreadPool), replayClients);
  BuildReplayClients<BookQuote>(replayConfig,
    replayConfig.m_bookQuotePartitions, marketDataAddresses,
    sourceServiceClients, targetServiceClients, Ref(socketThreadPool),
    Ref(timerThreadPool), replayClients);
  for(auto& replayClient : replayClients) {
    try {
      replayClient->Open();
    } catch(const std::exception& e) {
      cerr << "Exception caught: " << diagnostic_information(e) << flush;
      return -1;
    }
  }
  WaitForKillEvent();
  return 0;
}
