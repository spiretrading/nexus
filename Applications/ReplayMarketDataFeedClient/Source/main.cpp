#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string_view>
#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
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
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/SqlHistoricalDataStore.hpp"
#include "ReplayMarketDataFeedClient/ReplayMarketDataFeedClient.hpp"
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
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace TCLAP;
using namespace Viper;

namespace {
  using SqlDataStore = SqlHistoricalDataStore<Sqlite3::Connection>;
  using BaseMarketDataFeedClient = MarketDataFeedClient<std::string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  using ApplicationMarketDataFeedClient = ReplayMarketDataFeedClient<
    BaseMarketDataFeedClient, SqlDataStore*, LiveNtpTimeClient*, LiveTimer>;

  auto ParseSecurities(const std::string& path,
      const MarketDatabase& marketDatabase) {
    auto config = Require(LoadFile, path);
    auto securitiesNode = GetNode(config, "securities");
    auto securities = std::vector<Security>();
    for(auto item : securitiesNode) {
      auto symbol = GetNode(item, "symbol");
      securities.push_back(ParseSecurity(symbol.as<std::string>(),
        marketDatabase));
    }
    return securities;
  }

  auto BuildReplayClients(const YAML::Node& config,
      std::vector<Security> securities, SqlDataStore* dataStore,
      const std::vector<IpAddress>& addresses,
      Ref<ApplicationServiceLocatorClient> serviceLocatorClient,
      LiveNtpTimeClient* timeClient, Ref<SocketThreadPool> socketThreadPool,
      Ref<TimerThreadPool> timerThreadPool) {
    auto sampling = Extract<time_duration>(config, "sampling");
    auto startTime = Extract<ptime>(config, "start_time");
    auto client_count = Extract<int>(config, "client_count");
    auto chunks = static_cast<int>(securities.size()) / client_count;
    if(securities.size() % client_count != 0) {
      ++chunks;
    }
    auto timerBuilder =
      [=] (auto duration) mutable {
        return std::make_unique<LiveTimer>(duration, Ref(timerThreadPool));
      };
    std::vector<std::unique_ptr<ApplicationMarketDataFeedClient>> replayClients;
    for(auto i = 0; i < client_count; ++i) {
      auto securitySubset = std::vector<Security>();
      securitySubset.insert(securitySubset.end(),
        std::min(securities.begin() + i * chunks, securities.end()),
        std::min(securities.begin() + (i + 1) * chunks, securities.end()));
      replayClients.emplace_back(std::make_unique<
        ApplicationMarketDataFeedClient>(std::move(securitySubset), startTime,
        Initialize(Initialize(addresses, Ref(socketThreadPool)),
        SessionAuthenticator<ApplicationServiceLocatorClient::Client>(
        Ref(**serviceLocatorClient)), Initialize(sampling,
        Ref(timerThreadPool)), Initialize(seconds(10), Ref(timerThreadPool))),
        dataStore, timeClient, timerBuilder));
    }
    return replayClients;
  }
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "1.0-r" REPLAY_MARKET_DATA_FEED_CLIENT_VERSION
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
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_address,
      Ref(socketThreadPool), Ref(timerThreadPool));
    serviceLocatorClient->SetCredentials(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password);
    serviceLocatorClient->Open();
  } catch(const std::exception& e) {
    std::cerr << "Error logging in: " << e.what() << std::endl;
    return -1;
  }
  auto definitionsClient = ApplicationDefinitionsClient();
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the definitions service." << std::endl;
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
    auto ntpPool = FromString<std::vector<IpAddress>>(get<std::string>(
      timeService.GetProperties().At("addresses")));
    timeClient = MakeLiveNtpTimeClient(ntpPool, Ref(socketThreadPool),
      Ref(timerThreadPool));
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize NTP client: " << e.what() << std::endl;
    return -1;
  }
  try {
    timeClient->Open();
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the time service." << std::endl;
    return -1;
  }
  auto dataStorePath = Extract<std::string>(config, "data_store");
  auto historicalDataStore = SqlDataStore(
    [=] {
      return Sqlite3::Connection(dataStorePath);
    });
  auto feedClients =
    std::vector<std::unique_ptr<ApplicationMarketDataFeedClient>>();
  auto securities = ParseSecurities(Extract<std::string>(
    config, "securities_path", "securities.yml"),
    definitionsClient->LoadMarketDatabase());
  securities.push_back(Security("MSFT", DefaultMarkets::NASDAQ(),
    DefaultCountries::US()));
  securities.push_back(Security("ABX", DefaultMarkets::TSX(),
    DefaultCountries::CA()));
  try {
    auto marketDataServices = serviceLocatorClient->Locate(
      MarketDataService::FEED_SERVICE_NAME);
    if(marketDataServices.empty()) {
      std::cerr << "No market data services available." << std::endl;
      return -1;
    }
    auto& marketDataService = marketDataServices.front();
    auto marketDataAddresses = FromString<std::vector<IpAddress>>(
      get<std::string>(marketDataService.GetProperties().At("addresses")));
    feedClients = BuildReplayClients(config, securities, &historicalDataStore,
      marketDataAddresses, Ref(serviceLocatorClient), timeClient.get(),
      Ref(socketThreadPool), Ref(timerThreadPool));
  } catch(const YAML::ParserException& e) {
    std::cerr << "Invalid YAML at line " << (e.mark.line + 1) << ", " <<
      "column " << (e.mark.column + 1) << ": " << e.msg << std::endl;
    return -1;
  } catch(const std::exception& e) {
    std::cerr << "Exception caught: " << diagnostic_information(e);
    return -1;
  }
  try {
    for(auto& client : feedClients) {
      client->Open();
    }
  } catch(const std::exception& e) {
    std::cerr << "Exception caught: " << diagnostic_information(e);
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
