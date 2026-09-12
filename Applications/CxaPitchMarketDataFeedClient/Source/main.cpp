#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <stop_token>
#include <vector>
#include <Beam/IO/AsyncWriter.hpp>
#include <Beam/IO/IOException.hpp>
#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchConfiguration.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchGapClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchMarketDataFeedClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchProtocolClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSessionClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSpinClient.hpp"
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
  using ApplicationGapClient = CxaPitchGapClient<ApplicationSessionClient,
    LiveTimer, std::unique_ptr<LocalTimeClient>>;
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
    auto feed_configuration = CxaPitchConfiguration::parse(config);
    auto sampling = extract<time_duration>(config, "sampling");
    auto options = MulticastSocketOptions();
    static const auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(134217728);
    options.m_receive_buffer_size =
      extract<int>(config, "receive_buffer", DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_max_datagram_size =
      extract<int>(config, "mtu", options.m_max_datagram_size);
    auto make_session = [] (
        const CxaPitchSession& session, std::stop_token stop_token) {
      static const auto HEARTBEAT = seconds(1);
      auto login = CxaPitchLogin();
      login.m_session_sub_id = session.m_session_sub_id;
      login.m_username = session.m_username;
      login.m_password = session.m_password;
      auto channel = std::make_unique<TcpSocketChannel>(session.m_address);
      auto reader = &channel->get_reader();
      auto writer = &channel->get_writer();
      return std::make_shared<ApplicationSessionClient>(login,
        std::make_unique<ApplicationSessionChannel>(
          std::move(channel), reader, writer),
        init(HEARTBEAT), stop_token);
    };
    auto gap_client = optional<std::unique_ptr<ApplicationGapClient>>();
    if(feed_configuration.m_retransmission) {
      auto session = *feed_configuration.m_retransmission;
      gap_client = try_or_nest([&] {
        static const auto RECONNECT = seconds(10);
        return std::make_unique<ApplicationGapClient>(
          [=] (std::stop_token stop_token) {
            return make_session(session, stop_token);
          }, init(RECONNECT), std::make_unique<LocalTimeClient>());
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
    auto feeds = std::vector<std::unique_ptr<ApplicationProtocolClient>>();
    auto recovery = std::vector<std::unique_ptr<ApplicationProtocolClient>>();
    for(auto& feed : feed_configuration.m_feeds) {
      feeds.push_back(
        make_protocol_client(feed.m_address, feed.m_interface, options));
      if(gap_client && feed.m_gap_address) {
        recovery.push_back(
          make_protocol_client(*feed.m_gap_address, feed.m_interface, options));
      }
    }
    auto client = CxaPitchClient(feed_configuration.m_unit,
      feed_configuration.m_feed_timeout, feed_configuration.m_gap_timeout,
      std::move(feeds), std::move(recovery), std::move(gap_client),
      std::move(spin_client), std::make_unique<LocalTimeClient>(),
      std::make_unique<LiveTimer>(feed_configuration.m_feed_timeout));
    auto market_data_feed_client = ApplicationMarketDataFeedClient(
      Ref(service_locator_client), sampling, feed_configuration.m_country);
    auto feed_client = CxaPitchMarketDataFeedClient(
      feed_configuration, &market_data_feed_client, &client);
    wait_for_kill_event();
    feed_client.close();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
