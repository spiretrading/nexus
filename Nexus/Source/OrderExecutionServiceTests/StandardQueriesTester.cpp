#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::OrderExecutionService;

TEST_SUITE("StandardQueries") {
  TEST_CASE("make_daily_order_submission_query") {
    auto account = DirectoryEntry::MakeAccount(123, "test");
    auto start = time_from_string("2024-07-16 00:00:00");
    auto end = time_from_string("2024-07-16 00:00:00");
    auto query = make_daily_order_submission_query(TSX, account, start, end,
      DEFAULT_VENUES, get_default_time_zone_database());
    REQUIRE(query.GetIndex() == account);
    REQUIRE(query.GetRange().GetStart() <= query.GetRange().GetEnd());
    REQUIRE(query.GetSnapshotLimit() == SnapshotLimit::Unlimited());
  }
}
