#include <future>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <doctest/doctest.h>
#include "Nexus/MoldUdp64/MoldUdp64Client.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace boost;
using namespace Nexus;

namespace {
  struct Fixture {
    LocalServerConnection<SharedBuffer> m_server;
    optional<LocalClientChannel<SharedBuffer>> m_client_channel;
    optional<MoldUdp64Client<LocalClientChannel<SharedBuffer>*>> m_client;
    std::unique_ptr<LocalServerChannel<SharedBuffer>> m_server_channel;

    Fixture() {
      auto server_channel_async = std::async(std::launch::async, [&] {
        return m_server.Accept();
      });
      m_client_channel.emplace("mold_udp", m_server);
      m_client.emplace(&*m_client_channel);
      m_server_channel = server_channel_async.get();
    }
  };

  auto make_message_buffer(std::uint8_t type, const std::string& data) {
    auto message_length = std::uint16_t(1 + data.size());
    auto buffer = SharedBuffer();
    auto message_length_be = ToBigEndian(message_length);
    buffer.Append(message_length_be);
    buffer.Append(type);
    buffer.Append(data.c_str(), data.size());
    return buffer;
  }

  auto make_packet_buffer(const std::string& session,
      std::uint64_t sequence_number,
      const std::vector<SharedBuffer>& messages) {
    auto count = std::uint16_t(messages.size());
    auto buffer = SharedBuffer();
    buffer.Append(session.c_str(), std::min<std::size_t>(session.size(), 10));
    for(auto i = session.size(); i < 10; ++i) {
      buffer.Append(std::uint8_t(0));
    }
    auto sequence_number_be = ToBigEndian(sequence_number);
    buffer.Append(sequence_number_be);
    auto count_be = ToBigEndian(count);
    buffer.Append(count_be);
    for(const auto& message : messages) {
      buffer.Append(message);
    }
    return buffer;
  }

  auto is_equal(const char* data, const char* expected) {
    return std::memcmp(data, expected, std::strlen(expected)) == 0;
  }
}

TEST_SUITE("MoldUdp64Client") {
  TEST_CASE("read_single_packet_single_message") {
    auto fixture = Fixture();
    auto packet = make_packet_buffer(
      "SESSION01", 42, {make_message_buffer(0xAB, "DATA")});
    fixture.m_server_channel->GetWriter().Write(packet);
    auto expected_sequence = std::uint64_t(0);
    auto message = fixture.m_client->read(Store(expected_sequence));
    REQUIRE(message.m_message_type == 0xAB);
    REQUIRE(expected_sequence == 42);
    REQUIRE(is_equal(message.m_data, "DATA"));
  }

  TEST_CASE("read_single_packet_multiple_messages") {
    auto fixture = Fixture();
    auto packet = make_packet_buffer("SESSION02", 100,
      {make_message_buffer(0xC1, "ONE"), make_message_buffer(0xC2, "TWO")});
    fixture.m_server_channel->GetWriter().Write(packet);
    auto expected_sequence1 = std::uint64_t(0);
    auto message1 = fixture.m_client->read(Store(expected_sequence1));
    REQUIRE(message1.m_message_type == 0xC1);
    REQUIRE(expected_sequence1 == 100);
    REQUIRE(is_equal(message1.m_data, "ONE"));
    auto expected_sequence2 = std::uint64_t(0);
    auto message2 = fixture.m_client->read(Store(expected_sequence2));
    REQUIRE(message2.m_message_type == 0xC2);
    REQUIRE(expected_sequence2 == 101);
    REQUIRE(is_equal(message2.m_data, "TWO"));
  }

  TEST_CASE("read_empty_packet") {
    auto fixture = Fixture();
    auto empty_packet = make_packet_buffer("SESSION00", 200, {});
    auto packet =
      make_packet_buffer("SESSION01", 201, {make_message_buffer(0xD1, "REAL")});
    fixture.m_server_channel->GetWriter().Write(empty_packet);
    fixture.m_server_channel->GetWriter().Write(packet);
    auto expected_sequence = std::uint64_t(0);
    auto message = fixture.m_client->read(Store(expected_sequence));
    REQUIRE(message.m_message_type == 0xD1);
    REQUIRE(expected_sequence == 201);
    REQUIRE(is_equal(message.m_data, "REAL"));
  }
}
