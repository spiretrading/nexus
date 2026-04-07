#include <array>
#include <cstring>
#include <doctest/doctest.h>
#include "Nexus/MoldUdp64/MoldUdp64Packet.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::endian;
using namespace Nexus;

TEST_SUITE("MoldUdp64Packet") {
  TEST_CASE("parse_valid_packet") {
    auto session = "ABCDEFGHIJ";
    auto sequence_number = std::uint64_t(0x0102030405060708);
    auto count = std::uint16_t(0x1122);
    auto payload = "PAYLOAD";
    const auto payload_length = sizeof(payload) - 1;
    auto buffer =
      std::array<char, MoldUdp64Packet::PACKET_LENGTH + payload_length>();
    std::memcpy(buffer.data(), session, MoldUdp64Packet::SESSION_FIELD_LENGTH);
    auto sequence_be = native_to_big(sequence_number);
    std::memcpy(buffer.data() + MoldUdp64Packet::SESSION_FIELD_LENGTH,
      &sequence_be, sizeof(std::uint64_t));
    auto count_be = native_to_big(count);
    std::memcpy(buffer.data() + MoldUdp64Packet::SESSION_FIELD_LENGTH +
      sizeof(std::uint64_t), &count_be, sizeof(std::uint16_t));
    std::memcpy(
      buffer.data() + MoldUdp64Packet::PACKET_LENGTH, payload, payload_length);
    auto packet =
      MoldUdp64Packet::parse(std::string_view(buffer.data(), buffer.size()));
    REQUIRE(packet.m_session == session);
    REQUIRE(packet.m_sequence_number == sequence_number);
    REQUIRE(packet.m_count == count);
    REQUIRE(std::memcmp(packet.m_payload, payload, payload_length) == 0);
  }

  TEST_CASE("parse_too_short_packet") {
    const auto short_length = std::size_t(10);
    auto buffer = std::array<char, short_length>();
    REQUIRE_THROWS_AS(
      MoldUdp64Packet::parse(std::string_view(buffer.data(), buffer.size())),
      MoldUdp64ParserException);
  }

  TEST_CASE("parse_correct_endianness") {
    auto session = "ABCDEFGHIJ";
    auto sequence_number = std::uint64_t(0x0102030405060708);
    auto count = std::uint16_t(0x1122);
    auto payload = "PAYLOAD";
    const auto payload_length = sizeof("PAYLOAD") - 1;
    auto buffer =
      std::array<char, MoldUdp64Packet::PACKET_LENGTH + payload_length>();
    std::memcpy(buffer.data(), session, MoldUdp64Packet::SESSION_FIELD_LENGTH);
    auto sequence_be = native_to_big(sequence_number);
    std::memcpy(buffer.data() + MoldUdp64Packet::SESSION_FIELD_LENGTH,
      &sequence_be, sizeof(std::uint64_t));
    auto count_be = native_to_big(count);
    std::memcpy(buffer.data() + MoldUdp64Packet::SESSION_FIELD_LENGTH +
      sizeof(std::uint64_t), &count_be, sizeof(std::uint16_t));
    std::memcpy(
      buffer.data() + MoldUdp64Packet::PACKET_LENGTH, payload, payload_length);
    auto packet =
      MoldUdp64Packet::parse(std::string_view(buffer.data(), buffer.size()));
    REQUIRE(packet.m_sequence_number == sequence_number);
    REQUIRE(packet.m_count == count);
  }

  TEST_CASE("parse_payload_pointer") {
    auto session = "ABCDEFGHIJ";
    auto sequence_number = std::uint64_t(0x0102030405060708);
    auto count = std::uint16_t(0x1122);
    auto payload = "PAYLOAD";
    const auto payload_length = sizeof("PAYLOAD") - 1;
    auto buffer =
      std::array<char, MoldUdp64Packet::PACKET_LENGTH + payload_length>();
    std::memcpy(buffer.data(), session, MoldUdp64Packet::SESSION_FIELD_LENGTH);
    auto sequence_be = native_to_big(sequence_number);
    std::memcpy(buffer.data() + MoldUdp64Packet::SESSION_FIELD_LENGTH,
      &sequence_be, sizeof(std::uint64_t));
    auto count_be = native_to_big(count);
    std::memcpy(buffer.data() + MoldUdp64Packet::SESSION_FIELD_LENGTH +
      sizeof(std::uint64_t), &count_be, sizeof(std::uint16_t));
    std::memcpy(
      buffer.data() + MoldUdp64Packet::PACKET_LENGTH, payload, payload_length);
    auto packet =
      MoldUdp64Packet::parse(std::string_view(buffer.data(), buffer.size()));
    REQUIRE(packet.m_payload == buffer.data() + MoldUdp64Packet::PACKET_LENGTH);
    REQUIRE(std::memcmp(packet.m_payload, payload, payload_length) == 0);
  }

  TEST_CASE("parse_session_field_length") {
    auto session = "1234567890";
    auto sequence_number = std::uint64_t(0x0102030405060708);
    auto count = std::uint16_t(0x1122);
    auto payload = "PAYLOAD";
    const auto payload_length = sizeof("PAYLOAD") - 1;
    auto buffer =
      std::array<char, MoldUdp64Packet::PACKET_LENGTH + payload_length>();
    std::memcpy(buffer.data(), session, MoldUdp64Packet::SESSION_FIELD_LENGTH);
    auto sequence_be = native_to_big(sequence_number);
    std::memcpy(buffer.data() + MoldUdp64Packet::SESSION_FIELD_LENGTH,
      &sequence_be, sizeof(std::uint64_t));
    auto count_be = native_to_big(count);
    std::memcpy(buffer.data() + MoldUdp64Packet::SESSION_FIELD_LENGTH +
      sizeof(std::uint64_t), &count_be, sizeof(std::uint16_t));
    std::memcpy(
      buffer.data() + MoldUdp64Packet::PACKET_LENGTH, payload, payload_length);
    auto packet =
      MoldUdp64Packet::parse(std::string_view(buffer.data(), buffer.size()));
    REQUIRE(packet.m_session == session);
  }
}
