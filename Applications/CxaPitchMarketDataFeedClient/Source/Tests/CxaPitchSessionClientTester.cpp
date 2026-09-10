#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <stop_token>
#include <string>
#include <string_view>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/IOException.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchSessionClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  const auto LOGIN_SIZE = CxaPitchHeader::LENGTH + CxaPitchLogin::LENGTH;
  const auto GAP_RESPONSE =
    std::string_view("\x0a\x04" "\x01" "\x3b\x10\x00\x00" "\x32\x00" "A", 10);

  SharedBuffer encode(std::string_view messages, std::uint8_t count) {
    auto block = std::string();
    auto length =
      static_cast<std::uint16_t>(CxaPitchHeader::LENGTH + messages.size());
    block += static_cast<char>(length & 0xFF);
    block += static_cast<char>((length >> 8) & 0xFF);
    block += static_cast<char>(count);
    block += char(0);
    block.append(4, char(0));
    block += messages;
    return SharedBuffer(block.data(), block.size());
  }

  std::string read_exactly(LocalServerChannel& channel, std::size_t size) {
    auto buffer = SharedBuffer();
    while(buffer.get_size() < size) {
      channel.get_reader().read(out(buffer), size - buffer.get_size());
    }
    return std::string(buffer.get_data(), buffer.get_size());
  }

  struct Fixture {
    LocalServerConnection m_server;
    optional<LocalClientChannel> m_channel;
    TriggerTimer m_timer;
    std::string m_login;
    std::unique_ptr<LocalServerChannel> m_server_channel;
    optional<
      CxaPitchSessionClient<LocalClientChannel*, TriggerTimer*>> m_client;

    explicit Fixture(char status) {
      auto server_channel = std::async(std::launch::async, [&] {
        auto channel = m_server.accept();
        m_login = read_exactly(*channel, LOGIN_SIZE);
        auto response = std::string("\x03\x02", 2);
        response += status;
        channel->get_writer().write(encode(response, 1));
        return channel;
      });
      m_channel.emplace("cxa", m_server);
      auto login = CxaPitchLogin();
      login.m_session_sub_id = "0001";
      login.m_username = "FIRM";
      login.m_password = "ABCD00";
      m_client.emplace(login, &*m_channel, &m_timer);
      m_server_channel = server_channel.get();
    }

    Fixture()
      : Fixture(CxaPitchLoginResponse::ACCEPTED) {}
  };
}

