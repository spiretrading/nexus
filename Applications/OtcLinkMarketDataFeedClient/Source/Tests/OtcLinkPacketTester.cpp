#include <array>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkPacket.hpp"

using namespace Nexus;

TEST_SUITE("OtcLinkPacket") {
  TEST_CASE("parse_valid_packet") {
    auto data = std::array<char, OtcLinkPacket::HEADER_LENGTH>{
      0x00, 0x0C,
      0x00, 0x00, 0x00, 0x05,
      0x01,
      0x03,
      0x00, 0x01, 0x51, static_cast<char>(0x80)
    };
    auto packet = OtcLinkPacket::parse(
      std::string_view(data.data(), data.size()));
    REQUIRE(packet.m_size == 12);
    REQUIRE(packet.m_sequence_number == 5);
    REQUIRE(packet.m_flag == OtcLinkPacket::Flag::HEARTBEAT);
    REQUIRE(packet.m_message_count == 3);
    REQUIRE(packet.m_milliseconds == 86400);
    REQUIRE(packet.m_payload == data.data() + OtcLinkPacket::HEADER_LENGTH);
  }

  TEST_CASE("parse_packet_too_short") {
    auto data = std::array<char, 11>();
    REQUIRE_THROWS_AS(
      OtcLinkPacket::parse(std::string_view(data.data(), data.size())),
      OtcLinkParserException);
  }
}
