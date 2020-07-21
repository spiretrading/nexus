#include <boost/lexical_cast.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Quantity.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

TEST_SUITE("Quantity") {
  TEST_CASE("to_string") {
    REQUIRE(lexical_cast<std::string>(Quantity(0)) == "0");
    REQUIRE(lexical_cast<std::string>(Quantity(1)) == "1");
    REQUIRE(lexical_cast<std::string>(Quantity(1.1)) == "1.100000");
  }

  TEST_CASE("from_string") {
    REQUIRE(Quantity::FromValue("1") == Quantity(1));
    REQUIRE(Quantity::FromValue("1.1") == Quantity(1.1));
  }

  TEST_CASE("assignment") {
    auto q = Quantity(0);
    q = 100;
    REQUIRE(q == 100);
  }

  TEST_CASE("modulus") {
    REQUIRE(Quantity(1) % Quantity(5) == Quantity(1));
    REQUIRE(Quantity(3) % Quantity(2) == Quantity(1));
  }

  TEST_CASE("floor") {
    REQUIRE(Floor(Quantity(1), 0) == Quantity(1));
    REQUIRE(Floor(Quantity(-1), 0) == Quantity(-1));
    REQUIRE(Floor(Quantity(10), -1) == Quantity(10));
    REQUIRE(Floor(Quantity(-10), -1) == Quantity(-10));
    REQUIRE(Floor(Quantity(10), 1) == Quantity(10));
    REQUIRE(Floor(Quantity(-10), 1) == Quantity(-10));
    REQUIRE(Floor(*Quantity::FromValue("-1.5"), 0) == Quantity(-2));
    REQUIRE(Floor(*Quantity::FromValue("-1.5"), -1) == Quantity(-10));
    REQUIRE(Floor(*Quantity::FromValue("-0.5"), -1) == Quantity(-10));
    REQUIRE(Floor(*Quantity::FromValue("0.5"), -1) == Quantity(0));
    REQUIRE(Floor(*Quantity::FromValue("0.5"), 0) == Quantity(0));
    REQUIRE(Floor(*Quantity::FromValue("0.5"), 1) ==
      Quantity::FromValue("0.5"));
    REQUIRE(Floor(*Quantity::FromValue("0.5"), 2) ==
      Quantity::FromValue("0.5"));
    REQUIRE(Floor(Quantity(5), 0) == Quantity(5));
    REQUIRE(Floor(Quantity(5), -1) == Quantity(0));
    REQUIRE(Floor(Quantity(5), -2) == Quantity(0));
    REQUIRE(Floor(Quantity(37), 0) == Quantity(37));
    REQUIRE(Floor(Quantity(37), -1) == Quantity(30));
    REQUIRE(Floor(Quantity(37), -2) == Quantity(0));
    REQUIRE(Floor(Quantity(33), 0) == Quantity(33));
    REQUIRE(Floor(Quantity(33), -1) == Quantity(30));
    REQUIRE(Floor(Quantity(33), -2) == Quantity(0));
    REQUIRE(Floor(Quantity(73), 0) == Quantity(73));
    REQUIRE(Floor(Quantity(73), -1) == Quantity(70));
    REQUIRE(Floor(Quantity(73), -2) == Quantity(0));
    REQUIRE(Floor(*Quantity::FromValue("555.555"), -3) == Quantity(0));
    REQUIRE(Floor(*Quantity::FromValue("555.555"), -2) == Quantity(500));
    REQUIRE(Floor(*Quantity::FromValue("555.555"), -1) == Quantity(550));
    REQUIRE(Floor(*Quantity::FromValue("555.555"), 0) == Quantity(555));
    REQUIRE(Floor(*Quantity::FromValue("555.555"), 1) ==
      Quantity::FromValue("555.5"));
    REQUIRE(Floor(*Quantity::FromValue("555.555"), 2) ==
      Quantity::FromValue("555.55"));
    REQUIRE(Floor(*Quantity::FromValue("555.555"), 3) ==
      Quantity::FromValue("555.555"));
    REQUIRE(Floor(*Quantity::FromValue("555.555"), 4) ==
      Quantity::FromValue("555.555"));
    REQUIRE(Floor(*Quantity::FromValue("-555.555"), 1) ==
      Quantity::FromValue("-555.6"));
  }

  TEST_CASE("ceil") {
    REQUIRE(Ceil(Quantity(1), 0) == Quantity(1));
    REQUIRE(Ceil(Quantity(-1), 0) == Quantity(-1));
    REQUIRE(Ceil(Quantity(10), -1) == Quantity(10));
    REQUIRE(Ceil(Quantity(-10), -1) == Quantity(-10));
    REQUIRE(Ceil(Quantity(10), 1) == Quantity(10));
    REQUIRE(Ceil(Quantity(-10), 1) == Quantity(-10));
    REQUIRE(Ceil(*Quantity::FromValue("-1.5"), 0) == Quantity(-1));
    REQUIRE(Ceil(*Quantity::FromValue("-1.5"), -1) == Quantity(0));
    REQUIRE(Ceil(*Quantity::FromValue("-0.5"), -1) == Quantity(0));
    REQUIRE(Ceil(*Quantity::FromValue("0.5"), -1) == Quantity(10));
    REQUIRE(Ceil(*Quantity::FromValue("0.5"), 0) == Quantity(1));
    REQUIRE(Ceil(*Quantity::FromValue("0.5"), 1) == Quantity::FromValue("0.5"));
    REQUIRE(Ceil(*Quantity::FromValue("0.5"), 2) == Quantity::FromValue("0.5"));
    REQUIRE(Ceil(Quantity(5), 0) == Quantity(5));
    REQUIRE(Ceil(Quantity(5), -1) == Quantity(10));
    REQUIRE(Ceil(Quantity(5), -2) == Quantity(100));
    REQUIRE(Ceil(Quantity(37), 0) == Quantity(37));
    REQUIRE(Ceil(Quantity(37), -1) == Quantity(40));
    REQUIRE(Ceil(Quantity(37), -2) == Quantity(100));
    REQUIRE(Ceil(Quantity(33), 0) == Quantity(33));
    REQUIRE(Ceil(Quantity(33), -1) == Quantity(40));
    REQUIRE(Ceil(Quantity(33), -2) == Quantity(100));
    REQUIRE(Ceil(Quantity(73), 0) == Quantity(73));
    REQUIRE(Ceil(Quantity(73), -1) == Quantity(80));
    REQUIRE(Ceil(Quantity(73), -2) == Quantity(100));
    REQUIRE(Ceil(*Quantity::FromValue("555.555"), -3) == Quantity(1000));
    REQUIRE(Ceil(*Quantity::FromValue("555.555"), -2) == Quantity(600));
    REQUIRE(Ceil(*Quantity::FromValue("555.555"), -1) == Quantity(560));
    REQUIRE(Ceil(*Quantity::FromValue("555.555"), 0) == Quantity(556));
    REQUIRE(Ceil(*Quantity::FromValue("555.555"), 1) ==
      Quantity::FromValue("555.6"));
    REQUIRE(Ceil(*Quantity::FromValue("555.555"), 2) ==
      Quantity::FromValue("555.56"));
    REQUIRE(Ceil(*Quantity::FromValue("555.555"), 3) ==
      Quantity::FromValue("555.555"));
    REQUIRE(Ceil(*Quantity::FromValue("555.555"), 4) ==
      Quantity::FromValue("555.555"));
    REQUIRE(Ceil(*Quantity::FromValue("-555.555"), 1) ==
      Quantity::FromValue("-555.5"));
  }

  TEST_CASE("round") {
    REQUIRE(Round(*Quantity::FromValue("0.5"), 0) == Quantity(1));
    REQUIRE(Round(Quantity(5), -2) == Quantity(0));
    REQUIRE(Round(Quantity(37), -2) == Quantity(0));
    REQUIRE(Round(Quantity(33), -2) == Quantity(0));
    REQUIRE(Round(Quantity(37), -3) == Quantity(0));
    REQUIRE(Round(Quantity(73), -2) == Quantity(100));
    REQUIRE(Round(Quantity(73), -2) == Quantity(100));
  }
}
