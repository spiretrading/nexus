#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Money.hpp"

using namespace Nexus;

TEST_SUITE("Money") {
  TEST_CASE("to_string") {
    REQUIRE(Money::ZERO.ToString() == "0.00");
    REQUIRE(Money::ONE.ToString() == "1.00");
    REQUIRE((Money::ONE / 10).ToString() == "0.10");
    REQUIRE((Money::ONE / 100).ToString() == "0.01");
    REQUIRE((Money::ONE / 1000).ToString() == "0.001");
    REQUIRE((Money::ONE / 10000).ToString() == "0.0001");
    REQUIRE((Money::ONE / 100000).ToString() == "0.00001");
    REQUIRE((Money::ONE / 1000000).ToString() == "0.000001");
    REQUIRE((13 * Money::CENT + Money::CENT / 2).ToString() == "0.135");
    REQUIRE((3 * Money::CENT + Money::CENT / 2).ToString() == "0.035");
  }

  TEST_CASE("from_string") {
    REQUIRE(Money::FromValue("0") == Money::ZERO);
    REQUIRE(Money::FromValue("-0") == Money::ZERO);
    REQUIRE(Money::FromValue("+0") == Money::ZERO);
    REQUIRE(Money::FromValue("+.") == Money::ZERO);
    REQUIRE(Money::FromValue("-.") == Money::ZERO);
    REQUIRE(Money::FromValue("0.01") == Money::CENT);
    REQUIRE(Money::FromValue("0.02") == 2 * Money::CENT);
    REQUIRE(Money::FromValue("0.10") == 10 * Money::CENT);
    REQUIRE(Money::FromValue("0.11") == 11 * Money::CENT);
    REQUIRE(Money::FromValue("0.12") == 12 * Money::CENT);
    REQUIRE(Money::FromValue("1.00") == Money::ONE);
    REQUIRE(Money::FromValue("1.01") == Money::ONE + Money::CENT);
    REQUIRE(Money::FromValue("1.02") == Money::ONE + 2 * Money::CENT);
    REQUIRE(Money::FromValue("1.10") == Money::ONE + 10 * Money::CENT);
    REQUIRE(Money::FromValue("1.11") == Money::ONE + 11 * Money::CENT);
    REQUIRE(Money::FromValue("1.12") == Money::ONE + 12 * Money::CENT);
    REQUIRE(Money::FromValue("2.01") == 2 * Money::ONE + Money::CENT);
    REQUIRE(Money::FromValue("+0.01") == Money::CENT);
    REQUIRE(Money::FromValue("+0.02") == 2 * Money::CENT);
    REQUIRE(Money::FromValue("+0.10") == 10 * Money::CENT);
    REQUIRE(Money::FromValue("+0.11") == 11 * Money::CENT);
    REQUIRE(Money::FromValue("+0.12") == 12 * Money::CENT);
    REQUIRE(Money::FromValue("+1.00") == Money::ONE);
    REQUIRE(Money::FromValue("+1.01") == Money::ONE + Money::CENT);
    REQUIRE(Money::FromValue("+1.02") == Money::ONE + 2 * Money::CENT);
    REQUIRE(Money::FromValue("+1.10") == Money::ONE + 10 * Money::CENT);
    REQUIRE(Money::FromValue("+1.11") == Money::ONE + 11 * Money::CENT);
    REQUIRE(Money::FromValue("+1.12") == Money::ONE + 12 * Money::CENT);
    REQUIRE(Money::FromValue("+2.01") == 2 * Money::ONE + Money::CENT);
    REQUIRE(Money::FromValue("-0.01") == -Money::CENT);
    REQUIRE(Money::FromValue("-0.02") == -(2 * Money::CENT));
    REQUIRE(Money::FromValue("-0.10") == -(10 * Money::CENT));
    REQUIRE(Money::FromValue("-0.11") == -(11 * Money::CENT));
    REQUIRE(Money::FromValue("-0.12") == -(12 * Money::CENT));
    REQUIRE(Money::FromValue("-1.00") == -Money::ONE);
    REQUIRE(Money::FromValue("-1.01") == -(Money::ONE + Money::CENT));
    REQUIRE(Money::FromValue("-1.02") == -(Money::ONE + 2 * Money::CENT));
    REQUIRE(Money::FromValue("-1.10") == -(Money::ONE + 10 * Money::CENT));
    REQUIRE(Money::FromValue("-1.11") == -(Money::ONE + 11 * Money::CENT));
    REQUIRE(Money::FromValue("-1.12") == -(Money::ONE + 12 * Money::CENT));
    REQUIRE(Money::FromValue("-2.01") == -(2 * Money::ONE + Money::CENT));
    REQUIRE(Money::FromValue(".01") == Money::CENT);
    REQUIRE(Money::FromValue(".02") == 2 * Money::CENT);
    REQUIRE(Money::FromValue(".10") == 10 * Money::CENT);
    REQUIRE(Money::FromValue(".11") == 11 * Money::CENT);
    REQUIRE(Money::FromValue(".12") == 12 * Money::CENT);
    REQUIRE(Money::FromValue("+.01") == Money::CENT);
    REQUIRE(Money::FromValue("+.02") == 2 * Money::CENT);
    REQUIRE(Money::FromValue("+.10") == 10 * Money::CENT);
    REQUIRE(Money::FromValue("+.11") == 11 * Money::CENT);
    REQUIRE(Money::FromValue("+.12") == 12 * Money::CENT);
    REQUIRE(Money::FromValue("-.01") == -Money::CENT);
    REQUIRE(Money::FromValue("-.02") == -(2 * Money::CENT));
    REQUIRE(Money::FromValue("-.10") == -(10 * Money::CENT));
    REQUIRE(Money::FromValue("-.11") == -(11 * Money::CENT));
    REQUIRE(Money::FromValue("-.12") == -(12 * Money::CENT));
    REQUIRE(Money::FromValue("0.00000100") == (Money::ONE / 1000000));
  }

  TEST_CASE("floor") {
    Money value = 2.055 * Money::CENT;
    Money flooredValue = Floor(value, 2);
    REQUIRE(flooredValue == 2 * Money::CENT);
  }

  TEST_CASE("round") {
    {
      auto value = Money::FromValue("75.504");
      auto roundValue = Round(*value, 2);
      REQUIRE(roundValue == Money::FromValue("75.50"));
    }
    {
      auto value = Money::FromValue("75.505");
      auto roundValue = Round(*value, 2);
      REQUIRE(roundValue == Money::FromValue("75.51"));
    }
    {
      auto value = Money::FromValue("75.506");
      auto roundValue = Round(*value, 2);
      REQUIRE(roundValue == Money::FromValue("75.51"));
    }
  }
}
