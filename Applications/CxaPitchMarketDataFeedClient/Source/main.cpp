#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>
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
#include "CxaPitchMarketDataFeedClient/CxaPitchMessages.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchProtocolClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSessionClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSpinClient.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
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
  using ApplicationSessionClient =
    CxaPitchSessionClient<TcpSocketChannel, LiveTimer>;
  using ApplicationGapClient =
    CxaPitchGapClient<std::unique_ptr<ApplicationSessionClient>>;
  using ApplicationSpinClient =
    CxaPitchSpinClient<std::unique_ptr<ApplicationSessionClient>>;
  using ApplicationCxaPitchClient =
    CxaPitchClient<std::unique_ptr<ApplicationProtocolClient>,
      std::unique_ptr<ApplicationGapClient>,
      std::unique_ptr<ApplicationSpinClient>, std::unique_ptr<LocalTimeClient>>;
  static const auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(134217728);

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

  void log(const CxaPitchMessage& message) {
    visit(message, [] (const auto& message) {
      std::cout << message << std::endl;
    });
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
    auto options = MulticastSocketOptions();
    options.m_receive_buffer_size =
      extract<int>(config, "receive_buffer", DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_max_datagram_size =
      extract<int>(config, "mtu", options.m_max_datagram_size);
    static const auto HEARTBEAT = seconds(1);
    auto make_session = [&] (const CxaPitchSession& session) {
      auto login = CxaPitchLogin();
      login.m_session_sub_id = session.m_session_sub_id;
      login.m_username = session.m_username;
      login.m_password = session.m_password;
      return std::make_unique<ApplicationSessionClient>(
        login, init(session.m_address), init(HEARTBEAT));
    };
    auto gap_client = optional<std::unique_ptr<ApplicationGapClient>>();
    if(feed_configuration.m_retransmission) {
      try {
        gap_client = std::make_unique<ApplicationGapClient>(
          make_session(*feed_configuration.m_retransmission));
        std::cout << ",retransmission,connected" << std::endl;
      } catch(const std::exception&) {
        std::cout << ",retransmission,error" << std::endl;
        std::cout << make_exception_report(std::current_exception()) <<
          std::endl;
      }
    }
    auto spin_client = optional<std::unique_ptr<ApplicationSpinClient>>();
    if(feed_configuration.m_spin) {
      try {
        spin_client = std::make_unique<ApplicationSpinClient>(
          make_session(*feed_configuration.m_spin));
        std::cout << ",spin,connected" << std::endl;
      } catch(const std::exception&) {
        std::cout << ",spin,error" << std::endl;
        std::cout << make_exception_report(std::current_exception()) <<
          std::endl;
      }
    }
    auto feeds = std::vector<std::unique_ptr<ApplicationProtocolClient>>();
    auto recovery = std::vector<std::unique_ptr<ApplicationProtocolClient>>();
    for(auto& feed : feed_configuration.m_feeds) {
      feeds.push_back(
        make_protocol_client(feed.m_address, feed.m_interface, options));
      if(gap_client && !feed.m_gap_address.get_host().empty()) {
        recovery.push_back(
          make_protocol_client(feed.m_gap_address, feed.m_interface, options));
      }
    }
    auto client = ApplicationCxaPitchClient(feed_configuration.m_unit,
      feed_configuration.m_liveness, feed_configuration.m_gap_timeout,
      std::move(feeds), std::move(recovery), std::move(gap_client),
      std::move(spin_client), std::make_unique<LocalTimeClient>());
    auto read_loop = RoutineHandler(spawn([&] {
      while(true) {
        try {
          auto message = client.read();
          if(feed_configuration.m_is_logging_messages) {
            log(message);
          }
        } catch(const std::exception&) {
          break;
        }
      }
    }));
    wait_for_kill_event();
    client.close();
    read_loop.wait();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
