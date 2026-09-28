#include <algorithm>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpMarketDataFeedClient.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  using ApplicationFeedChannel = BufferedMulticastSocketChannel;
  using ApplicationProtocolClient = TmxIpProtocolClient<
    std::unique_ptr<ApplicationFeedChannel>, LiveNtpTimeClient*>;
  using ApplicationRecoveryChannel = BufferedUdpSocketChannel;
  using ApplicationRecoveryProtocolClient = TmxIpProtocolClient<
    std::unique_ptr<ApplicationRecoveryChannel>, LiveNtpTimeClient*>;
  using ApplicationRecoveryClient = TmxIpRecoveryClient<TcpSocketChannel,
    std::unique_ptr<ApplicationRecoveryProtocolClient>, LiveTimer>;
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" TMX_IP_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_locator_client = connect<ApplicationServiceLocatorClient>(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto definitions_client =
      connect<ApplicationDefinitionsClient>(Ref(service_locator_client));
    load_definitions(definitions_client);
    auto schedule = definitions_client.load_trading_schedule();
    auto configuration = TmxIpConfiguration::parse(config);
    auto market_data_client =
      connect<ApplicationMarketDataClient>(Ref(service_locator_client));
    auto market_data_feed_client = connect<ApplicationMarketDataFeedClient>(
      Ref(service_locator_client), configuration.m_sampling, Countries::CA);
    auto time_client = connect([&] {
      return make_live_ntp_time_client(service_locator_client);
    });
    auto feed_clients =
      std::vector<std::unique_ptr<ApplicationProtocolClient>>();
    for(auto& feed : configuration.m_feeds) {
      feed_clients.push_back(try_or_nest([&] {
        auto channel = std::make_unique<ApplicationFeedChannel>(
          feed.m_address, feed.m_interface, configuration.m_socket_options);
        return std::make_unique<ApplicationProtocolClient>(
          std::move(channel), time_client.get());
      }, std::runtime_error("Unable to join a TMX IP multicast group.")));
    }
    auto recovery_client =
      boost::optional<std::unique_ptr<ApplicationRecoveryClient>>();
    if(auto recovery = configuration.m_recovery) {
      auto protocol_client = try_or_nest([&] {
        auto channel = std::make_unique<ApplicationRecoveryChannel>(
          recovery->m_address, recovery->m_delivery_address,
          configuration.m_socket_options);
        return std::make_unique<ApplicationRecoveryProtocolClient>(
          std::move(channel), time_client.get());
      }, std::runtime_error(
        "Unable to open the TMX IP recovery delivery socket."));
      recovery_client = std::make_unique<ApplicationRecoveryClient>(
        [=] (std::stop_token token) {
          if(token.stop_requested()) {
            boost::throw_with_location(
              IOException("TMX IP recovery connection canceled."));
          }
          return std::make_shared<TcpSocketChannel>(
            recovery->m_address, recovery->m_interface);
        }, std::move(protocol_client), init(recovery->m_timeout));
    }
    auto client = TmxIpClient(configuration.m_time_zone,
      configuration.m_rollover_time, configuration.m_feed_timeout,
      configuration.m_gap_timeout, std::move(feed_clients),
      std::move(recovery_client), time_client.get(),
      std::make_unique<LiveTimer>(std::min({configuration.m_retry_interval,
        configuration.m_feed_timeout, configuration.m_gap_timeout})));
    auto feed_client = TmxIpMarketDataFeedClient(configuration,
      std::move(schedule), &client, &market_data_client, time_client.get(),
      &market_data_feed_client);
    while(!feed_client.is_finished() && !received_kill_event()) {
      sleep_for(boost::posix_time::milliseconds(100));
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
