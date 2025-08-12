#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/Backtester/ActiveBacktesterEvent.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("ActiveBacktesterEvent") {
  TEST_CASE("constructor") {
    auto event = ActiveBacktesterEvent();
    REQUIRE(event.get_timestamp() == boost::posix_time::neg_infin);
    REQUIRE(!event.is_passive());
  }

  TEST_CASE("timestamp_constructor") {
    auto timestamp = time_from_string("2025-08-12 12:34:56.000");
    auto event = ActiveBacktesterEvent(timestamp);
    REQUIRE(event.get_timestamp() == timestamp);
    REQUIRE(!event.is_passive());
  }

  TEST_CASE("execute") {
    auto event = ActiveBacktesterEvent();
    event.execute();
  }
}
