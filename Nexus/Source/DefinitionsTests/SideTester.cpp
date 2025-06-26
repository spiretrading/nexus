#include <doctest/doctest.h>
#include "Nexus/Definitions/Side.hpp"
#include <sstream>
#include <string>

using namespace Nexus;

TEST_SUITE("Side") {
  TEST_CASE("pick") {
    auto ask_value(10);
    auto bid_value(20);
    auto& ask_result = pick(Side::ASK, ask_value, bid_value);
    CHECK(&ask_result == &ask_value);
    auto& bid_result = pick(Side::BID, ask_value, bid_value);
    CHECK(&bid_result == &bid_value);
    auto& none_result = pick(Side::NONE, ask_value, bid_value);
    CHECK((&none_result == &ask_value || &none_result == &bid_result));
  }

  TEST_CASE("get_direction") {
    CHECK(get_direction(Side::ASK) == -1);
    CHECK(get_direction(Side::BID) == 1);
    CHECK(get_direction(Side::NONE) == 0);
  }

  TEST_CASE("get_side") {
    CHECK(get_side(10) == Side::BID);
    CHECK(get_side(-10) == Side::ASK);
    CHECK(get_side(0) == Side::NONE);
  }

  TEST_CASE("get_opposite") {
    CHECK(get_opposite(Side::ASK) == Side::BID);
    CHECK(get_opposite(Side::BID) == Side::ASK);
    CHECK(get_opposite(Side::NONE) == Side::NONE);
  }

  TEST_CASE("to_char") {
    CHECK(to_char(Side::ASK) == 'A');
    CHECK(to_char(Side::BID) == 'B');
    CHECK(to_char(Side::NONE) == '?');
  }

  TEST_CASE("stream") {
    auto ss = std::ostringstream();
    ss << Side::ASK;
    CHECK(ss.str() == "ASK");
    ss.str(std::string());
    ss << Side::BID;
    CHECK(ss.str() == "BID");
    ss.str(std::string());
    ss << Side::NONE;
    CHECK(ss.str() == "NONE");
  }
}
