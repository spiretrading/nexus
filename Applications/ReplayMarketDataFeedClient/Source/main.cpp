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
#include "Nexus/MarketDataService/ServiceMarketDataFeedClient.hpp"
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
using namespace Viper;

namespace {
  using SqlDataStore =
    SqlHistoricalDataStore<SqlConnection<Sqlite3::Connection>>;
  using BaseMarketDataFeedClient = ServiceMarketDataFeedClient<
    std::string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
      SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  using ApplicationMarketDataFeedClient = ReplayMarketDataFeedClient<
    BaseMarketDataFeedClient, SqlDataStore*, LiveNtpTimeClient*, LiveTimer>;

  auto parse_securities(const std::string& path, const VenueDatabase& venues) {
    return TryOrNest([&] {
      auto config = Require(LoadFile, path);
      auto securities_node = GetNode(config, "securities");
      auto securities = std::vector<Security>();
      for(auto item : securities_node) {
        auto symbol = GetNode(item, "symbol").as<std::string>();
        auto security = parse_security(symbol, venues);
        if(!security) {
          throw std::runtime_error("Invalid security: " + symbol);
        }
        securities.push_back(security);
      }
      return securities;
    }, std::runtime_error("Failed to parse securities."));
  }

  auto build_replay_clients(const YAML::Node& config,
      std::vector<Security> securities, SqlDataStore* data_store,
      const std::vector<IpAddress>& addresses,
      ApplicationServiceLocatorClient& service_locator_client,
      LiveNtpTimeClient* time_client) {
    return TryOrNest([&] {
      auto sampling = Extract<time_duration>(config, "sampling");
      auto start_time = Extract<ptime>(config, "start_time");
      auto client_count = Extract<int>(config, "client_count");
      auto chunks = static_cast<int>(securities.size()) / client_count;
      if(securities.size() % client_count != 0) {
        ++chunks;
      }
      auto timer_builder = [=] (auto duration) {
        return std::make_unique<LiveTimer>(duration);
      };
      auto replay_clients =
        std::vector<std::unique_ptr<ApplicationMarketDataFeedClient>>();
      for(auto i = 0; i < client_count; ++i) {
        auto security_subset = std::vector<Security>();
        security_subset.insert(security_subset.end(),
          std::min(securities.begin() + i * chunks, securities.end()),
          std::min(securities.begin() + (i + 1) * chunks, securities.end()));
        replay_clients.emplace_back(
          std::make_unique<ApplicationMarketDataFeedClient>(
            std::move(security_subset), start_time, Initialize(
              Initialize(addresses),
              SessionAuthenticator(service_locator_client.Get()),
              Initialize(sampling), Initialize(seconds(10))), data_store,
            time_client, timer_builder));
      }
      return replay_clients;
    }, std::runtime_error("Failed to build replay clients."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "0.9-r" REPLAY_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto service_locator_client = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto definitions_client =
      ApplicationDefinitionsClient(service_locator_client.Get());
    auto time_client =
      MakeLiveNtpTimeClientFromServiceLocator(*service_locator_client);
    auto data_store_path = Extract<std::string>(config, "data_store");
    auto venues = definitions_client->load_venue_database();
    auto historical_data_store = SqlDataStore(venues, [=] {
      return SqlConnection(Sqlite3::Connection(data_store_path));
    });
    auto securities = parse_securities(Extract<std::string>(
      config, "securities_path", "securities.yml"), venues);
    auto market_data_services =
      service_locator_client->Locate(MARKET_DATA_FEED_SERVICE_NAME);
    if(market_data_services.empty()) {
      throw std::runtime_error("No market data services available.");
    }
    auto& market_data_service = market_data_services.front();
    auto market_data_addresses = Parse<std::vector<IpAddress>>(
      get<std::string>(market_data_service.GetProperties().At("addresses")));
    auto feed_clients = build_replay_clients(config, securities,
      &historical_data_store, market_data_addresses, service_locator_client,
      time_client.get());
    WaitForKillEvent();
    service_locator_client->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
