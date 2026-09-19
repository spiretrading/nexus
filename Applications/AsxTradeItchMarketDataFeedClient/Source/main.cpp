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
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchClient.hpp"
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchConfiguration.hpp"
#include "Nexus/Definitions/StandardVenues.hpp"
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

  void log(const AsxTradeItchMessage& message) {
    auto out = std::stringstream();
    out << "(message " << static_cast<char>(message.m_type) << ' ' <<
      std::hex << std::setfill('0');
    for(auto i = std::size_t(0);
        i != message.m_length - AsxTradeItchMessage::HEADER_LENGTH; ++i) {
      out << std::setw(2) << static_cast<unsigned int>(
        static_cast<unsigned char>(message.m_payload[i]));
    }
    out << ")\n";
    std::cout << out.str() << std::flush;
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" ASX_TRADE_ITCH_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto configuration = AsxTradeItchConfiguration::parse(config);
    auto feed_clients =
      std::vector<std::unique_ptr<ApplicationProtocolClient>>();
    for(auto& feed : configuration.m_feeds) {
      feed_clients.push_back(
        make_protocol_client(feed, configuration.m_socket_options));
    }
    auto recovery_client = try_or_nest([&] {
      auto channel = std::make_unique<UdpSocketChannel>(
        configuration.m_rewind.m_address, configuration.m_rewind.m_interface,
        configuration.m_socket_options);
      auto reader = &channel->get_reader();
      return std::make_unique<ApplicationRecoveryClient>(
        std::make_unique<ApplicationRecoveryChannel>(
          std::move(channel), reader));
    }, std::runtime_error("Unable to open the ASX Trade ITCH rewind socket."));
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
      configuration.m_request_timeout, std::move(feed_clients),
      std::move(recovery_client), std::move(glimpse_client),
      std::make_unique<LocalTimeClient>(),
      std::make_unique<LiveTimer>(configuration.get_timer_interval()));
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
      } catch(const EndOfFileException&) {
      } catch(const std::exception&) {
        exception = std::current_exception();
      }
      is_finished = true;
    }));
    while(!is_finished && !received_kill_event()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    client.close();
    reader.wait();
    if(exception) {
      std::rethrow_exception(exception);
    }
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
