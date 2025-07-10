#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/MarketDataService/EntitlementSet.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;

TEST_SUITE("EntitlementSet") {
  TEST_CASE("construct_from_venue") {
    auto key = EntitlementKey(NYSE);
    REQUIRE(key.m_venue == NYSE);
    REQUIRE(key.m_source == NYSE);
  }

  TEST_CASE("stream_key") {
    auto key = EntitlementKey(OMGA, TSX);
    auto stream = std::stringstream();
    stream << key;
    REQUIRE(stream.str() == "(OMGA, TSX)");
  }

  TEST_CASE("shuttle_key") {
    auto key = EntitlementKey(OMGA, TSX);
    Beam::Serialization::Tests::TestRoundTripShuttle(key);
  }

  TEST_CASE("grant_and_contains") {
    auto entitlements = EntitlementSet();
    REQUIRE(!entitlements.contains({TSX, TSX}, MarketDataType::BBO_QUOTE));
    entitlements.grant({TSX, TSX}, {MarketDataType::BBO_QUOTE});
    REQUIRE(entitlements.contains({TSX, TSX}, MarketDataType::BBO_QUOTE));
    REQUIRE(!entitlements.contains({TSX, TSX}, MarketDataType::BOOK_QUOTE));
    REQUIRE(!entitlements.contains({OMGA, TSX}, MarketDataType::BBO_QUOTE));
  }

  TEST_CASE("security_query") {
    auto entitlements = EntitlementSet();
    entitlements.grant({TSX, TSX}, {MarketDataType::BBO_QUOTE});
    auto query = SecurityMarketDataQuery();
    query.SetIndex(Security("TST", TSX));
    REQUIRE(contains<BboQuote>(entitlements, query));
    REQUIRE(!contains<BookQuote>(entitlements, query));
  }

  TEST_CASE("venue_query") {
    auto entitlements = EntitlementSet();
    entitlements.grant({TSX, TSX}, {MarketDataType::ORDER_IMBALANCE});
    auto query = VenueMarketDataQuery();
    query.SetIndex(TSX);
    REQUIRE(contains<OrderImbalance>(entitlements, query));
    REQUIRE(!contains<BboQuote>(entitlements, query));
  }
}
