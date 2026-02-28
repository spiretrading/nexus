#include <future>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::endian;
using namespace Nexus;

namespace {
  struct Fixture {
    LocalServerConnection m_server;
    optional<LocalClientChannel> m_client_channel;
    optional<OtcLinkClient<LocalClientChannel*>> m_client;
    std::unique_ptr<LocalServerChannel> m_server_channel;

    Fixture() {
      auto server_channel_async = std::async(std::launch::async, [&] {
        return m_server.accept();
      });
      m_client_channel.emplace("otc", m_server);
      m_client.emplace(&*m_client_channel);
      m_server_channel = server_channel_async.get();
    }
  };

  auto make_message_buffer(std::uint8_t type, const std::string& data) {
    auto message_size = std::uint16_t(3 + data.size());
    auto buffer = SharedBuffer();
    auto message_size_be = native_to_big(message_size);
    append(buffer, message_size_be);
    append(buffer, type);
    append(buffer, data.c_str(), data.size());
    return buffer;
  }

  auto make_packet_buffer(std::uint32_t sequence_number,
      std::uint8_t flag, const std::vector<SharedBuffer>& messages) {
    auto packet_size = std::uint16_t(OtcLinkPacket::HEADER_LENGTH);
    for(const auto& message : messages) {
      packet_size += message.get_size();
    }
    auto count = std::uint8_t(messages.size());
    auto buffer = SharedBuffer();
    auto packet_size_be = native_to_big(packet_size);
    append(buffer, packet_size_be);
    auto sequence_number_be = native_to_big(sequence_number);
    append(buffer, sequence_number_be);
    append(buffer, flag);
    append(buffer, count);
    auto milliseconds = std::uint32_t(0);
    auto milliseconds_be = native_to_big(milliseconds);
    append(buffer, milliseconds_be);
    for(const auto& message : messages) {
      append(buffer, message);
    }
    return buffer;
  }

  auto is_payload_equal(const void* data, const char* expected) {
    return std::memcmp(data, expected, std::strlen(expected)) == 0;
  }
}

