#include <cstdlib>
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
#include <Beam/Sql/SqlConnection.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
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
using namespace Viper;

namespace {
  using SqlDataStore = SqlHistoricalDataStore<
    SqlConnection<Sqlite3::Connection>>;
  using BaseMarketDataFeedClient = MarketDataFeedClient<std::string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
      SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  using ApplicationMarketDataFeedClient = ReplayMarketDataFeedClient<
    BaseMarketDataFeedClient, SqlDataStore*, LiveNtpTimeClient*, LiveTimer>;

  auto ParseSecurities(const std::string& path,
      const MarketDatabase& marketDatabase) {
    return TryOrNest([&] {
      auto config = Require(LoadFile, path);
      auto securitiesNode = GetNode(config, "securities");
      auto securities = std::vector<Security>();
      for(auto item : securitiesNode) {
        auto symbol = GetNode(item, "symbol").as<std::string>();
        auto security = ParseSecurity(symbol, marketDatabase);
        if(security == Security()) {
          throw std::runtime_error("Invalid security: " + symbol);
        }
        securities.push_back(security);
      }
      return securities;
    }, std::runtime_error("Failed to parse securities."));
  }

  auto BuildReplayClients(const YAML::Node& config,
      std::vector<Security> securities, SqlDataStore* dataStore,
      const std::vector<IpAddress>& addresses,
      ApplicationServiceLocatorClient& serviceLocatorClient,
      LiveNtpTimeClient* timeClient) {
    return TryOrNest([&] {
      auto sampling = Extract<time_duration>(config, "sampling");
      auto startTime = Extract<ptime>(config, "start_time");
      auto clientCount = Extract<int>(config, "client_count");
      auto chunks = static_cast<int>(securities.size()) / clientCount;
      if(securities.size() % clientCount != 0) {
        ++chunks;
      }
      auto timerBuilder = [=] (auto duration) {
        return std::make_unique<LiveTimer>(duration);
      };
      auto replayClients =
        std::vector<std::unique_ptr<ApplicationMarketDataFeedClient>>();
      for(auto i = 0; i < clientCount; ++i) {
        auto securitySubset = std::vector<Security>();
        securitySubset.insert(securitySubset.end(),
          std::min(securities.begin() + i * chunks, securities.end()),
          std::min(securities.begin() + (i + 1) * chunks, securities.end()));
        replayClients.emplace_back(std::make_unique<
          ApplicationMarketDataFeedClient>(std::move(securitySubset), startTime,
            Initialize(Initialize(addresses),
              SessionAuthenticator(serviceLocatorClient.Get()),
              Initialize(sampling), Initialize(seconds(10))), dataStore,
            timeClient, timerBuilder));
      }
      return replayClients;
    }, std::runtime_error("Failed to build replay clients."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "0.9-r" REPLAY_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto definitionsClient = ApplicationDefinitionsClient(
      serviceLocatorClient.Get());
    auto timeClient = MakeLiveNtpTimeClientFromServiceLocator(
      *serviceLocatorClient);
    auto dataStorePath = Extract<std::string>(config, "data_store");
    auto historicalDataStore = SqlDataStore([=] {
      return SqlConnection(Sqlite3::Connection(dataStorePath));
    });
    auto securities = ParseSecurities(Extract<std::string>(config,
      "securities_path", "securities.yml"),
      definitionsClient->LoadMarketDatabase());
    auto marketDataServices = serviceLocatorClient->Locate(
      MarketDataService::FEED_SERVICE_NAME);
    if(marketDataServices.empty()) {
      throw std::runtime_error("No market data services available.");
    }
    auto& marketDataService = marketDataServices.front();
    auto marketDataAddresses = Parse<std::vector<IpAddress>>(
      get<std::string>(marketDataService.GetProperties().At("addresses")));
    auto feedClients = BuildReplayClients(config, securities,
      &historicalDataStore, marketDataAddresses, serviceLocatorClient,
      timeClient.get());
    WaitForKillEvent();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
