#include <limits>
#include <doctest/doctest.h>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchCursor.hpp"

using namespace Nexus;
using namespace std::literals;

TEST_SUITE("AsxTradeItchCursor") {
  TEST_CASE("read") {
    SUBCASE("integers") {
      auto source = "\x2A\x01\x2C\x00\x00\x02\xBC"
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF"sv;
      auto cursor = AsxTradeItchCursor(source);
      REQUIRE(cursor.read_uint8() == 42);
      REQUIRE(cursor.read_uint16() == 300);
      REQUIRE(cursor.read_uint32() == 700);
      REQUIRE(cursor.read_uint64() == 0x0123456789ABCDEFULL);
      REQUIRE_THROWS_AS(cursor.read_char(), AsxTradeItchParserException);
    }
    SUBCASE("unsigned_limits") {
      auto source = std::string(15, '\xFF');
      auto cursor = AsxTradeItchCursor(source);
      REQUIRE(cursor.read_uint8() ==
        std::numeric_limits<std::uint8_t>::max());
      REQUIRE(cursor.read_uint16() ==
        std::numeric_limits<std::uint16_t>::max());
      REQUIRE(cursor.read_uint32() ==
        std::numeric_limits<std::uint32_t>::max());
      REQUIRE(cursor.read_uint64() ==
        std::numeric_limits<std::uint64_t>::max());
    }
    SUBCASE("prices") {
      auto source = "\x00\x00\xD6\xD8\xFF\xFF\xFF\x9C"
        "\x80\x00\x00\x00\x7F\xFF\xFF\xFF"sv;
      auto cursor = AsxTradeItchCursor(source);
      REQUIRE(cursor.read_price() == 55000);
      REQUIRE(cursor.read_price() == -100);
      REQUIRE(cursor.read_price() ==
        std::numeric_limits<std::int32_t>::min());
      REQUIRE(cursor.read_price() ==
        std::numeric_limits<std::int32_t>::max());
    }
    SUBCASE("text") {
      auto cursor = AsxTradeItchCursor(" ABC  XXXX    \xE9!"sv);
      REQUIRE(cursor.read_text(6) == " ABC");
      REQUIRE(cursor.read_text(4) == "XXXX");
      REQUIRE(cursor.read_text(4).empty());
      REQUIRE(cursor.read_text(1) == "\xE9");
      REQUIRE(cursor.read_text(0).empty());
      REQUIRE(cursor.read_char() == '!');
      REQUIRE_THROWS_AS(cursor.read_text(1), AsxTradeItchParserException);
      auto empty = AsxTradeItchCursor({});
      REQUIRE(empty.read_text(0).empty());
      REQUIRE_NOTHROW(empty.skip(0));
    }
    SUBCASE("sides") {
      auto cursor = AsxTradeItchCursor("BS X"sv);
      REQUIRE(cursor.read_side() == Side::BID);
      REQUIRE(cursor.read_side() == Side::ASK);
      REQUIRE(cursor.read_side() == Side::NONE);
      REQUIRE_THROWS_AS(cursor.read_side(), AsxTradeItchParserException);
    }
    SUBCASE("skip") {
      auto cursor = AsxTradeItchCursor("abc"sv);
      REQUIRE_THROWS_AS(cursor.skip(4), AsxTradeItchParserException);
      cursor.skip(2);
      REQUIRE(cursor.read_char() == 'c');
      REQUIRE_THROWS_AS(cursor.skip(1), AsxTradeItchParserException);
    }
    SUBCASE("truncation") {
      auto check = [] (auto read, std::size_t size) {
        auto data = std::string(size, '\0');
        for(auto i = std::size_t(0); i != size; ++i) {
          auto cursor = AsxTradeItchCursor(std::string_view(data).substr(0, i));
          REQUIRE_THROWS_AS((cursor.*read)(), AsxTradeItchParserException);
        }
      };
      check(&AsxTradeItchCursor::read_uint8, sizeof(std::uint8_t));
      check(&AsxTradeItchCursor::read_uint16, sizeof(std::uint16_t));
      check(&AsxTradeItchCursor::read_uint32, sizeof(std::uint32_t));
      check(&AsxTradeItchCursor::read_uint64, sizeof(std::uint64_t));
      check(&AsxTradeItchCursor::read_price, sizeof(std::int32_t));
      check(&AsxTradeItchCursor::read_char, sizeof(char));
      check(&AsxTradeItchCursor::read_side, sizeof(char));
      auto cursor = AsxTradeItchCursor("AB"sv);
      REQUIRE_THROWS_AS(cursor.read_text(3), AsxTradeItchParserException);
      REQUIRE(cursor.read_text(2) == "AB");
    }
  }
}
