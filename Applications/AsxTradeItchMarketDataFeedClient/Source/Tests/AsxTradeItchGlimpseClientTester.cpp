#include <future>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchGlimpseClient.hpp"
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchSequencer.hpp"
#include "Nexus/MoldUdp64/MoldUdp64Request.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace std::literals;

namespace {
  struct Fixture {
    LocalServerConnection m_server;
    std::unique_ptr<LocalClientChannel> m_channel;
    std::unique_ptr<LocalServerChannel> m_server_channel;
    TriggerTimer m_timer;
    optional<AsxTradeItchGlimpseClient<LocalClientChannel*, TriggerTimer*>>
      m_client;

    Fixture() {
      auto connection = std::async(std::launch::async, [&] {
        return m_server.accept();
      });
      m_channel = std::make_unique<LocalClientChannel>("glimpse", m_server);
      m_server_channel = connection.get();
    }

    void send(char type, std::string_view payload) {
      auto buffer = SharedBuffer();
      append(buffer, endian::native_to_big(
        static_cast<std::uint16_t>(sizeof(type) + payload.size())));
      append(buffer, type);
      append(buffer, payload);
      m_server_channel->get_writer().write(buffer);
    }

    void log_in(std::uint64_t sequence) {
      auto value = std::to_string(sequence);
      send('A', "SESSION123" + std::string(
        LoginAcceptedPacket::SEQUENCE_LENGTH - value.size(), ' ') + value);
      m_client.emplace("user", "pass", m_channel.get(), &m_timer);
    }

    void finish(std::uint64_t sequence) {
      auto value = std::to_string(sequence);
      send('S', "G" + std::string(
        AsxTradeItchEndOfSnapshot::LENGTH - 1 - value.size(), ' ') + value);
    }
  };
}

TEST_SUITE("AsxTradeItchGlimpseClient") {
  TEST_CASE_FIXTURE(Fixture, "login") {
    SUBCASE("initial_sequence") {
      log_in(1);
      auto login = SharedBuffer();
      m_server_channel->get_reader().read(out(login));
      auto expected = SharedBuffer();
      make_login_request_packet("user", "pass", "", 1, out(expected));
      REQUIRE(login == expected);
    }
    SUBCASE("incomplete_image") {
      REQUIRE_THROWS_AS(log_in(2), ConnectException);
    }
  }

  TEST_CASE_FIXTURE(Fixture, "snapshot") {
    log_in(1);
    auto count = std::size_t(0);
    SUBCASE("empty") {}
    SUBCASE("messages") {
      send('S', "T\x00\x00\x00\x01"sv);
      send('H', "");
      send('+', "debug");
      send('S', "T\x00\x00\x00\x02"sv);
      count = 2;
    }
    finish(123);
    auto snapshot = m_client->load_snapshot();
    REQUIRE(snapshot.m_sequence == 123);
    REQUIRE(snapshot.m_messages.size() == count);
    if(count != 0) {
      REQUIRE(snapshot.m_messages[0] == SharedBuffer(
        "T\x00\x00\x00\x01", AsxTradeItchSeconds::LENGTH));
      REQUIRE(snapshot.m_messages[1] == SharedBuffer(
        "T\x00\x00\x00\x02", AsxTradeItchSeconds::LENGTH));
    }
    auto login = SharedBuffer();
    m_server_channel->get_reader().read(out(login));
    REQUIRE_THROWS_AS(
      m_server_channel->get_reader().read(out(login)), IOException);
  }

  TEST_CASE_FIXTURE(Fixture, "interrupted_snapshot") {
    log_in(1);
    send('S', "T\x00\x00\x00\x01"sv);
    SUBCASE("end_of_session") {
      send('Z', "");
    }
    SUBCASE("disconnect") {
      m_server_channel->get_connection().close();
    }
    REQUIRE_THROWS_AS(m_client->load_snapshot(), IOException);
  }

  TEST_CASE_FIXTURE(Fixture, "malformed_snapshot") {
    log_in(1);
    send('S', "T\x00\x00\x00\x01"sv);
    SUBCASE("empty_message") {
      send('S', "");
    }
    SUBCASE("truncated_message") {
      send('S', "A");
    }
    SUBCASE("truncated_finish") {
      send('S', "G");
    }
    SUBCASE("invalid_sequence") {
      send('S', "G                   x");
    }
    finish(123);
    REQUIRE_THROWS_AS(
      m_client->load_snapshot(), AsxTradeItchParserException);
  }

  TEST_CASE_FIXTURE(Fixture, "close") {
    log_in(1);
    auto snapshot = std::async(std::launch::async, [&] {
      return m_client->load_snapshot();
    });
    m_client->close();
    REQUIRE_THROWS_AS(snapshot.get(), IOException);
  }

  TEST_CASE_FIXTURE(Fixture, "multicast_handoff") {
    auto sequencer = AsxTradeItchSequencer(
      1, posix_time::duration_from_string("00:00:03"));
    auto source = SharedBuffer();
    encode(MoldUdp64Request("SESSION123", 122, 3), out(source));
    for(auto message : {"T\x00\x00\x00\x01"sv, "T\x00\x00\x00\x02"sv,
        "T\x00\x00\x00\x03"sv}) {
      append(source, endian::native_to_big(
        static_cast<std::uint16_t>(message.size())));
      append(source, message);
    }
    sequencer.add(0, MoldUdp64Packet::parse(
      std::string_view(source.get_data(), source.get_size())),
      posix_time::time_from_string("2026-09-19 10:00:00"));
    log_in(1);
    finish(123);
    auto snapshot = m_client->load_snapshot();
    sequencer.reset(snapshot.m_sequence);
    auto message = sequencer.read();
    REQUIRE(message.has_value());
    REQUIRE(*message == "T\x00\x00\x00\x02"sv);
    message = sequencer.read();
    REQUIRE(message.has_value());
    REQUIRE(*message == "T\x00\x00\x00\x03"sv);
    REQUIRE(!sequencer.read());
  }
}
