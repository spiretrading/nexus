#include <type_traits>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;

TEST_SUITE("SecurityMarketDataQuery") {
  TEST_CASE("make_security_info_query") {
    auto security = Security("ABC", NYSE);
    auto query = make_security_info_query(security);
    REQUIRE(query.GetIndex() == security);
    auto limit = query.GetSnapshotLimit();
    REQUIRE(limit.GetSize() == 1);
    REQUIRE(limit.GetType() == SnapshotLimit::Type::HEAD);
  }
}
