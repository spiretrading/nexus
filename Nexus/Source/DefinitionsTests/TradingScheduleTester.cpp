#include <doctest/doctest.h>
#include "Nexus/Definitions/TradingSchedule.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("TradingSchedule") {
  TEST_CASE("construct") {
    auto events = std::vector<TradingSchedule::Event>();
    events.push_back({TradingSchedule::Type::OPEN, "O",
      ptime(date(1984, 5, 7), time_duration(1, 30, 0, 0))});
    events.push_back({TradingSchedule::Type::CLOSE, "C",
      ptime(date(1984, 5, 7), time_duration(4, 30, 0, 0))});
    auto schedule = TradingSchedule(events);
    REQUIRE(schedule.GetEvents() == events);
  }

  TEST_CASE("filter_type") {
    auto events = std::vector<TradingSchedule::Event>();
    events.push_back({TradingSchedule::Type::OPEN, "O",
      ptime(date(1984, 5, 7), time_duration(1, 30, 0, 0))});
    events.push_back({TradingSchedule::Type::CLOSE, "C",
      ptime(date(1984, 5, 7), time_duration(4, 30, 0, 0))});
    auto schedule = TradingSchedule(events);
    auto filter = Filter(schedule, TradingSchedule::Type::OPEN);
    REQUIRE(filter.GetEvents().size() == 1);
    REQUIRE(filter.GetEvents().front() == events.front());
  }
}
