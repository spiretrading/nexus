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
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
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

namespace {
  using BaseMarketDataFeedClient = MarketDataFeedClient<std::string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
      SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  using ApplicationMarketDataFeedClient = SimulationMarketDataFeedClient<
    BaseMarketDataFeedClient, LiveNtpTimeClient*, LiveTimer, LiveTimer,
    LiveTimer>;

  std::vector<Security> ParseSecurities(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    return TryOrNest([&] {
      auto securities = std::vector<Security>();
      for(auto& item : config) {
        auto symbol = item.as<std::string>();
        auto security = ParseSecurity(symbol, marketDatabase);
        if(security == Security()) {
          throw std::runtime_error("Invalid security: " + symbol);
        }
        securities.push_back(security);
      }
      return securities;
    }, std::runtime_error("Failed to parse securities."));
  }

  std::vector<std::unique_ptr<ApplicationMarketDataFeedClient>>
      BuildMockFeedClients(const YAML::Node& config,
      const MarketDatabase& marketDatabase,
      const std::vector<IpAddress>& addresses,
      ApplicationMarketDataClient& marketDataClient,
      ApplicationServiceLocatorClient& serviceLocatorClient,
      LiveNtpTimeClient& timeClient) {
    return TryOrNest([&] {
      auto feedClients =
        std::vector<std::unique_ptr<ApplicationMarketDataFeedClient>>();
      auto securities = ParseSecurities(GetNode(config, "symbols"),
        marketDatabase);
      auto feedCount = std::min<int>(Extract<int>(config, "feeds"),
        securities.size());
      auto securitiesPerFeed = securities.size() / feedCount;
      auto bboPeriod = Extract<time_duration>(config, "bbo_period");
      auto marketQuotePeriod = Extract<time_duration>(config,
        "market_quote_period");
      auto timeAndSalesPeriod = Extract<time_duration>(config,
        "time_and_sales_period");
      auto sampling = Extract<time_duration>(config, "sampling");
      for(auto i = 0; i < feedCount; ++i) {
        auto feedSecurities = std::vector<Security>();
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
            marketDatabase, *marketDataClient, Initialize(Initialize(addresses),
              SessionAuthenticator(serviceLocatorClient.Get()),
              Initialize(sampling), Initialize(seconds(10))), &timeClient,
            Initialize(bboPeriod), Initialize(marketQuotePeriod),
            Initialize(timeAndSalesPeriod));
        feedClients.push_back(std::move(applicationMarketDataFeed));
      }
      return feedClients;
    }, std::runtime_error("Failed to build feed clients."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" SIMULATION_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto serviceLocatorClient =
      MakeApplicationServiceLocatorClient(GetNode(config, "service_locator"));
    auto definitionsClient =
      ApplicationDefinitionsClient(serviceLocatorClient.Get());
    auto timeClient =
      MakeLiveNtpTimeClientFromServiceLocator(*serviceLocatorClient);
    auto marketDataServices =
      serviceLocatorClient->Locate(MarketDataService::FEED_SERVICE_NAME);
    if(marketDataServices.empty()) {
      throw std::runtime_error("No market data services available.");
    }
    auto& marketDataService = marketDataServices.front();
    auto marketDataAddresses = Parse<std::vector<IpAddress>>(
      get<std::string>(marketDataService.GetProperties().At("addresses")));
    auto marketDataClient =
      ApplicationMarketDataClient(serviceLocatorClient.Get());
    auto feedClients =
      BuildMockFeedClients(config, definitionsClient->LoadMarketDatabase(),
        marketDataAddresses, marketDataClient, serviceLocatorClient,
        *timeClient);
    WaitForKillEvent();
    serviceLocatorClient->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
