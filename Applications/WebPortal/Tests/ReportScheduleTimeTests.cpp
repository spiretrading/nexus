#include <limits>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <doctest/doctest.h>
#include "WebPortal/ReportSchedule.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  ReportSchedule make_schedule(
      const std::string& start, ReportSchedule::Interval interval) {
    return ReportSchedule("schedule", {}, {}, {}, {}, {},
      time_from_string(start), {}, interval, "America/Toronto");
  }
}

TEST_SUITE("ReportScheduleTime") {
  TEST_CASE("browser_timezones") {
    REQUIRE(convert_report_time(time_from_string("2026-07-01 12:00:00"),
      "America/Toronto", "UTC") == time_from_string("2026-07-01 16:00:00"));
    REQUIRE(convert_report_time(time_from_string("2026-01-01 12:00:00"),
      "America/Toronto", "UTC") == time_from_string("2026-01-01 17:00:00"));
    REQUIRE(convert_report_time(time_from_string("2026-07-01 16:00:00"), "UTC",
      "America/Toronto") == time_from_string("2026-07-01 12:00:00"));
    REQUIRE_THROWS_AS(convert_report_time(
      time_from_string("2026-03-08 02:30:00"), "America/Toronto", "UTC"),
      std::invalid_argument);
    for(auto& zone : {"", "Not/AZone"}) {
      REQUIRE_THROWS_AS(convert_report_time(
        time_from_string("2026-07-01 12:00:00"), zone, "UTC"),
        std::invalid_argument);
    }
  }

  TEST_CASE("calendar_anchor_and_future_occurrences") {
    auto schedule = make_schedule("2026-01-31 10:00:00",
      ReportSchedule::Interval(1, ReportSchedule::Interval::Unit::MONTH));
    REQUIRE(next_report_run(
      schedule, time_from_string("2026-02-01 00:00:00")) ==
      time_from_string("2026-02-28 10:00:00"));
    REQUIRE(next_report_run(
      schedule, time_from_string("2026-02-28 16:00:00")) ==
      time_from_string("2026-03-31 10:00:00"));
    schedule.m_start_time = time_from_string("2026-02-28 10:00:00");
    REQUIRE(next_report_run(
      schedule, time_from_string("2026-03-01 00:00:00")) ==
      time_from_string("2026-03-28 10:00:00"));
    schedule = make_schedule("2024-02-29 10:00:00",
      ReportSchedule::Interval(1, ReportSchedule::Interval::Unit::YEAR));
    REQUIRE(next_report_run(
      schedule, time_from_string("2025-03-01 00:00:00")) ==
      time_from_string("2026-02-28 10:00:00"));
    REQUIRE(next_report_run(
      schedule, time_from_string("2027-03-01 00:00:00")) ==
      time_from_string("2028-02-29 10:00:00"));
    schedule = make_schedule("2026-10-01 10:00:00",
      ReportSchedule::Interval(2, ReportSchedule::Interval::Unit::WEEK));
    REQUIRE(next_report_run(
      schedule, time_from_string("2026-10-06 00:00:00")) ==
      time_from_string("2026-10-15 10:00:00"));
    schedule.m_repeat_interval.reset();
    REQUIRE(next_report_run(
      schedule, time_from_string("2026-10-06 00:00:00")) ==
      schedule.m_start_time);
  }

  TEST_CASE("daylight_saving_transitions") {
    auto schedule = make_schedule("2026-03-07 02:30:00",
      ReportSchedule::Interval(1, ReportSchedule::Interval::Unit::DAY));
    REQUIRE(next_report_run(
      schedule, time_from_string("2026-03-07 08:00:00")) ==
      time_from_string("2026-03-09 02:30:00"));
    schedule.m_start_time = time_from_string("2026-10-31 01:30:00");
    REQUIRE(next_report_run(
      schedule, time_from_string("2026-11-01 05:45:00")) ==
      time_from_string("2026-11-02 01:30:00"));
    REQUIRE(convert_report_time(time_from_string("2026-11-01 01:30:00"),
      "America/Toronto", "UTC") == time_from_string("2026-11-01 05:30:00"));
  }

  TEST_CASE("required_timezone_and_date_bounds") {
    auto schedule = make_schedule("2026-10-07 10:00:00",
      ReportSchedule::Interval(1, ReportSchedule::Interval::Unit::DAY));
    auto serialized = to_json(schedule);
    auto start = serialized.find(",\"time_zone\":");
    REQUIRE(start != std::string::npos);
    serialized.erase(start, serialized.size() - start - 1);
    REQUIRE_THROWS_AS(from_json<ReportSchedule>(serialized), std::out_of_range);
    schedule.m_repeat_interval->m_count = 0;
    REQUIRE_THROWS_AS(next_report_run(schedule,
      time_from_string("2026-10-06 00:00:00")), std::invalid_argument);
    schedule.m_repeat_interval->m_count = 1;
    schedule.m_time_zone = "UTC";
    schedule.m_start_time = time_from_string("9999-12-31 12:00:00");
    REQUIRE_THROWS_AS(next_report_run(schedule,
      time_from_string("9999-12-31 13:00:00")),
      ReportScheduleExhaustedException);
    schedule.m_start_time = time_from_string("2026-10-07 10:00:00");
    for(auto unit : {ReportSchedule::Interval::Unit::DAY,
        ReportSchedule::Interval::Unit::WEEK,
        ReportSchedule::Interval::Unit::MONTH,
        ReportSchedule::Interval::Unit::YEAR}) {
      schedule.m_repeat_interval = ReportSchedule::Interval(
        std::numeric_limits<std::uint32_t>::max(), unit);
      REQUIRE(next_report_run(schedule,
        time_from_string("2026-10-06 00:00:00")) == schedule.m_start_time);
      REQUIRE_THROWS_AS(next_report_run(schedule, schedule.m_start_time),
        ReportScheduleExhaustedException);
    }
  }
}
