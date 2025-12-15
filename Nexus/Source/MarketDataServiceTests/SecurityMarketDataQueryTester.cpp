#include <doctest/doctest.h>
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("SecurityMarketDataQuery") {
  TEST_CASE("make_security_info_query") {
    auto security = Security("ABC", TSX);
    auto query = make_security_info_query(security);
    REQUIRE(query.get_index() == security);
    auto limit = query.get_snapshot_limit();
    REQUIRE(limit.get_size() == 1);
    REQUIRE(limit.get_type() == SnapshotLimit::Type::HEAD);
  }
}
