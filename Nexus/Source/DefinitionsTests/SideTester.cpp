#include <doctest/doctest.h>
#include "Nexus/Definitions/Side.hpp"

using namespace Nexus;

TEST_SUITE("Side") {
  TEST_CASE("pick") {
    auto ask_value(10);
    auto bid_value(20);
    auto& ask_result = pick(Side::ASK, ask_value, bid_value);
    REQUIRE(&ask_result == &ask_value);
    auto& bid_result = pick(Side::BID, ask_value, bid_value);
    REQUIRE(&bid_result == &bid_value);
    auto& none_result = pick(Side::NONE, ask_value, bid_value);
    REQUIRE((&none_result == &ask_value || &none_result == &bid_result));
  }

  TEST_CASE("get_direction") {
    REQUIRE(get_direction(Side::ASK) == -1);
    REQUIRE(get_direction(Side::BID) == 1);
    REQUIRE(get_direction(Side::NONE) == 0);
  }

  TEST_CASE("get_side") {
    REQUIRE(get_side(10) == Side::BID);
    REQUIRE(get_side(-10) == Side::ASK);
    REQUIRE(get_side(0) == Side::NONE);
  }

  TEST_CASE("get_opposite") {
    REQUIRE(get_opposite(Side::ASK) == Side::BID);
    REQUIRE(get_opposite(Side::BID) == Side::ASK);
    REQUIRE(get_opposite(Side::NONE) == Side::NONE);
  }

  TEST_CASE("to_char") {
    REQUIRE(to_char(Side::ASK) == 'A');
    REQUIRE(to_char(Side::BID) == 'B');
    REQUIRE(to_char(Side::NONE) == '?');
  }
}