TEST_SUITE("OtcLinkClient") {
  TEST_CASE("read_single_packet_single_message") {
    auto fixture = Fixture();
    auto packet =
      make_packet_buffer(42, 0, {make_message_buffer(0x11, "DATA")});
    fixture.m_server_channel->get_writer().write(packet);
    auto expected_sequence = std::uint32_t(0);
    auto message = fixture.m_client->read(out(expected_sequence));
    REQUIRE(message.m_type == OtcLinkMessage::Type::TRADE);
    REQUIRE(expected_sequence == 42);
    REQUIRE(is_payload_equal(message.m_payload, "DATA"));
  }

  TEST_CASE("read_single_packet_multiple_messages") {
    auto fixture = Fixture();
    auto packet = make_packet_buffer(100, 0,
      {make_message_buffer(0x01, "ONE"), make_message_buffer(0x02, "TWO")});
    fixture.m_server_channel->get_writer().write(packet);
    auto expected_sequence1 = std::uint32_t(0);
    auto message1 = fixture.m_client->read(out(expected_sequence1));
    REQUIRE(message1.m_type == OtcLinkMessage::Type::QUOTE);
    REQUIRE(expected_sequence1 == 100);
    REQUIRE(is_payload_equal(message1.m_payload, "ONE"));
    auto expected_sequence2 = std::uint32_t(0);
    auto message2 = fixture.m_client->read(out(expected_sequence2));
    REQUIRE(message2.m_type == OtcLinkMessage::Type::QUOTE_UPDATE);
    REQUIRE(expected_sequence2 == 100);
    REQUIRE(is_payload_equal(message2.m_payload, "TWO"));
  }

  TEST_CASE("read_empty_packet") {
    auto fixture = Fixture();
    auto empty_packet = make_packet_buffer(200, 0, {});
    auto packet =
      make_packet_buffer(201, 0, {make_message_buffer(0x03, "REAL")});
    fixture.m_server_channel->get_writer().write(empty_packet);
    fixture.m_server_channel->get_writer().write(packet);
    auto expected_sequence = std::uint32_t(0);
    auto message = fixture.m_client->read(out(expected_sequence));
    REQUIRE(message.m_type == OtcLinkMessage::Type::INSIDE);
    REQUIRE(expected_sequence == 201);
    REQUIRE(is_payload_equal(message.m_payload, "REAL"));
  }

  TEST_CASE("skip_heartbeat_packet") {
    auto fixture = Fixture();
    auto heartbeat = make_packet_buffer(
      300, static_cast<std::uint8_t>(OtcLinkPacket::Flag::HEARTBEAT), {});
    auto packet =
      make_packet_buffer(301, 0, {make_message_buffer(0x11, "AFTER")});
    fixture.m_server_channel->get_writer().write(heartbeat);
    fixture.m_server_channel->get_writer().write(packet);
    auto sequence = std::uint32_t(0);
    auto message = fixture.m_client->read(out(sequence));
    REQUIRE(message.m_type == OtcLinkMessage::Type::TRADE);
    REQUIRE(sequence == 301);
    REQUIRE(is_payload_equal(message.m_payload, "AFTER"));
  }

  TEST_CASE("sequence_number_reset") {
    auto fixture = Fixture();
    auto first_packet =
      make_packet_buffer(10, 0, {make_message_buffer(0x11, "FIRST")});
    auto reset_packet = make_packet_buffer(
      1, static_cast<std::uint8_t>(OtcLinkPacket::Flag::SEQUENCE_NUMBER_RESET),
      {});
    auto second_packet =
      make_packet_buffer(1, 0, {make_message_buffer(0x11, "SECOND")});
    fixture.m_server_channel->get_writer().write(first_packet);
    fixture.m_server_channel->get_writer().write(reset_packet);
    fixture.m_server_channel->get_writer().write(second_packet);
    auto sequence = std::uint32_t(0);
    fixture.m_client->read(out(sequence));
    REQUIRE(sequence == 10);
    auto message = fixture.m_client->read(out(sequence));
    REQUIRE(message.m_type == OtcLinkMessage::Type::TRADE);
    REQUIRE(sequence == 1);
    REQUIRE(is_payload_equal(message.m_payload, "SECOND"));
  }

  TEST_CASE("skip_replay_packet") {
    auto fixture = Fixture();
    auto replay = make_packet_buffer(
      400, static_cast<std::uint8_t>(OtcLinkPacket::Flag::REPLAY),
      {make_message_buffer(0x11, "REPLAY")});
    auto packet =
      make_packet_buffer(401, 0, {make_message_buffer(0x11, "LIVE")});
    fixture.m_server_channel->get_writer().write(replay);
    fixture.m_server_channel->get_writer().write(packet);
    auto sequence = std::uint32_t(0);
    auto message = fixture.m_client->read(out(sequence));
    REQUIRE(message.m_type == OtcLinkMessage::Type::TRADE);
    REQUIRE(sequence == 401);
    REQUIRE(is_payload_equal(message.m_payload, "LIVE"));
  }

  TEST_CASE("skip_test_packet") {
    auto fixture = Fixture();
    auto test_packet = make_packet_buffer(
      500, static_cast<std::uint8_t>(OtcLinkPacket::Flag::TEST),
      {make_message_buffer(0x11, "TEST")});
    auto packet =
      make_packet_buffer(501, 0, {make_message_buffer(0x11, "LIVE")});
    fixture.m_server_channel->get_writer().write(test_packet);
    fixture.m_server_channel->get_writer().write(packet);
    auto sequence = std::uint32_t(0);
    auto message = fixture.m_client->read(out(sequence));
    REQUIRE(message.m_type == OtcLinkMessage::Type::TRADE);
    REQUIRE(sequence == 501);
    REQUIRE(is_payload_equal(message.m_payload, "LIVE"));
  }
}
