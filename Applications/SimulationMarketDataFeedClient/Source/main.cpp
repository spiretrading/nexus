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
#include "Nexus/MarketDataService/ServiceMarketDataFeedClient.hpp"
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

namespace {
  using BaseMarketDataFeedClient = ServiceMarketDataFeedClient<
    std::string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
      SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  using ApplicationMarketDataFeedClient = SimulationMarketDataFeedClient<
    BaseMarketDataFeedClient, LiveNtpTimeClient*, LiveTimer, LiveTimer,
    LiveTimer>;

  std::vector<Security> parse_securities(
      const YAML::Node& config, const VenueDatabase& venues) {
    return TryOrNest([&] {
      auto securities = std::vector<Security>();
      for(auto& item : config) {
        auto symbol = item.as<std::string>();
        auto security = parse_security(symbol, venues);
        if(!security) {
          throw std::runtime_error("Invalid security: " + symbol);
        }
        securities.push_back(security);
      }
      return securities;
    }, std::runtime_error("Failed to parse securities."));
  }

  std::vector<std::unique_ptr<ApplicationMarketDataFeedClient>>
      build_mock_feed_clients(
        const YAML::Node& config, const VenueDatabase& venues,
        const std::vector<IpAddress>& addresses,
        ApplicationMarketDataClient& market_data_client,
        ApplicationServiceLocatorClient& service_locator_client,
        LiveNtpTimeClient& time_client) {
    return TryOrNest([&] {
      auto feed_clients =
        std::vector<std::unique_ptr<ApplicationMarketDataFeedClient>>();
      auto securities =
        parse_securities(GetNode(config, "symbols"), venues);
      auto feed_count =
        std::min<int>(Extract<int>(config, "feeds"), securities.size());
      auto securities_per_feed = securities.size() / feed_count;
      auto bbo_period = Extract<time_duration>(config, "bbo_period");
      auto book_quote_period =
        Extract<time_duration>(config, "book_quote_period");
      auto time_and_sales_period =
        Extract<time_duration>(config, "time_and_sales_period");
      auto sampling = Extract<time_duration>(config, "sampling");
      for(auto i = 0; i < feed_count; ++i) {
        auto feed_securities = std::vector<Security>();
        if(i < feed_count - 1) {
          feed_securities.insert(feed_securities.end(), securities.begin() +
            i * securities_per_feed, securities.begin() +
              (i + 1) * securities_per_feed);
        } else {
          feed_securities.insert(feed_securities.end(), securities.begin() +
            i * securities_per_feed, securities.end());
        }
        auto application_market_data_feed =
          std::make_unique<ApplicationMarketDataFeedClient>(feed_securities,
            venues, *market_data_client, Initialize(Initialize(addresses),
              SessionAuthenticator(service_locator_client.Get()),
              Initialize(sampling), Initialize(seconds(10))), &time_client,
            Initialize(bbo_period), Initialize(book_quote_period),
            Initialize(time_and_sales_period));
        feed_clients.push_back(std::move(application_market_data_feed));
      }
      return feed_clients;
    }, std::runtime_error("Failed to build feed clients."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" SIMULATION_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto service_locator_client =
      MakeApplicationServiceLocatorClient(GetNode(config, "service_locator"));
    auto definitions_client =
      ApplicationDefinitionsClient(service_locator_client.Get());
    auto time_client =
      MakeLiveNtpTimeClientFromServiceLocator(*service_locator_client);
    auto market_data_services =
      service_locator_client->Locate(MARKET_DATA_FEED_SERVICE_NAME);
    if(market_data_services.empty()) {
      throw std::runtime_error("No market data services available.");
    }
    auto& market_data_service = market_data_services.front();
    auto market_data_addresses = Parse<std::vector<IpAddress>>(
      get<std::string>(market_data_service.GetProperties().At("addresses")));
    auto market_data_client =
      ApplicationMarketDataClient(service_locator_client.Get());
    auto feed_clients =
      build_mock_feed_clients(config, definitions_client->load_venue_database(),
        market_data_addresses, market_data_client, service_locator_client,
        *time_client);
    WaitForKillEvent();
    service_locator_client->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
