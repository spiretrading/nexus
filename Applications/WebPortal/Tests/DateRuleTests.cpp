#include <limits>
#include <sstream>
#include <Beam/Json/JsonParser.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "WebPortal/DateRule.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("DateRule") {
  TEST_CASE("calendar_rules") {
    auto reference = time_from_string("2026-10-07 12:34:56.123456");
    using Boundary = MonthBoundaryDateRule::Boundary;
    auto cases = std::vector<std::pair<DateRule, const char*>>({
      {SpecificDateRule(date(2024, 2, 29)), "2024-02-29"},
      {DayOffsetDateRule(-1), "2026-10-06"},
      {DayOffsetDateRule(0), "2026-10-07"},
      {DayOffsetDateRule(2), "2026-10-09"},
      {WeekdayDateRule(-2, Monday), "2026-09-21"},
      {WeekdayDateRule(-1, Tuesday), "2026-09-29"},
      {WeekdayDateRule(0, Thursday), "2026-10-08"},
      {WeekdayDateRule(0, Sunday), "2026-10-11"},
      {WeekdayDateRule(1, Monday), "2026-10-12"},
      {DayOfMonthDateRule(-1, 15), "2026-09-15"},
      {DayOfMonthDateRule(-1, 31), "2026-09-30"},
      {DayOfMonthDateRule(3, 1), "2027-01-01"},
      {MonthBoundaryDateRule(0, Boundary::FIRST, 0), "2026-10-01"},
      {MonthBoundaryDateRule(0, Boundary::LAST, -1), "2026-10-30"},
      {MonthBoundaryDateRule(0, Boundary::FIRST, -1), "2026-09-30"},
      {MonthBoundaryDateRule(0, Boundary::LAST, 1), "2026-11-01"},
      {MonthBoundaryDateRule(-10, Boundary::FIRST, -1), "2025-11-30"}});
    for(auto& [rule, expected] : cases) {
      auto result = apply(rule, reference);
      REQUIRE(result.date() == from_simple_string(expected));
      REQUIRE(result.time_of_day() == reference.time_of_day());
    }
    auto sunday = time_from_string("2026-10-11 12:00:00");
    REQUIRE(apply(WeekdayDateRule(0, Monday), sunday) ==
      time_from_string("2026-10-05 12:00:00"));
    REQUIRE(apply(WeekdayDateRule(1, Monday), sunday) ==
      time_from_string("2026-10-12 12:00:00"));
    for(auto& [reference, expected] : std::vector({
        std::pair("2024-03-31 09:10:11", "2024-02-29 09:10:11"),
        std::pair("2025-03-31 09:10:11", "2025-02-28 09:10:11")})) {
      auto timestamp = time_from_string(reference);
      REQUIRE(apply(DayOfMonthDateRule(-1, 31), timestamp) ==
        time_from_string(expected));
      REQUIRE(apply(MonthBoundaryDateRule(-1, Boundary::LAST, 0), timestamp) ==
        time_from_string(expected));
    }
  }

  TEST_CASE("range_and_validation") {
    auto reference = time_from_string("2026-10-07 12:00:00");
    auto minimum = ptime(date(boost::date_time::min_date_time));
    auto maximum = ptime(date(boost::date_time::max_date_time));
    using Boundary = MonthBoundaryDateRule::Boundary;
    for(auto offset : {std::numeric_limits<std::int32_t>::min(),
        std::numeric_limits<std::int32_t>::max()}) {
      for(auto& rule : std::vector<DateRule>({DayOffsetDateRule(offset),
          WeekdayDateRule(offset, Monday), DayOfMonthDateRule(offset, 1),
          MonthBoundaryDateRule(offset, Boundary::FIRST, 0),
          MonthBoundaryDateRule(0, Boundary::FIRST, offset)})) {
        REQUIRE_THROWS_AS(apply(rule, reference), std::out_of_range);
      }
    }
    REQUIRE(apply(DayOffsetDateRule(0), minimum) == minimum);
    REQUIRE(apply(DayOffsetDateRule(0), maximum) == maximum);
    REQUIRE_THROWS_AS(apply(DayOffsetDateRule(-1), minimum), std::out_of_range);
    REQUIRE_THROWS_AS(apply(DayOffsetDateRule(1), maximum), std::out_of_range);
    REQUIRE_THROWS_AS(
      apply(DayOfMonthDateRule(-1, 1), minimum), std::out_of_range);
    REQUIRE_THROWS_AS(
      apply(DayOfMonthDateRule(1, 1), maximum), std::out_of_range);
    REQUIRE_THROWS_AS(apply(MonthBoundaryDateRule(0, Boundary::LAST, 1),
      maximum), std::out_of_range);
    for(auto& rule : std::vector<DateRule>({SpecificDateRule(),
        DayOfMonthDateRule(0, 0), DayOfMonthDateRule(0, 32),
        MonthBoundaryDateRule(0, static_cast<Boundary>(2), 0)})) {
      REQUIRE_THROWS_AS(apply(rule, reference), std::invalid_argument);
    }
    auto rules = std::vector<DateRule>({SpecificDateRule(date(2026, 10, 7)),
      DayOffsetDateRule(), WeekdayDateRule(), DayOfMonthDateRule(),
      MonthBoundaryDateRule()});
    for(auto& rule : rules) {
      for(auto& special : {ptime(), ptime(pos_infin), ptime(neg_infin)}) {
        REQUIRE_THROWS_AS(apply(rule, special), std::invalid_argument);
      }
    }
  }

  TEST_CASE("names") {
    auto types = std::vector({DateRuleType::SPECIFIC_DATE,
      DateRuleType::DAY_OFFSET, DateRuleType::WEEKDAY,
      DateRuleType::DAY_OF_MONTH, DateRuleType::MONTH_BOUNDARY});
    auto output = std::ostringstream();
    for(auto type : types) {
      REQUIRE(parse_date_rule_type(to_string(type)) == type);
      REQUIRE(to_json(type) == to_json(to_string(type)));
      REQUIRE(from_json<DateRuleType>(to_json(type)) == type);
      output << type << ' ';
    }
    REQUIRE(output.str() ==
      "SPECIFIC_DATE DAY_OFFSET WEEKDAY DAY_OF_MONTH MONTH_BOUNDARY ");
    REQUIRE_THROWS_AS(parse_date_rule_type("Unknown"), std::invalid_argument);
    REQUIRE_THROWS_AS(parse_date_rule_type("1"), std::invalid_argument);
    for(auto day : {Monday, Tuesday, Wednesday, Thursday, Friday, Saturday,
        Sunday}) {
      auto weekday = greg_weekday(day);
      auto name = boost::to_upper_copy(std::string(weekday.as_long_string()));
      REQUIRE(parse_weekday(name) == weekday);
    }
    REQUIRE_THROWS_AS(parse_weekday("Mon"), std::invalid_argument);
    REQUIRE_THROWS_AS(parse_weekday("7"), std::invalid_argument);
    using Boundary = MonthBoundaryDateRule::Boundary;
    REQUIRE(to_string(Boundary::FIRST) == "FIRST");
    REQUIRE(to_string(Boundary::LAST) == "LAST");
    for(auto boundary : {Boundary::FIRST, Boundary::LAST}) {
      REQUIRE(to_json(boundary) == to_json(to_string(boundary)));
      REQUIRE(from_json<Boundary>(to_json(boundary)) == boundary);
    }
    REQUIRE_THROWS_AS(from_json<DateRuleType>(std::string("4")),
      SerializationException);
    REQUIRE_THROWS_AS(from_json<Boundary>(std::string("1")),
      SerializationException);
    REQUIRE(parse_month_boundary("FIRST") == Boundary::FIRST);
    REQUIRE(parse_month_boundary("LAST") == Boundary::LAST);
    REQUIRE_THROWS_AS(parse_month_boundary("last"), std::invalid_argument);
  }

  TEST_CASE("serialization") {
    auto rules = std::vector<DateRule>({SpecificDateRule(date(2024, 2, 29)),
      DayOffsetDateRule(std::numeric_limits<std::int32_t>::min()),
      WeekdayDateRule(-2, Sunday), DayOfMonthDateRule(1, 31),
      MonthBoundaryDateRule(-1, MonthBoundaryDateRule::Boundary::LAST, -1),
      DayOffsetDateRule(std::numeric_limits<std::int32_t>::max())});
    auto expected = parse<JsonValue>(to_json(rules));
    test_round_trip_shuttle(rules, [&] (const auto& received) {
      REQUIRE(parse<JsonValue>(to_json(received)) == expected);
    });
    auto received = from_json<std::vector<DateRule>>(to_json(rules));
    REQUIRE(parse<JsonValue>(to_json(received)) == expected);
    auto month = get<JsonObject>(parse<JsonValue>(to_json(rules[4])));
    REQUIRE(month.at("type") == JsonValue("MONTH_BOUNDARY"));
    auto& value = get<JsonObject>(month.at("value"));
    REQUIRE(value.at("boundary") == JsonValue("LAST"));
    REQUIRE(value.at("offset") == -1);
    REQUIRE(value.at("day_offset") == -1);
    REQUIRE(!month.get("which"));
    auto weekday = get<JsonObject>(parse<JsonValue>(to_json(rules[2])));
    REQUIRE(get<JsonObject>(weekday.at("value")).at("day") ==
      JsonValue("SUNDAY"));
    auto reference = time_from_string("2026-10-07 12:00:00");
    for(auto i = std::size_t(2); i < 5; ++i) {
      REQUIRE(get_type(received[i]) == get_type(rules[i]));
      REQUIRE(apply(received[i], reference) == apply(rules[i], reference));
    }
    for(auto& invalid : {R"({"type":1,"value":{"offset":0}})",
        R"({"type":"Unknown","value":{}})",
        R"({"type":"DAY_OFFSET","value":{"offset":0.5}})",
        R"({"type":"DAY_OFFSET","value":{"offset":2147483648}})",
        R"({"type":"DAY_OFFSET","value":{"offset":-2147483649}})",
        R"({"type":"DAY_OFFSET","value":{"offset":null}})",
        R"({"type":"WEEKDAY","value":{"offset":0,"day":"Funday"}})",
        R"({"type":"DAY_OF_MONTH","value":{"offset":0,"day":32}})",
        R"({"type":"DAY_OF_MONTH","value":{"offset":0,"day":1.5}})",
        R"({"type":"MONTH_BOUNDARY","value":{"offset":0,
          "boundary":"LAST","day_offset":0.5}})",
        R"({"type":"MONTH_BOUNDARY","value":{"offset":0,
          "boundary":"Other","day_offset":0}})",
        R"({"type":"SPECIFIC_DATE","value":{"date":"not-a-date-time"}})",
        R"({"type":"SPECIFIC_DATE","value":{"date":"20260230"}})"}) {
      REQUIRE_THROWS_AS(
        from_json<DateRule>(std::string(invalid)), std::exception);
    }
  }
}
