#include <atomic>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include "TmxIpMarketDataFeedClient/TmxIpClient.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpConfiguration.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  using ApplicationFeedChannel =
    WrapperChannel<std::unique_ptr<MulticastSocketChannel>,
      QueuedReader<MulticastSocketChannel::Reader*>>;
  using ApplicationProtocolClient =
    TmxIpProtocolClient<std::unique_ptr<ApplicationFeedChannel>>;
  using ApplicationRecoveryChannel =
    WrapperChannel<std::unique_ptr<UdpSocketChannel>,
      QueuedReader<UdpSocketChannel::Reader*>>;
  using ApplicationRecoveryProtocolClient =
    TmxIpProtocolClient<std::unique_ptr<ApplicationRecoveryChannel>>;
  using ApplicationRecoveryClient = TmxIpRecoveryClient<TcpSocketChannel,
    std::unique_ptr<ApplicationRecoveryProtocolClient>, LiveTimer>;

  void log(const StampMessage& message) {
    auto out = std::stringstream();
    auto write_section = [&] (
        std::string_view name, const StampMessage::Section& section) {
      out << " (" << name;
      for(auto& field : section) {
        out << " (" << field.m_identifier << ' ' << field.m_index << ' ' <<
          std::quoted(field.m_value) << ')';
      }
      out << ')';
    };
    out << "(stamp";
    write_section("control", message.m_control_header);
    write_section("business", message.m_business_content);
    out << ")\n";
    std::cout << out.str() << std::flush;
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" TMX_IP_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto configuration = TmxIpConfiguration::parse(config);
    auto feed_clients =
      std::vector<std::unique_ptr<ApplicationProtocolClient>>();
    for(auto& feed : configuration.m_feeds) {
      feed_clients.push_back(try_or_nest([&] {
        auto channel = std::make_unique<MulticastSocketChannel>(
          feed.m_address, feed.m_interface, configuration.m_socket_options);
        auto reader = &channel->get_reader();
        return std::make_unique<ApplicationProtocolClient>(
          std::make_unique<ApplicationFeedChannel>(std::move(channel), reader));
      }, std::runtime_error("Unable to join a TMX IP multicast group.")));
    }
    auto recovery_client =
      boost::optional<std::unique_ptr<ApplicationRecoveryClient>>();
    if(auto recovery = configuration.m_recovery) {
      auto protocol_client = try_or_nest([&] {
        auto channel = std::make_unique<UdpSocketChannel>(
          recovery->m_address, recovery->m_delivery_address,
          configuration.m_socket_options);
        auto reader = &channel->get_reader();
        return std::make_unique<ApplicationRecoveryProtocolClient>(
          std::make_unique<ApplicationRecoveryChannel>(
            std::move(channel), reader));
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
    auto client = TmxIpClient(configuration.m_feed_timeout,
      std::move(feed_clients), std::move(recovery_client),
      std::make_unique<LocalTimeClient>(),
      std::make_unique<LiveTimer>(configuration.m_retry_interval));
    auto exception = std::exception_ptr();
    auto is_finished = std::atomic_bool(false);
    auto reader = RoutineHandler(spawn([&] {
      try {
        while(true) {
          auto message = client.read();
          if(configuration.m_is_logging_messages) {
            log(message);
          }
        }
      } catch(const std::exception&) {
        exception = std::current_exception();
      }
      is_finished = true;
    }));
    while(!is_finished && !received_kill_event()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    auto is_interrupted = received_kill_event();
    client.close();
    reader.wait();
    if(!is_interrupted && exception) {
      std::rethrow_exception(exception);
    }
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
