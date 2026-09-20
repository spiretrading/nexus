#include <ostream>
#include <doctest/doctest.h>
#include "Nexus/Stamp/StampFieldReader.hpp"

using namespace Nexus;

namespace {
  auto text(std::string_view source) {
    return source;
  }
}

TEST_SUITE("StampFieldReader") {
  TEST_CASE("get_count") {
    auto message = StampMessage::parse("\x01\x1e" "1=0\x1c"
      "\x1e" "70.9=002\x1e" "55=ABX\x1e" "40.3=1"
      "\x1e" "70.0=001\x1e" "64.9999=100");
    auto fields = StampFieldReader(message.m_business_content);
    REQUIRE(fields.get_count({}) == 0);
    REQUIRE(fields.get_count({41}) == 0);
    REQUIRE(fields.get_count({55}) == 1);
    REQUIRE(fields.get_count({70}) == 10);
    REQUIRE(fields.get_count({40, 55}) == 4);
    REQUIRE(fields.get_count({40, 70, 41}) == 10);
    REQUIRE(fields.get_count({70, 40, 70}) == 10);
    REQUIRE(fields.get_count({64}) == 10000);
  }

  TEST_CASE("duplicate_fields") {
    auto source = std::string_view();
    SUBCASE("scalar") {
      source = "\x01\x1e" "1=0\x1c\x1e" "55=ABX\x1e" "55=FOO";
    }
    SUBCASE("explicit_zero") {
      source = "\x01\x1e" "1=0\x1c\x1e" "55=ABX\x1e" "55.0=ABX";
    }
    SUBCASE("indexed") {
      source = "\x01\x1e" "1=0\x1c\x1e" "70.1=001\x1e" "70.1=002";
    }
    auto message = StampMessage::parse(source);
    REQUIRE_THROWS_AS(StampFieldReader(message.m_business_content),
      StampParserException);
  }

  TEST_CASE("read_optional") {
    auto message = StampMessage::parse("\x01\x1e" "1=0\x1c"
      "\x1e" "55=ABX\x1e" "41=\x1e" "70.1=002");
    auto fields = StampFieldReader(message.m_business_content);
    SUBCASE("present") {
      REQUIRE(fields.read_optional(55, text).value() == "ABX");
      REQUIRE(fields.read_optional(70, 1, text).value() == "002");
    }
    SUBCASE("missing") {
      auto calls = 0;
      auto parser = [&] (auto value) {
        ++calls;
        return value;
      };
      REQUIRE_FALSE(fields.read_optional(40, parser).has_value());
      REQUIRE_FALSE(fields.read_optional(70, 0, parser).has_value());
      REQUIRE(calls == 0);
    }
    SUBCASE("empty") {
      auto calls = 0;
      auto parser = [&] (auto value) {
        ++calls;
        REQUIRE(value.empty());
        return 123;
      };
      REQUIRE(fields.read_optional(41, parser).value() == 123);
      REQUIRE(calls == 1);
    }
    SUBCASE("indexed_scalar") {
      REQUIRE_THROWS_AS(fields.read_optional(70, text), StampParserException);
    }
    SUBCASE("parser_exception") {
      auto parser = [] (auto) -> int {
        throw std::invalid_argument("Invalid field.");
      };
      REQUIRE_THROWS_AS(fields.read_optional(55, parser),
        std::invalid_argument);
    }
  }

  TEST_CASE("read") {
    auto message = StampMessage::parse("\x01\x1e" "1=0\x1c"
      "\x1e" "70.1=002\x1e" "55=ABX\x1e" "70=001\x1e" "64=100");
    auto fields = StampFieldReader(message.m_business_content);
    SUBCASE("scalar") {
      REQUIRE(fields.read(55, text) == "ABX");
      REQUIRE(fields.read(64, [] (auto value) {
        return value.size();
      }) == 3);
    }
    SUBCASE("indexed") {
      REQUIRE(fields.read(70, 0, text) == "001");
      REQUIRE(fields.read(70, 1, text) == "002");
    }
    SUBCASE("missing") {
      REQUIRE_THROWS_AS(fields.read(40, text), StampParserException);
      REQUIRE_THROWS_AS(fields.read(70, 2, text), StampParserException);
    }
    SUBCASE("indexed_scalar") {
      REQUIRE_THROWS_AS(fields.read(70, text), StampParserException);
    }
    SUBCASE("parser_exception") {
      auto parser = [] (auto) -> int {
        throw std::invalid_argument("Invalid field.");
      };
      REQUIRE_THROWS_AS(fields.read(55, parser), std::invalid_argument);
    }
  }
}
