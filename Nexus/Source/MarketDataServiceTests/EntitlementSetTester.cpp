#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/MarketDataService/EntitlementSet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::Venues;

TEST_SUITE("EntitlementSet") {
  TEST_CASE("format") {
    auto key = EntitlementKey(TSX, TSXV);
    REQUIRE(std::format("{}", key) == "(TSX, TSXV)");
    REQUIRE(std::format("{:>13}", key) == "  (TSX, TSXV)");
    auto venues = VenueDatabase();
    auto entry = VenueDatabase::Entry();
    entry.m_venue = TSX;
    entry.m_display_name = "CUSTOM";
    venues.add(entry);
    REQUIRE(std::format(make_locale(venues), "{}", EntitlementKey(TSX)) ==
      "(CUSTOM, CUSTOM)");
    auto output = std::ostringstream();
    output << venues << EntitlementKey(TSX);
    REQUIRE(output.str() == "(CUSTOM, CUSTOM)");
  }

  TEST_CASE("construct_from_venue") {
    auto key = EntitlementKey(ASX);
    REQUIRE(key.m_venue == ASX);
    REQUIRE(key.m_source == ASX);
  }

  TEST_CASE("stream_key") {
    auto key = EntitlementKey(OMGA, TSX);
    auto stream = std::stringstream();
    stream << key;
    REQUIRE(stream.str() == "(OMGA, TSX)");
  }

  TEST_CASE("shuttle_key") {
    auto key = EntitlementKey(OMGA, TSX);
    test_round_trip_shuttle(key);
  }

  TEST_CASE("grant_and_contains") {
    auto entitlements = EntitlementSet();
    REQUIRE(!entitlements.contains({TSX, TSX}, MarketDataType::BBO_QUOTE));
    entitlements.grant({TSX, TSX}, {MarketDataType::BBO_QUOTE});
    REQUIRE(entitlements.contains({TSX, TSX}, MarketDataType::BBO_QUOTE));
    REQUIRE(!entitlements.contains({TSX, TSX}, MarketDataType::BOOK_QUOTE));
    REQUIRE(!entitlements.contains({OMGA, TSX}, MarketDataType::BBO_QUOTE));
  }

  TEST_CASE("ticker_query") {
    auto entitlements = EntitlementSet();
    entitlements.grant({TSX, TSX}, {MarketDataType::BBO_QUOTE});
    auto query = TickerQuery();
    query.set_index(parse_ticker("TST.TSX"));
    REQUIRE(contains<BboQuote>(entitlements, query));
    REQUIRE(!contains<BookQuote>(entitlements, query));
  }

  TEST_CASE("venue_query") {
    auto entitlements = EntitlementSet();
    entitlements.grant({TSX, TSX}, {MarketDataType::ORDER_IMBALANCE});
    auto query = VenueQuery();
    query.set_index(TSX);
    REQUIRE(contains<OrderImbalance>(entitlements, query));
    REQUIRE(!contains<BboQuote>(entitlements, query));
  }
}
