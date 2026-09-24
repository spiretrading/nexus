#include <thread>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkConfiguration.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkMarketDataFeedClient.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkRecoveryClient.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkSnapshotClient.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  using ApplicationProtocolClient = OtcLinkProtocolClient<
    std::unique_ptr<BufferedMulticastSocketChannel>, LiveNtpTimeClient*>;
  using ApplicationRecoveryClient =
    OtcLinkRecoveryClient<TcpSocketChannel, LiveTimer>;
  using ApplicationSnapshotClient = OtcLinkSnapshotClient<
    std::unique_ptr<ApplicationProtocolClient>, LiveTimer>;
  using ApplicationOtcClient = OtcLinkClient<
    std::unique_ptr<ApplicationProtocolClient>, LiveNtpTimeClient*,
    std::unique_ptr<LiveTimer>>;

  std::unique_ptr<ApplicationProtocolClient> make_protocol_client(
      const OtcLinkFeed& feed, const MulticastSocketOptions& options,
      LiveNtpTimeClient& time_client) {
    try {
      auto channel = std::make_unique<BufferedMulticastSocketChannel>(
        feed.m_address, feed.m_interface, options);
      return std::make_unique<ApplicationProtocolClient>(
        std::move(channel), &time_client);
    } catch(...) {
      throw_nested_with_location(
        std::runtime_error("Unable to join an OTC Link multicast group."));
    }
  }

  std::unique_ptr<ApplicationRecoveryClient> make_recovery_client(
      const OtcLinkRecoveryConfiguration& server, std::uint16_t channel) {
    return std::make_unique<ApplicationRecoveryClient>(server.m_sender,
      channel, [=] (std::stop_token token) {
        if(token.stop_requested()) {
          boost::throw_with_location(
            IOException("OTC Link recovery connection canceled."));
        }
        return std::make_shared<TcpSocketChannel>(
          server.m_address, server.m_interface);
      }, init(server.m_timeout));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" OTC_LINK_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto definitions_client =
      ApplicationDefinitionsClient(Ref(service_locator_client));
    load_definitions(definitions_client);
    auto configuration = OtcLinkConfiguration::parse(config);
    auto time_client = make_live_ntp_time_client(service_locator_client);
    auto market_data_feed_client = ApplicationMarketDataFeedClient(
      Ref(service_locator_client), configuration.m_sampling,
      configuration.m_country);
    auto feed_clients =
      std::vector<std::unique_ptr<ApplicationProtocolClient>>();
    for(auto& feed : configuration.m_feeds) {
      feed_clients.push_back(make_protocol_client(
        feed, configuration.m_socket_options, *time_client));
    }
    auto recovery_client = std::unique_ptr<ApplicationRecoveryClient>();
    auto recovery = ApplicationOtcClient::RecoveryFunction();
    if(auto server = configuration.m_recovery) {
      recovery_client = make_recovery_client(*server, configuration.m_channel);
      recovery = [&] (std::uint32_t sequence, std::uint32_t count,
          std::stop_token token) {
        return recovery_client->request(sequence, count, token);
      };
    }
    auto snapshot_server = std::unique_ptr<ApplicationRecoveryClient>();
    auto snapshot_client = std::unique_ptr<ApplicationSnapshotClient>();
    auto snapshot = ApplicationOtcClient::SnapshotFunction();
    if(auto settings = configuration.m_snapshot) {
      snapshot_server =
        make_recovery_client(settings->m_server, configuration.m_channel);
      snapshot_client = std::make_unique<ApplicationSnapshotClient>(
        OtcLinkSpinType::MARKET_DATA, [&] (std::stop_token token) {
          snapshot_server->request_snapshot(token);
        }, make_protocol_client(settings->m_feed,
          configuration.m_socket_options, *time_client),
        init(settings->m_timeout));
      snapshot = [&] (std::stop_token token) {
        return snapshot_client->load_snapshot(token);
      };
    }
    auto client = ApplicationOtcClient(configuration.m_feed_timeout,
      configuration.m_gap_timeout, std::move(feed_clients), time_client.get(),
      std::make_unique<LiveTimer>(configuration.get_timer_interval()),
      std::move(recovery), std::move(snapshot));
    auto reference = boost::optional<OtcLinkSnapshot>();
    if(auto settings = configuration.m_reference) {
      auto server = make_recovery_client(
        settings->m_server, OtcLinkConfiguration::INSIDE_CHANNEL);
      auto snapshot = ApplicationSnapshotClient(OtcLinkSpinType::REFERENCE,
        [&] (std::stop_token token) {
          server->request_snapshot(token);
        }, make_protocol_client(settings->m_feed,
          configuration.m_socket_options, *time_client),
        init(settings->m_timeout));
      reference = snapshot.load_snapshot(std::stop_token());
    }
    auto feed_client = OtcLinkMarketDataFeedClient(
      &market_data_feed_client, &client, time_client.get(),
      std::move(reference));
    while(!feed_client.is_finished() && !received_kill_event()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    auto is_interrupted = received_kill_event();
    feed_client.close();
    auto exception = feed_client.get_exception();
    if(!is_interrupted && exception) {
      std::rethrow_exception(exception);
    }
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
