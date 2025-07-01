#include <Beam/SerializationTests/ValueShuttleTests.hpp>
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
    REQUIRE(parse_quantity("1") == Quantity(1));
    REQUIRE(parse_quantity("1.1") == Quantity(1.1));
    REQUIRE(try_parse_quantity("1a.1") == none);
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
    REQUIRE(floor(Quantity(1), 0) == Quantity(1));
    REQUIRE(floor(Quantity(-1), 0) == Quantity(-1));
    REQUIRE(floor(Quantity(10), -1) == Quantity(10));
    REQUIRE(floor(Quantity(-10), -1) == Quantity(-10));
    REQUIRE(floor(Quantity(10), 1) == Quantity(10));
    REQUIRE(floor(Quantity(-10), 1) == Quantity(-10));
    REQUIRE(floor(parse_quantity("-1.5"), 0) == Quantity(-2));
    REQUIRE(floor(parse_quantity("-1.5"), -1) == Quantity(-10));
    REQUIRE(floor(parse_quantity("-0.5"), -1) == Quantity(-10));
    REQUIRE(floor(parse_quantity("0.5"), -1) == Quantity(0));
    REQUIRE(floor(parse_quantity("0.5"), 0) == Quantity(0));
    REQUIRE(floor(parse_quantity("0.5"), 1) == parse_quantity("0.5"));
    REQUIRE(floor(parse_quantity("0.5"), 2) == parse_quantity("0.5"));
    REQUIRE(floor(Quantity(5), 0) == Quantity(5));
    REQUIRE(floor(Quantity(5), -1) == Quantity(0));
    REQUIRE(floor(Quantity(5), -2) == Quantity(0));
    REQUIRE(floor(Quantity(37), 0) == Quantity(37));
    REQUIRE(floor(Quantity(37), -1) == Quantity(30));
    REQUIRE(floor(Quantity(37), -2) == Quantity(0));
    REQUIRE(floor(Quantity(33), 0) == Quantity(33));
    REQUIRE(floor(Quantity(33), -1) == Quantity(30));
    REQUIRE(floor(Quantity(33), -2) == Quantity(0));
    REQUIRE(floor(Quantity(73), 0) == Quantity(73));
    REQUIRE(floor(Quantity(73), -1) == Quantity(70));
    REQUIRE(floor(Quantity(73), -2) == Quantity(0));
    REQUIRE(floor(parse_quantity("555.555"), -3) == Quantity(0));
    REQUIRE(floor(parse_quantity("555.555"), -2) == Quantity(500));
    REQUIRE(floor(parse_quantity("555.555"), -1) == Quantity(550));
    REQUIRE(floor(parse_quantity("555.555"), 0) == Quantity(555));
    REQUIRE(floor(parse_quantity("555.555"), 1) == parse_quantity("555.5"));
    REQUIRE(floor(parse_quantity("555.555"), 2) == parse_quantity("555.55"));
    REQUIRE(floor(parse_quantity("555.555"), 3) == parse_quantity("555.555"));
    REQUIRE(floor(parse_quantity("555.555"), 4) == parse_quantity("555.555"));
    REQUIRE(floor(parse_quantity("-555.555"), 1) == parse_quantity("-555.6"));
  }

  TEST_CASE("ceil") {
    REQUIRE(ceil(Quantity(1), 0) == Quantity(1));
    REQUIRE(ceil(Quantity(-1), 0) == Quantity(-1));
    REQUIRE(ceil(Quantity(10), -1) == Quantity(10));
    REQUIRE(ceil(Quantity(-10), -1) == Quantity(-10));
    REQUIRE(ceil(Quantity(10), 1) == Quantity(10));
    REQUIRE(ceil(Quantity(-10), 1) == Quantity(-10));
    REQUIRE(ceil(parse_quantity("-1.5"), 0) == Quantity(-1));
    REQUIRE(ceil(parse_quantity("-1.5"), -1) == Quantity(0));
    REQUIRE(ceil(parse_quantity("-0.5"), -1) == Quantity(0));
    REQUIRE(ceil(parse_quantity("0.5"), -1) == Quantity(10));
    REQUIRE(ceil(parse_quantity("0.5"), 0) == Quantity(1));
    REQUIRE(ceil(parse_quantity("0.5"), 1) == parse_quantity("0.5"));
    REQUIRE(ceil(parse_quantity("0.5"), 2) == parse_quantity("0.5"));
    REQUIRE(ceil(Quantity(5), 0) == Quantity(5));
    REQUIRE(ceil(Quantity(5), -1) == Quantity(10));
    REQUIRE(ceil(Quantity(5), -2) == Quantity(100));
    REQUIRE(ceil(Quantity(37), 0) == Quantity(37));
    REQUIRE(ceil(Quantity(37), -1) == Quantity(40));
    REQUIRE(ceil(Quantity(37), -2) == Quantity(100));
    REQUIRE(ceil(Quantity(33), 0) == Quantity(33));
    REQUIRE(ceil(Quantity(33), -1) == Quantity(40));
    REQUIRE(ceil(Quantity(33), -2) == Quantity(100));
    REQUIRE(ceil(Quantity(73), 0) == Quantity(73));
    REQUIRE(ceil(Quantity(73), -1) == Quantity(80));
    REQUIRE(ceil(Quantity(73), -2) == Quantity(100));
    REQUIRE(ceil(parse_quantity("555.555"), -3) == Quantity(1000));
    REQUIRE(ceil(parse_quantity("555.555"), -2) == Quantity(600));
    REQUIRE(ceil(parse_quantity("555.555"), -1) == Quantity(560));
    REQUIRE(ceil(parse_quantity("555.555"), 0) == Quantity(556));
    REQUIRE(ceil(parse_quantity("555.555"), 1) == parse_quantity("555.6"));
    REQUIRE(ceil(parse_quantity("555.555"), 2) == parse_quantity("555.56"));
    REQUIRE(ceil(parse_quantity("555.555"), 3) == parse_quantity("555.555"));
    REQUIRE(ceil(parse_quantity("555.555"), 4) == parse_quantity("555.555"));
    REQUIRE(ceil(parse_quantity("-555.555"), 1) == parse_quantity("-555.5"));
  }

  TEST_CASE("round") {
    REQUIRE(round(parse_quantity("0.5"), 0) == Quantity(1));
    REQUIRE(round(Quantity(5), -2) == Quantity(0));
    REQUIRE(round(Quantity(37), -2) == Quantity(0));
    REQUIRE(round(Quantity(33), -2) == Quantity(0));
    REQUIRE(round(Quantity(37), -3) == Quantity(0));
    REQUIRE(round(Quantity(73), -2) == Quantity(100));
    REQUIRE(round(Quantity(73), -2) == Quantity(100));
  }

  TEST_CASE("shuttle") {
    Beam::Serialization::Tests::TestRoundTripShuttle(Quantity(567));
  }
}
