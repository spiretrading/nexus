#include <doctest/doctest.h>
#include "Nexus/FixUtilities/FixConversions.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::FixUtilities;

TEST_SUITE("FixConversions") {
  TEST_CASE("timestamp_conversions") {
    auto timestamp = ptime(date(2015, 3, 12), time_duration(13, 33, 26) +
      milliseconds(361));
    auto fix_timestamp = GetTimestamp(timestamp);
    REQUIRE(fix_timestamp.getYear() == 2015);
    REQUIRE(fix_timestamp.getMonth() == 3);
    REQUIRE(fix_timestamp.getDay() == 12);
    REQUIRE(fix_timestamp.getHour() == 13);
    REQUIRE(fix_timestamp.getMinute() == 33);
    REQUIRE(fix_timestamp.getSecond() == 26);
    REQUIRE(fix_timestamp.getMillisecond() == 361);
  }
}
