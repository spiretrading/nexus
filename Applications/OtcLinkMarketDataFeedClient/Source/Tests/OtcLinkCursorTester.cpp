#include <limits>
#include <ostream>
#include <string>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkCursor.hpp"

using namespace Nexus;
using namespace std::literals;

TEST_SUITE("OtcLinkCursor") {
  TEST_CASE("read") {
    SUBCASE("integers") {
      auto source = "!\xAB\x01\x23\x45\x67\x89\xAB"
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF"sv;
      auto cursor = OtcLinkCursor(source.substr(1));
      REQUIRE(cursor.read_uint8() == 0xAB);
      REQUIRE(cursor.read_uint16() == 0x0123);
      REQUIRE(cursor.read_uint32() == 0x456789AB);
      REQUIRE(cursor.read_uint64() == 0x0123456789ABCDEFULL);
      REQUIRE_THROWS_AS(cursor.read_uint8(), OtcLinkParserException);
    }
    SUBCASE("unsigned_limits") {
      auto source = std::string(sizeof(std::uint8_t) + sizeof(std::uint16_t) +
        sizeof(std::uint32_t) + sizeof(std::uint64_t), '\xFF');
      auto cursor = OtcLinkCursor(source);
      REQUIRE(cursor.read_uint8() == std::numeric_limits<std::uint8_t>::max());
      REQUIRE(cursor.read_uint16() ==
        std::numeric_limits<std::uint16_t>::max());
      REQUIRE(cursor.read_uint32() ==
        std::numeric_limits<std::uint32_t>::max());
      REQUIRE(cursor.read_uint64() ==
        std::numeric_limits<std::uint64_t>::max());
    }
    SUBCASE("bytes") {
      auto source = "A\0B "sv;
      auto cursor = OtcLinkCursor(source);
      REQUIRE_THROWS_AS(cursor.read_bytes(source.size() + 1),
        OtcLinkParserException);
      REQUIRE_THROWS_AS(cursor.read_bytes(
        std::numeric_limits<std::size_t>::max()), OtcLinkParserException);
      auto value = cursor.read_bytes(source.size());
      REQUIRE(value == source);
      REQUIRE(value.data() == source.data());
      REQUIRE(cursor.read_bytes(0).empty());
      REQUIRE_THROWS_AS(cursor.read_bytes(1), OtcLinkParserException);
    }
    SUBCASE("skip") {
      auto cursor = OtcLinkCursor("abc"sv);
      REQUIRE_THROWS_AS(cursor.skip(4), OtcLinkParserException);
      cursor.skip(2);
      REQUIRE(cursor.read_bytes(1) == "c");
      REQUIRE_NOTHROW(cursor.skip(0));
      REQUIRE_THROWS_AS(cursor.skip(1), OtcLinkParserException);
      auto empty = OtcLinkCursor({});
      REQUIRE(empty.read_bytes(0).empty());
      REQUIRE_NOTHROW(empty.skip(0));
      REQUIRE_THROWS_AS(empty.read_uint8(), OtcLinkParserException);
    }
    SUBCASE("truncation") {
      auto check = [] (auto read, std::size_t size) {
        auto source = std::string(size, '\0');
        for(auto i = std::size_t(0); i < size; ++i) {
          auto prefix = std::string_view(source).substr(0, i);
          auto cursor = OtcLinkCursor(prefix);
          REQUIRE_THROWS_AS((cursor.*read)(), OtcLinkParserException);
          REQUIRE(cursor.read_bytes(i) == prefix);
        }
      };
      check(&OtcLinkCursor::read_uint8, sizeof(std::uint8_t));
      check(&OtcLinkCursor::read_uint16, sizeof(std::uint16_t));
      check(&OtcLinkCursor::read_uint32, sizeof(std::uint32_t));
      check(&OtcLinkCursor::read_uint64, sizeof(std::uint64_t));
    }
  }
}
