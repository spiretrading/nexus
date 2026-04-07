#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Accounting/Position.hpp"
#include "Nexus/Definitions/Security.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  auto TST = Security("TST", TSX);
}

TEST_SUITE("Position") {
  TEST_CASE("stream") {
    auto position = Position(TST, CAD, 100, 1000 * Money::ONE);
    test_round_trip_shuttle(Position(TST, CAD, 100, 1000 * Money::ONE));
    REQUIRE(to_string(position) == "(TST.TSX CAD 100 1000.00)");
  }

  TEST_CASE("get_average_price") {
    auto position1 = Position(TST, CAD, 100, 1234 * Money::ONE);
    REQUIRE(get_average_price(position1) == 12.34 * Money::ONE);
    auto position2 = Position(TST, CAD, -100, -1234 * Money::ONE);
    REQUIRE(get_average_price(position2) == 12.34 * Money::ONE);
    auto position3 = Position(TST, CAD, 0, Money::ZERO);
    REQUIRE(get_average_price(position3) == Money::ZERO);
  }

  TEST_CASE("get_side") {
    auto position1 = Position(TST, CAD, 100, 1000 * Money::ONE);
    REQUIRE(get_side(position1) == Side::BID);
    auto position2 = Position(TST, CAD, -100, -1000 * Money::ONE);
    REQUIRE(get_side(position2) == Side::ASK);
    auto position3 = Position(TST, CAD, 0, Money::ZERO);
    REQUIRE(get_side(position3) == Side::NONE);
  }
}
