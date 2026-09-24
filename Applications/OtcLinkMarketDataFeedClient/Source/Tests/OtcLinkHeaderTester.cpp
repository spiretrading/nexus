#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkHeader.hpp"

using namespace Nexus;
using namespace std::literals;

TEST_SUITE("OtcLinkHeader") {
  TEST_CASE("parse") {
    auto source = "\x01\x02\x89\xAB\xCD\xEF\xC0\x02"
      "\x01\x02\x03\x04"sv;
    SUBCASE("fields") {
      auto header = OtcLinkHeader::parse(source);
      REQUIRE(header.m_length == 0x0102);
      REQUIRE(header.m_sequence == 0x89ABCDEF);
      REQUIRE(header.m_flags == 0xC0);
      REQUIRE(header.m_count == 2);
      REQUIRE(header.m_milliseconds == 0x01020304);
      REQUIRE(header.has_flag(OtcLinkHeader::Flag::REPLAY));
      REQUIRE(header.has_flag(OtcLinkHeader::Flag::TEST));
      REQUIRE_FALSE(header.has_flag(OtcLinkHeader::Flag::HEARTBEAT));
      REQUIRE_FALSE(header.has_flag(OtcLinkHeader::Flag::SEQUENCE_RESET));
    }
    SUBCASE("control_flags") {
      auto header = OtcLinkHeader::parse(
        "\x00\x0C\x00\x00\x00\x01\x03\x00\x00\x00\x00\x00"sv);
      REQUIRE(header.m_length == OtcLinkHeader::LENGTH);
      REQUIRE(header.m_sequence == 1);
      REQUIRE(header.m_count == 0);
      REQUIRE(header.m_milliseconds == 0);
      REQUIRE(header.has_flag(OtcLinkHeader::Flag::HEARTBEAT));
      REQUIRE(header.has_flag(OtcLinkHeader::Flag::SEQUENCE_RESET));
      REQUIRE_FALSE(header.has_flag(OtcLinkHeader::Flag::REPLAY));
      REQUIRE_FALSE(header.has_flag(OtcLinkHeader::Flag::TEST));
    }
    SUBCASE("individual_flags") {
      for(auto [bits, expected] : {
          std::pair(0x01, OtcLinkHeader::Flag::HEARTBEAT),
          std::pair(0x02, OtcLinkHeader::Flag::SEQUENCE_RESET),
          std::pair(0x40, OtcLinkHeader::Flag::REPLAY),
          std::pair(0x80, OtcLinkHeader::Flag::TEST)}) {
        auto bytes = std::string(source);
        bytes[sizeof(std::uint16_t) + sizeof(std::uint32_t)] =
          static_cast<char>(bits);
        auto header = OtcLinkHeader::parse(bytes);
        for(auto flag : {OtcLinkHeader::Flag::HEARTBEAT,
            OtcLinkHeader::Flag::SEQUENCE_RESET, OtcLinkHeader::Flag::REPLAY,
            OtcLinkHeader::Flag::TEST}) {
          if(flag == expected) {
            REQUIRE(header.has_flag(flag));
          } else {
            REQUIRE_FALSE(header.has_flag(flag));
          }
        }
      }
    }
    SUBCASE("reserved_flags") {
      auto header = OtcLinkHeader::parse(
        "\x00\x0C\x00\x00\x00\x01\x3C\x00\x00\x00\x00\x00"sv);
      REQUIRE(header.m_flags == 0x3C);
      REQUIRE_FALSE(header.has_flag(OtcLinkHeader::Flag::HEARTBEAT));
      REQUIRE_FALSE(header.has_flag(OtcLinkHeader::Flag::SEQUENCE_RESET));
      REQUIRE_FALSE(header.has_flag(OtcLinkHeader::Flag::REPLAY));
      REQUIRE_FALSE(header.has_flag(OtcLinkHeader::Flag::TEST));
    }
    SUBCASE("truncation") {
      for(auto i = std::size_t(0); i < OtcLinkHeader::LENGTH; ++i) {
        REQUIRE_THROWS_AS(OtcLinkHeader::parse(source.substr(0, i)),
          OtcLinkParserException);
      }
    }
    SUBCASE("short_length") {
      auto bytes = std::string(OtcLinkHeader::LENGTH, '\0');
      for(auto i = std::size_t(0); i < OtcLinkHeader::LENGTH; ++i) {
        bytes[sizeof(std::uint8_t)] = static_cast<char>(i);
        REQUIRE_THROWS_AS(OtcLinkHeader::parse(bytes), OtcLinkParserException);
      }
    }
  }
}
