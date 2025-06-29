#include <boost/lexical_cast.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Money.hpp"

using namespace boost;
using namespace Nexus;

TEST_SUITE("Money") {
  TEST_CASE("to_string") {
    REQUIRE(lexical_cast<std::string>(Money::ZERO) == "0.00");
    REQUIRE(lexical_cast<std::string>(Money::ONE) == "1.00");
    REQUIRE(lexical_cast<std::string>(Money::ONE / 10) == "0.10");
    REQUIRE(lexical_cast<std::string>(Money::ONE / 100) == "0.01");
    REQUIRE(lexical_cast<std::string>(Money::ONE / 1000) == "0.001");
    REQUIRE(lexical_cast<std::string>(Money::ONE / 10000) == "0.0001");
    REQUIRE(lexical_cast<std::string>(Money::ONE / 100000) == "0.00001");
    REQUIRE(lexical_cast<std::string>(Money::ONE / 1000000) == "0.000001");
    REQUIRE(lexical_cast<std::string>(13 * Money::CENT + Money::CENT / 2) ==
      "0.135");
    REQUIRE(lexical_cast<std::string>(3 * Money::CENT + Money::CENT / 2) ==
      "0.035");
  }

  TEST_CASE("from_string") {
    REQUIRE(try_parse_money("0") == Money::ZERO);
    REQUIRE(try_parse_money("-0") == Money::ZERO);
    REQUIRE(try_parse_money("+0") == Money::ZERO);
    REQUIRE(try_parse_money("+.") == Money::ZERO);
    REQUIRE(try_parse_money("-.") == Money::ZERO);
    REQUIRE(try_parse_money("0.01") == Money::CENT);
    REQUIRE(try_parse_money("0.02") == 2 * Money::CENT);
    REQUIRE(try_parse_money("0.10") == 10 * Money::CENT);
    REQUIRE(try_parse_money("0.11") == 11 * Money::CENT);
    REQUIRE(try_parse_money("0.12") == 12 * Money::CENT);
    REQUIRE(try_parse_money("1.00") == Money::ONE);
    REQUIRE(try_parse_money("1.01") == Money::ONE + Money::CENT);
    REQUIRE(try_parse_money("1.02") == Money::ONE + 2 * Money::CENT);
    REQUIRE(try_parse_money("1.10") == Money::ONE + 10 * Money::CENT);
    REQUIRE(try_parse_money("1.11") == Money::ONE + 11 * Money::CENT);
    REQUIRE(try_parse_money("1.12") == Money::ONE + 12 * Money::CENT);
    REQUIRE(try_parse_money("2.01") == 2 * Money::ONE + Money::CENT);
    REQUIRE(try_parse_money("+0.01") == Money::CENT);
    REQUIRE(try_parse_money("+0.02") == 2 * Money::CENT);
    REQUIRE(try_parse_money("+0.10") == 10 * Money::CENT);
    REQUIRE(try_parse_money("+0.11") == 11 * Money::CENT);
    REQUIRE(try_parse_money("+0.12") == 12 * Money::CENT);
    REQUIRE(try_parse_money("+1.00") == Money::ONE);
    REQUIRE(try_parse_money("+1.01") == Money::ONE + Money::CENT);
    REQUIRE(try_parse_money("+1.02") == Money::ONE + 2 * Money::CENT);
    REQUIRE(try_parse_money("+1.10") == Money::ONE + 10 * Money::CENT);
    REQUIRE(try_parse_money("+1.11") == Money::ONE + 11 * Money::CENT);
    REQUIRE(try_parse_money("+1.12") == Money::ONE + 12 * Money::CENT);
    REQUIRE(try_parse_money("+2.01") == 2 * Money::ONE + Money::CENT);
    REQUIRE(try_parse_money("-0.01") == -Money::CENT);
    REQUIRE(try_parse_money("-0.02") == -(2 * Money::CENT));
    REQUIRE(try_parse_money("-0.10") == -(10 * Money::CENT));
    REQUIRE(try_parse_money("-0.11") == -(11 * Money::CENT));
    REQUIRE(try_parse_money("-0.12") == -(12 * Money::CENT));
    REQUIRE(try_parse_money("-1.00") == -Money::ONE);
    REQUIRE(try_parse_money("-1.01") == -(Money::ONE + Money::CENT));
    REQUIRE(try_parse_money("-1.02") == -(Money::ONE + 2 * Money::CENT));
    REQUIRE(try_parse_money("-1.10") == -(Money::ONE + 10 * Money::CENT));
    REQUIRE(try_parse_money("-1.11") == -(Money::ONE + 11 * Money::CENT));
    REQUIRE(try_parse_money("-1.12") == -(Money::ONE + 12 * Money::CENT));
    REQUIRE(try_parse_money("-2.01") == -(2 * Money::ONE + Money::CENT));
    REQUIRE(try_parse_money(".01") == Money::CENT);
    REQUIRE(try_parse_money(".02") == 2 * Money::CENT);
    REQUIRE(try_parse_money(".10") == 10 * Money::CENT);
    REQUIRE(try_parse_money(".11") == 11 * Money::CENT);
    REQUIRE(try_parse_money(".12") == 12 * Money::CENT);
    REQUIRE(try_parse_money("+.01") == Money::CENT);
    REQUIRE(try_parse_money("+.02") == 2 * Money::CENT);
    REQUIRE(try_parse_money("+.10") == 10 * Money::CENT);
    REQUIRE(try_parse_money("+.11") == 11 * Money::CENT);
    REQUIRE(try_parse_money("+.12") == 12 * Money::CENT);
    REQUIRE(try_parse_money("-.01") == -Money::CENT);
    REQUIRE(try_parse_money("-.02") == -(2 * Money::CENT));
    REQUIRE(try_parse_money("-.10") == -(10 * Money::CENT));
    REQUIRE(try_parse_money("-.11") == -(11 * Money::CENT));
    REQUIRE(try_parse_money("-.12") == -(12 * Money::CENT));
    REQUIRE(try_parse_money("0.00000100") == (Money::ONE / 1000000));
  }

  TEST_CASE("floor") {
    auto value = 2.055 * Money::CENT;
    auto floored_value = floor(value, 2);
    REQUIRE(floored_value == 2 * Money::CENT);
  }

  TEST_CASE("round") {
    {
      auto value = try_parse_money("75.504");
      auto round_value = round(*value, 2);
      REQUIRE(round_value == try_parse_money("75.50"));
    }
    {
      auto value = try_parse_money("75.505");
      auto round_value = round(*value, 2);
      REQUIRE(round_value == try_parse_money("75.51"));
    }
    {
      auto value = try_parse_money("75.506");
      auto round_value = round(*value, 2);
      REQUIRE(round_value == try_parse_money("75.51"));
    }
  }
}
