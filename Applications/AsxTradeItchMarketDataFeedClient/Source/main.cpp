#include <thread>
#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchMarketDataFeedClient.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using ApplicationFeedChannel =
    WrapperChannel<std::unique_ptr<MulticastSocketChannel>,
      QueuedReader<MulticastSocketChannel::Reader*>>;
  using ApplicationProtocolClient =
    MoldUdp64Client<std::unique_ptr<ApplicationFeedChannel>>;
  using ApplicationRecoveryChannel =
    WrapperChannel<std::unique_ptr<UdpSocketChannel>,
      QueuedReader<UdpSocketChannel::Reader*>>;
  using ApplicationRecoveryClient =
    MoldUdp64Client<std::unique_ptr<ApplicationRecoveryChannel>>;
  using ApplicationGlimpseClient =
    AsxTradeItchGlimpseClient<std::unique_ptr<TcpSocketChannel>, LiveTimer>;

  std::unique_ptr<ApplicationProtocolClient> make_protocol_client(
      const AsxTradeItchFeed& feed, const MulticastSocketOptions& options) {
    auto socket = try_or_nest([&] {
      return std::make_unique<MulticastSocketChannel>(
        feed.m_address, feed.m_interface, options);
    }, std::runtime_error(
      "Unable to join ASX Trade ITCH feed " + feed.m_name + '.'));
    auto reader = &socket->get_reader();
    auto channel =
      std::make_unique<ApplicationFeedChannel>(std::move(socket), reader);
    auto& queued_reader = channel->get_reader();
    auto client =
      std::make_unique<ApplicationProtocolClient>(std::move(channel));
    queued_reader.poll();
    return client;
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" ASX_TRADE_ITCH_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto definitions_client =
      ApplicationDefinitionsClient(Ref(service_locator_client));
    load_definitions(definitions_client);
    auto configuration = AsxTradeItchConfiguration::parse(config);
    auto feed_clients =
      std::vector<std::unique_ptr<ApplicationProtocolClient>>();
    for(auto& feed : configuration.m_feeds) {
      feed_clients.push_back(
        make_protocol_client(feed, configuration.m_socket_options));
    }
    auto recovery_client =
      optional<std::unique_ptr<ApplicationRecoveryClient>>();
    if(auto rewind = configuration.m_rewind) {
      recovery_client = try_or_nest([&] {
        auto channel = std::make_unique<UdpSocketChannel>(
          rewind->m_address, rewind->m_interface,
          configuration.m_socket_options);
        auto reader = &channel->get_reader();
        return std::make_unique<ApplicationRecoveryClient>(
          std::make_unique<ApplicationRecoveryChannel>(
            std::move(channel), reader));
      }, std::runtime_error(
        "Unable to open the ASX Trade ITCH rewind socket."));
    }
    auto glimpse_client =
      optional<std::unique_ptr<ApplicationGlimpseClient>>();
    if(configuration.m_glimpse) {
      glimpse_client = try_or_nest([&] {
        auto& server = *configuration.m_glimpse;
        return std::make_unique<ApplicationGlimpseClient>(
          server.m_username, server.m_password,
          std::make_unique<TcpSocketChannel>(
            server.m_address, server.m_interface), init(seconds(1)));
      }, std::runtime_error("Unable to log in to ASX Trade ITCH Glimpse."));
    }
    auto client = AsxTradeItchClient(configuration.m_feed_timeout,
      configuration.m_gap_timeout, configuration.m_request_timeout,
      std::move(feed_clients), std::move(recovery_client),
      std::move(glimpse_client),
      std::make_unique<LocalTimeClient>(),
      std::make_unique<LiveTimer>(configuration.get_timer_interval()));
    auto market_data_feed_client = ApplicationMarketDataFeedClient(
      Ref(service_locator_client), configuration.m_sampling,
      configuration.m_country);
    auto feed_client = AsxTradeItchMarketDataFeedClient(
      configuration, &market_data_feed_client, &client);
    while(!feed_client.is_finished() && !received_kill_event()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    feed_client.close();
    if(auto exception = feed_client.get_exception()) {
      std::rethrow_exception(exception);
    }
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
