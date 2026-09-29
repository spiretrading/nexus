#include <ostream>
#include <doctest/doctest.h>
#include "Nexus/Stamp/StampField.hpp"

using namespace Nexus;
using namespace std::literals;

TEST_SUITE("StampField") {
  TEST_CASE("parse") {
    auto source = "\x1e" "55=ABX"sv;
    auto identifier = std::uint16_t(55);
    auto index = std::uint16_t(0);
    auto value = "ABX"sv;
    SUBCASE("unindexed") {}
    SUBCASE("explicit_zero") {
      source = "\x1e" "55.0=ABX"sv;
    }
    SUBCASE("indexed") {
      source = "\x1e" "55.3=ABX"sv;
      index = 3;
    }
    SUBCASE("leading_zeros") {
      source = "\x1e" "00055.00000=ABX"sv;
    }
    SUBCASE("maximum_identifiers") {
      source = "\x1e" "9999.9999=ABX"sv;
      identifier = 9999;
      index = 9999;
    }
    SUBCASE("empty_value") {
      source = "\x1e" "55="sv;
      value = ""sv;
    }
    SUBCASE("spaces_and_equals") {
      source = "\x1e" "55= A=B ~ "sv;
      value = " A=B ~ "sv;
    }
    auto field = StampField::parse(source);
    REQUIRE(field.m_identifier == identifier);
    REQUIRE(field.m_index == index);
    REQUIRE(field.m_value == value);
    REQUIRE(field.m_value.data() == source.data() + source.find('=') + 1);
  }

  TEST_CASE("malformed_field") {
    for(auto source : {""sv, "55=ABX"sv, "\x1e"sv, "\x1e" "55"sv,
        "\x1e" "=ABX"sv, "\x1e" "0=ABX"sv, "\x1e" "10000=ABX"sv,
        "\x1e" "000055=ABX"sv, "\x1e" "+55=ABX"sv,
        "\x1e" "-55=ABX"sv, "\x1e" "5X=ABX"sv,
        "\x1e" "55.=ABX"sv, "\x1e" "55.-1=ABX"sv,
        "\x1e" "55.+1=ABX"sv, "\x1e" "55.10000=ABX"sv,
        "\x1e" "55.000000=ABX"sv, "\x1e" "55.1.2=ABX"sv,
        "\x1e" "55.1X=ABX"sv, "\x1e" "55=ABX\0"sv,
        "\x1e" "55=ABX\x1e"sv, "\x1e" "55=ABX\x7f"sv,
        "\x1e" "55=ABX\x80"sv}) {
      CAPTURE(source);
      REQUIRE_THROWS_AS(StampField::parse(source), StampParserException);
    }
  }
}
