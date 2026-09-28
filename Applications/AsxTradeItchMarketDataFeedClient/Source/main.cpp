#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
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
  using ApplicationFeedChannel = BufferedMulticastSocketChannel;
  using ApplicationProtocolClient =
    MoldUdp64Client<std::unique_ptr<ApplicationFeedChannel>>;
  using ApplicationRecoveryChannel = BufferedUdpSocketChannel;
  using ApplicationRecoveryClient =
    MoldUdp64Client<std::unique_ptr<ApplicationRecoveryChannel>>;
  using ApplicationGlimpseClient =
    AsxTradeItchGlimpseClient<std::unique_ptr<TcpSocketChannel>, LiveTimer>;

  std::unique_ptr<ApplicationProtocolClient> make_protocol_client(
      const AsxTradeItchFeed& feed, const MulticastSocketOptions& options) {
    auto channel = try_or_nest([&] {
      return std::make_unique<ApplicationFeedChannel>(
        feed.m_address, feed.m_interface, options);
    }, std::runtime_error(
      "Unable to join ASX Trade ITCH feed " + feed.m_name + '.'));
    channel->get_reader().poll();
    return std::make_unique<ApplicationProtocolClient>(std::move(channel));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" ASX_TRADE_ITCH_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_locator_client = connect<ApplicationServiceLocatorClient>(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto definitions_client =
      connect<ApplicationDefinitionsClient>(Ref(service_locator_client));
    load_definitions(definitions_client);
    auto configuration = AsxTradeItchConfiguration::parse(config);
    auto time_client = connect([&] {
      return make_live_ntp_time_client(service_locator_client);
    });
    auto market_data_feed_client = connect<ApplicationMarketDataFeedClient>(
      Ref(service_locator_client), configuration.m_sampling,
      configuration.m_country);
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
        auto channel = std::make_unique<ApplicationRecoveryChannel>(
          rewind->m_address, rewind->m_interface,
          configuration.m_socket_options);
        return std::make_unique<ApplicationRecoveryClient>(std::move(channel));
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
      std::move(glimpse_client), std::move(time_client),
      std::make_unique<LiveTimer>(configuration.get_timer_interval()));
    auto feed_client = AsxTradeItchMarketDataFeedClient(
      configuration, &market_data_feed_client, &client);
    while(!feed_client.is_finished() && !received_kill_event()) {
      sleep_for(milliseconds(100));
    }
    service_locator_client.close();
    feed_client.close();
    if(auto exception = feed_client.get_exception()) {
      std::rethrow_exception(exception);
    }
  } catch(...) {
    if(received_kill_event()) {
      return 0;
    }
    report_current_exception();
    return -1;
  }
  return 0;
}
