#include <sstream>
#include <unordered_set>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Accounting/Position.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/Definitions/Security.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Accounting;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  using TestPosition = Position<Security>;
  using TestKey = TestPosition::Key;

  auto TST = Security("TST", TSX);
}

TEST_SUITE("Position") {
  TEST_CASE("key_hash") {
    auto key1 = TestKey(TST, CAD);
    auto key2 = TestKey(TST, USD);
    auto key_set = std::unordered_set<TestKey>();
    key_set.insert(key1);
    key_set.insert(key2);
    REQUIRE(key_set.size() == 2);
    REQUIRE(key_set.count(key1) == 1);
    REQUIRE(key_set.count(key2) == 1);
  }

  TEST_CASE("key_shuttle") {
    Beam::Serialization::Tests::TestRoundTripShuttle(TestKey(TST, CAD));
  }

  TEST_CASE("key_output_stream") {
    auto key = TestKey(TST, CAD);
    auto stream = std::stringstream();
    stream << key;
    REQUIRE(stream.str() == "(TST.TSX CAD)");
  }

  TEST_CASE("position_shuttle") {
    Beam::Serialization::Tests::TestRoundTripShuttle(
      TestPosition(TestKey(TST, CAD), 100, 1000 * Money::ONE));
  }

  TEST_CASE("position_output_stream") {
    auto key = TestKey(TST, CAD);
    auto position = TestPosition(key, 100, 1000 * Money::ONE);
    auto stream = std::stringstream();
    stream << position;
    REQUIRE(stream.str() == "((TST.TSX CAD) 100 1000.00)");
  }

  TEST_CASE("get_average_price") {
    auto key = TestKey(TST, CAD);
    auto position1 = TestPosition(key, 100, 1234 * Money::ONE);
    REQUIRE(get_average_price(position1) == 12.34 * Money::ONE);
    auto position2 = TestPosition(key, -100, -1234 * Money::ONE);
    REQUIRE(get_average_price(position2) == 12.34 * Money::ONE);
    auto position3 = TestPosition(key, 0, Money::ZERO);
    REQUIRE(get_average_price(position3) == Money::ZERO);
  }

  TEST_CASE("get_side") {
    auto key = TestKey(TST, CAD);
    auto position1 = TestPosition(key, 100, 1000 * Money::ONE);
    REQUIRE(get_side(position1) == Side::BID);
    auto position2 = TestPosition(key, -100, -1000 * Money::ONE);
    REQUIRE(get_side(position2) == Side::ASK);
    auto position3 = TestPosition(key, 0, Money::ZERO);
    REQUIRE(get_side(position3) == Side::NONE);
  }
}
