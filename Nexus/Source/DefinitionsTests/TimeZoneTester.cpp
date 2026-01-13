#include <doctest/doctest.h>
#include "Nexus/Definitions/Venue.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  auto GetTimeZoneDatabase() {
    auto ss = std::stringstream();
    ss <<
      "\"Australian_Eastern_Standard_Time\",\"AEST\",\"AEST\",\"AEST\",\"AEST\",\"+10:00:00\",\"+01:00:00\",\"1;0;10\",\"+02:00:00\",\"1;0;4\",\"+03:00:00\"\n"
      "\"Eastern_Time\",\"EST\",\"Eastern Standard Time\",\"EDT\",\"Eastern Daylight Time\",\"-05:00:00\",\"+01:00:00\",\"2;0;3\",\"+02:00:00\",\"1;0;11\",\"+02:00:00\"\n"
      "\"UTC\",\"UTC\",\"UTC\",\"\",\"\",\"+00:00:00\",\"+00:00:00\",\"\",\"\",\"\",\"+00:00:00\"\n";
    auto database = boost::local_time::tz_database();
    database.load_from_stream(ss);
    return database;
  }
}

TEST_SUITE("TimeZone") {
  TEST_CASE("tsx_start_of_day") {
    auto base_time = ptime(date(1984, 5, 7), time_duration(1, 30, 0, 0));
    auto converted_time = utc_start_of_day(
      DefaultVenues::TSX, base_time, DEFAULT_VENUES, GetTimeZoneDatabase());
    auto expected_time = ptime(date(1984, 5, 6), time_duration(4, 0, 0, 0));
    REQUIRE(converted_time == expected_time);
  }
}
