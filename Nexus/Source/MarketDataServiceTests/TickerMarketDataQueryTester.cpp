#include <doctest/doctest.h>
#include "Nexus/MarketDataService/TickerMarketDataQuery.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("TickerMarketDataQuery") {
  TEST_CASE("make_ticker_info_query") {
    auto ticker = parse_ticker("ABC.TSX");
    auto query = make_ticker_info_query(ticker);
    REQUIRE(query.get_index() == ticker);
    auto limit = query.get_snapshot_limit();
    REQUIRE(limit.get_size() == 1);
    REQUIRE(limit.get_type() == SnapshotLimit::Type::HEAD);
  }
}
