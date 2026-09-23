#include <limits>
#include <ostream>
#include <string>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkMessage.hpp"

using namespace Nexus;
using namespace std::literals;

TEST_SUITE("OtcLinkMessage") {
  TEST_CASE("parse") {
    auto source = "\x00\x07\xE6\x01\x02\x03\x04"sv;
    SUBCASE("fields") {
      auto message = OtcLinkMessage::parse(source);
      REQUIRE(message.m_length == source.size());
      REQUIRE(message.m_type == 0xE6);
      REQUIRE(message.m_payload == "\x01\x02\x03\x04"sv);
      REQUIRE(message.m_payload.data() ==
        source.data() + OtcLinkMessage::HEADER_LENGTH);
      auto cursor = message.get_cursor();
      REQUIRE(cursor.read_uint32() == 0x01020304);
      REQUIRE_THROWS_AS(cursor.read_uint8(), OtcLinkParserException);
    }
    SUBCASE("following_message") {
      auto bytes = std::string(source) + std::string(source);
      auto message = OtcLinkMessage::parse(bytes);
      REQUIRE(message.m_length == source.size());
      REQUIRE(message.m_payload ==
        source.substr(OtcLinkMessage::HEADER_LENGTH));
    }
    SUBCASE("empty_payload") {
      auto message = OtcLinkMessage::parse("\x00\x03\xFF"sv);
      REQUIRE(message.m_length == OtcLinkMessage::HEADER_LENGTH);
      REQUIRE(message.m_type == 0xFF);
      REQUIRE(message.m_payload.empty());
    }
    SUBCASE("maximum_length") {
      auto source = std::string(std::numeric_limits<std::uint16_t>::max(), 'x');
      source[0] = '\xFF';
      source[sizeof(std::uint8_t)] = '\xFF';
      source[sizeof(std::uint16_t)] = '\xE6';
      auto message = OtcLinkMessage::parse(source);
      REQUIRE(message.m_length == source.size());
      REQUIRE(message.m_payload.size() ==
        source.size() - OtcLinkMessage::HEADER_LENGTH);
      REQUIRE(message.m_payload.back() == 'x');
    }
    SUBCASE("truncation") {
      for(auto i = std::size_t(0); i < source.size(); ++i) {
        REQUIRE_THROWS_AS(OtcLinkMessage::parse(source.substr(0, i)),
          OtcLinkParserException);
      }
    }
    SUBCASE("short_length") {
      auto source = std::string(OtcLinkMessage::HEADER_LENGTH, '\0');
      for(auto i = std::size_t(0); i < OtcLinkMessage::HEADER_LENGTH; ++i) {
        source[sizeof(std::uint8_t)] = static_cast<char>(i);
        REQUIRE_THROWS_AS(
          OtcLinkMessage::parse(source), OtcLinkParserException);
      }
    }
  }
}
