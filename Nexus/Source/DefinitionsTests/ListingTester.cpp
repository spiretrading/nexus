#include <functional>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Listing.hpp"

using namespace Beam::Tests;
using namespace Nexus;

TEST_SUITE("Listing") {
  TEST_CASE("equality") {
    auto listing1 = Listing(101, Instrument(
      Asset(Asset::EQY, 1), Asset(Asset::CCY, 2), Instrument::Type::SPOT));
    auto listing2 = Listing(101, Instrument(
      Asset(Asset::EQY, 3), Asset(Asset::CCY, 4), Instrument::Type::FORWARD));
    auto listing3 = Listing(102, listing1.m_instrument);
    REQUIRE(listing1 == listing2);
    REQUIRE(listing1 != listing3);
  }

  TEST_CASE("hash") {
    auto listing = Listing(44, Instrument(
      Asset(Asset::EQY, 10), Asset(Asset::CCY, 11), Instrument::Type::SPOT));
    auto hash = std::hash<Listing>()(listing);
    auto expected = std::hash<std::uint64_t>()(listing.m_id);
    REQUIRE(hash == expected);
  }

  TEST_CASE("shuttle") {
    auto listing = Listing(77, Instrument(
      Asset(Asset::EQY, 5), Asset(Asset::CCY, 6), Instrument::Type::PERPETUAL));
    test_round_trip_shuttle(listing);
  }
}
