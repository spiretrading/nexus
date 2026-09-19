#include <doctest/doctest.h>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchMessage.hpp"

using namespace Nexus;
using namespace std::literals;

TEST_SUITE("AsxTradeItchMessage") {
  TEST_CASE("parse") {
    SUBCASE("seconds") {
      auto source = "T\x01\x02\x03\x04"sv;
      auto message = AsxTradeItchMessage::parse(source);
      REQUIRE(message.m_length == source.size());
      REQUIRE(message.m_type == 'T');
      REQUIRE(message.m_payload == source.data() +
        AsxTradeItchMessage::HEADER_LENGTH);
      auto cursor = message.get_cursor();
      REQUIRE(cursor.read_uint32() == 0x01020304);
      REQUIRE_THROWS_AS(cursor.read_char(), AsxTradeItchParserException);
    }
    SUBCASE("unknown") {
      auto message = AsxTradeItchMessage::parse("\xFF"sv);
      REQUIRE(message.m_type == 0xFF);
      REQUIRE(message.m_length == AsxTradeItchMessage::HEADER_LENGTH);
      REQUIRE_THROWS_AS(
        message.get_cursor().read_char(), AsxTradeItchParserException);
    }
    SUBCASE("empty") {
      REQUIRE_THROWS_AS(
        AsxTradeItchMessage::parse({}), AsxTradeItchParserException);
    }
    SUBCASE("invalid_length") {
      auto message = AsxTradeItchMessage();
      REQUIRE_THROWS_AS(message.get_cursor(), AsxTradeItchParserException);
    }
  }
}