TEST_SUITE("CxaPitchSessionClient") {
  TEST_CASE("log_in") {
    auto fixture = Fixture();
    REQUIRE(fixture.m_login == std::string_view(
      "\x1e\x00" "\x01" "\x00" "\x00\x00\x00\x00"
      "\x16\x01" "0001" "FIRM" "  " "ABCD00    ", 30));
  }

  TEST_CASE("reject_login") {
    REQUIRE_THROWS_AS(Fixture('N'), ConnectException);
  }

  TEST_CASE("cancel_pending_login") {
    auto server = LocalServerConnection();
    auto stop_source = std::stop_source();
    auto accepting = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      read_exactly(*channel, LOGIN_SIZE);
      stop_source.request_stop();
      return channel;
    });
    auto channel = LocalClientChannel("cxa", server);
    auto timer = TriggerTimer();
    REQUIRE_THROWS_AS(CxaPitchSessionClient(CxaPitchLogin(), &channel, &timer,
      stop_source.get_token()), ConnectException);
    auto server_channel = accepting.get();
    server_channel->get_connection().close();
  }

  TEST_CASE("cancel_before_login") {
    auto server = LocalServerConnection();
    auto accepting = std::async(std::launch::async, [&] {
      return server.accept();
    });
    auto channel = LocalClientChannel("cxa", server);
    auto server_channel = accepting.get();
    auto stop_source = std::stop_source();
    stop_source.request_stop();
    auto timer = TriggerTimer();
    REQUIRE_THROWS_AS(CxaPitchSessionClient(CxaPitchLogin(), &channel, &timer,
      stop_source.get_token()), ConnectException);
    server_channel->get_connection().close();
  }

  TEST_CASE("timeout_pending_login") {
    auto server = LocalServerConnection();
    auto timer = LiveTimer(milliseconds(1));
    auto accepting = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      read_exactly(*channel, LOGIN_SIZE);
      return channel;
    });
    auto channel = LocalClientChannel("cxa", server);
    REQUIRE_THROWS_AS(CxaPitchSessionClient(CxaPitchLogin(), &channel, &timer),
      ConnectException);
    auto server_channel = accepting.get();
    server_channel->get_connection().close();
  }

  TEST_CASE("timeout_silent_session") {
    auto server = LocalServerConnection();
    auto accepting = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      read_exactly(*channel, LOGIN_SIZE);
      auto response = std::string("\x03\x02", 2);
      response += CxaPitchLoginResponse::ACCEPTED;
      channel->get_writer().write(encode(response, 1));
      return channel;
    });
    auto channel = LocalClientChannel("cxa", server);
    auto timer = LiveTimer(milliseconds(1));
    auto client = CxaPitchSessionClient(CxaPitchLogin(), &channel, &timer);
    auto server_channel = accepting.get();
    REQUIRE_THROWS_AS(client.read(), IOException);
    server_channel->get_connection().close();
  }

  TEST_CASE("read_message") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(encode(GAP_RESPONSE, 1));
    auto message = fixture.m_client->read();
    REQUIRE(message.m_type == CxaPitchGapResponse::TYPE);
    auto response = CxaPitchGapResponse::parse(message);
    REQUIRE(response.m_unit == 1);
    REQUIRE(response.m_sequence == 4155);
    REQUIRE(response.m_count == 50);
    REQUIRE(response.m_status == CxaPitchGapResponse::ACCEPTED);
  }

  TEST_CASE("read_fragmented_message") {
    auto fixture = Fixture();
    auto block = encode(GAP_RESPONSE, 1);
    fixture.m_server_channel->get_writer().write(
      SharedBuffer(block.get_data(), 5));
    fixture.m_server_channel->get_writer().write(
      SharedBuffer(block.get_data() + 5, block.get_size() - 5));
    auto message = fixture.m_client->read();
    REQUIRE(message.m_type == CxaPitchGapResponse::TYPE);
    REQUIRE(CxaPitchGapResponse::parse(message).m_sequence == 4155);
  }

  TEST_CASE("read_multiple_messages") {
    auto fixture = Fixture();
    auto messages = std::string(GAP_RESPONSE) + std::string(GAP_RESPONSE);
    fixture.m_server_channel->get_writer().write(encode(messages, 2));
    REQUIRE(fixture.m_client->read().m_type == CxaPitchGapResponse::TYPE);
    REQUIRE(fixture.m_client->read().m_type == CxaPitchGapResponse::TYPE);
  }

  TEST_CASE("skip_heartbeat") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(encode("", 0));
    fixture.m_server_channel->get_writer().write(encode(GAP_RESPONSE, 1));
    REQUIRE(fixture.m_client->read().m_type == CxaPitchGapResponse::TYPE);
  }

  TEST_CASE("read_miscounted_block") {
    auto fixture = Fixture();
    auto messages = std::string(GAP_RESPONSE) + std::string("\x02\x04", 2);
    fixture.m_server_channel->get_writer().write(encode(messages, 1));
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
  }

  TEST_CASE("write_gap_request") {
    auto fixture = Fixture();
    auto request = CxaPitchGapRequest();
    request.m_unit = 1;
    request.m_sequence = 4155;
    request.m_count = 50;
    fixture.m_client->write(request);
    REQUIRE(read_exactly(*fixture.m_server_channel,
      CxaPitchHeader::LENGTH + CxaPitchGapRequest::LENGTH) == std::string_view(
        "\x11\x00" "\x01" "\x00" "\x00\x00\x00\x00"
        "\x09\x03" "\x01" "\x3b\x10\x00\x00" "\x32\x00", 17));
  }

  TEST_CASE("write_heartbeat") {
    auto fixture = Fixture();
    fixture.m_timer.trigger();
    REQUIRE(read_exactly(*fixture.m_server_channel, CxaPitchHeader::LENGTH) ==
      std::string_view("\x08\x00" "\x00" "\x00" "\x00\x00\x00\x00", 8));
  }
}
