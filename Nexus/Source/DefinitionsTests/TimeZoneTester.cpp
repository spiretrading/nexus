#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/Definitions/Venue.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("TimeZone") {
  TEST_CASE("tsx_start_of_day") {
    auto base_time = time_from_string("1984-05-07 01:30:00");
    auto converted_time = utc_start_of_day(DefaultVenues::TSX, base_time,
      DEFAULT_VENUES, get_default_time_zone_database());
    auto expected_time = time_from_string("1984-05-06 04:00:00");
    REQUIRE(converted_time == expected_time);
  }
}
