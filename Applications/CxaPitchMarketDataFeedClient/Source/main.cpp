#include <stdexcept>
#include <thread>
#include <Beam/IO/AsyncWriter.hpp>
#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchMarketDataFeedClient.hpp"
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
    CxaPitchProtocolClient<std::unique_ptr<ApplicationFeedChannel>>;
  using ApplicationSessionChannel =
    WrapperChannel<std::unique_ptr<TcpSocketChannel>,
      QueuedReader<TcpSocketChannel::Reader*>,
      AsyncWriter<TcpSocketChannel::Writer*>>;
  using ApplicationSessionClient = CxaPitchSessionClient<
    std::unique_ptr<ApplicationSessionChannel>, LiveTimer>;
  using ApplicationGapClient =
    CxaPitchGapClient<ApplicationSessionClient, LiveTimer, LiveNtpTimeClient*>;
  using ApplicationSpinClient =
    CxaPitchSpinClient<std::shared_ptr<ApplicationSessionClient>>;

  std::unique_ptr<ApplicationProtocolClient> make_protocol_client(
      const IpAddress& address, const IpAddress& interface,
      const MulticastSocketOptions& options) {
    auto channel = try_or_nest([&] {
      return std::make_unique<MulticastSocketChannel>(
        address, interface, options);
    }, std::runtime_error("Unable to join the CXA PITCH multicast group."));
    auto reader = &channel->get_reader();
    return std::make_unique<ApplicationProtocolClient>(
      std::make_unique<ApplicationFeedChannel>(std::move(channel), reader));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" CXA_PITCH_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto definitions_client =
      ApplicationDefinitionsClient(Ref(service_locator_client));
    load_definitions(definitions_client);
    auto time_client = make_live_ntp_time_client(service_locator_client);
    auto feed_configuration = CxaPitchConfiguration::parse(config);
    auto make_session = [] (
        const CxaPitchSession& session, std::stop_token stop_token) {
      static const auto HEARTBEAT = seconds(1);
      auto login = CxaPitchLogin(
        session.m_session_sub_id, session.m_username, session.m_password);
      auto channel = std::make_unique<TcpSocketChannel>(session.m_address);
      auto reader = &channel->get_reader();
      auto writer = &channel->get_writer();
      return std::make_shared<ApplicationSessionClient>(
        login, std::make_unique<ApplicationSessionChannel>(
          std::move(channel), reader, writer), init(HEARTBEAT), stop_token);
    };
    auto gap_client = optional<std::unique_ptr<ApplicationGapClient>>();
    if(feed_configuration.m_retransmission) {
      auto session = *feed_configuration.m_retransmission;
      gap_client = try_or_nest([&] {
        static const auto RECONNECT = seconds(10);
        return std::make_unique<ApplicationGapClient>(
          [=] (std::stop_token stop_token) {
            return make_session(session, stop_token);
          }, init(RECONNECT), time_client.get());
      }, std::runtime_error(
        "Unable to connect to the CXA PITCH gap request proxy."));
    }
    auto spin_client = optional<std::unique_ptr<ApplicationSpinClient>>();
    if(feed_configuration.m_spin) {
      spin_client = try_or_nest([&] {
        return std::make_unique<ApplicationSpinClient>(
          make_session(*feed_configuration.m_spin, std::stop_token()));
      }, std::runtime_error("Unable to connect to the CXA PITCH spin server."));
    }
    auto feed_clients =
      std::vector<std::unique_ptr<ApplicationProtocolClient>>();
    auto recovery_clients =
      std::vector<std::unique_ptr<ApplicationProtocolClient>>();
    for(auto& feed : feed_configuration.m_feeds) {
      feed_clients.push_back(make_protocol_client(feed.m_address,
        feed.m_interface, feed_configuration.m_socket_options));
      if(gap_client && feed.m_gap_address) {
        recovery_clients.push_back(make_protocol_client(*feed.m_gap_address,
          feed.m_interface, feed_configuration.m_socket_options));
      }
    }
    auto client = CxaPitchClient(feed_configuration.m_unit,
      feed_configuration.m_feed_timeout, feed_configuration.m_gap_timeout,
      std::move(feed_clients), std::move(recovery_clients),
      std::move(gap_client), std::move(spin_client), time_client.get(),
      std::make_unique<LiveTimer>(feed_configuration.get_timer_interval()));
    auto market_data_feed_client = ApplicationMarketDataFeedClient(
      Ref(service_locator_client), feed_configuration.m_sampling,
      feed_configuration.m_country);
    auto feed_client = CxaPitchMarketDataFeedClient(
      feed_configuration, &market_data_feed_client, &client);
    while(!received_kill_event()) {
      if(auto exception = feed_client.get_exception()) {
        std::rethrow_exception(exception);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    feed_client.close();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
