#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/TradingSchedule.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Venues;

TEST_SUITE("TradingSchedule") {
  TEST_CASE("is_match_empty") {
    REQUIRE(is_match(Venue(), date(2025, 6, 15), TradingSchedule::Rule()));
  }

  TEST_CASE("match_all_dates") {
    auto rule = TradingSchedule::Rule{{TSX}, {}, {}, {}, {}, {}};
    REQUIRE(is_match(TSX, date(1984, 5, 7), rule));
    REQUIRE(!is_match(ASX, date(1984, 5, 7), rule));
  }

  TEST_CASE("match_day_of_week") {
    auto rule = TradingSchedule::Rule{
      {ASX}, {greg_weekday::weekday_enum::Monday}, {}, {}, {}, {}};
    REQUIRE(is_match(ASX, date(2020, 7, 20), rule));
    REQUIRE(!is_match(ASX, date(2020, 7, 21), rule));
    REQUIRE(!is_match(ASX, date(2020, 7, 22), rule));
    REQUIRE(!is_match(ASX, date(2020, 7, 23), rule));
    REQUIRE(!is_match(ASX, date(2020, 7, 24), rule));
    REQUIRE(!is_match(ASX, date(2020, 7, 25), rule));
    REQUIRE(!is_match(ASX, date(2020, 7, 26), rule));
    REQUIRE(is_match(ASX, date(2020, 7, 27), rule));
    REQUIRE(!is_match(TSX, date(2020, 7, 27), rule));
  }

  TEST_CASE("match_day") {
    auto rule = TradingSchedule::Rule{{TSX}, {}, {12}, {}, {}, {}};
    REQUIRE(is_match(TSX, date(2020, 7, 12), rule));
    REQUIRE(is_match(TSX, date(2020, 8, 12), rule));
    REQUIRE(is_match(TSX, date(2020, 3, 12), rule));
    REQUIRE(is_match(TSX, date(2015, 1, 12), rule));
    REQUIRE(is_match(TSX, date(2020, 7, 12), rule));
    REQUIRE(!is_match(TSX, date(2020, 7, 13), rule));
    REQUIRE(!is_match(TSX, date(2020, 8, 11), rule));
    REQUIRE(!is_match(TSX, date(2015, 7, 19), rule));
    REQUIRE(!is_match(TSX, date(1995, 7, 24), rule));
    REQUIRE(!is_match(TSX, date(2020, 5, 25), rule));
    REQUIRE(!is_match(TSX, date(2020, 2, 26), rule));
    REQUIRE(!is_match(ASX, date(2020, 7, 12), rule));
  }

  TEST_CASE("match_month") {
    auto rule = TradingSchedule::Rule{
      {TSX}, {}, {}, {greg_month::month_enum::Feb}, {}, {}};
    REQUIRE(is_match(TSX, date(2020, 2, 1), rule));
    REQUIRE(is_match(TSX, date(2020, 2, 5), rule));
    REQUIRE(is_match(TSX, date(2020, 2, 7), rule));
    REQUIRE(is_match(TSX, date(2015, 2, 12), rule));
    REQUIRE(is_match(TSX, date(1993, 2, 19), rule));
    REQUIRE(!is_match(TSX, date(2020, 7, 13), rule));
    REQUIRE(!is_match(TSX, date(2020, 8, 11), rule));
    REQUIRE(!is_match(TSX, date(2015, 7, 19), rule));
    REQUIRE(!is_match(TSX, date(1995, 7, 24), rule));
    REQUIRE(!is_match(TSX, date(2020, 5, 25), rule));
    REQUIRE(!is_match(TSX, date(2020, 6, 26), rule));
    REQUIRE(!is_match(ASX, date(2020, 7, 12), rule));
  }

  TEST_CASE("match_year") {
    auto rule = TradingSchedule::Rule{{TSX}, {}, {}, {}, {2002}, {}};
    REQUIRE(is_match(TSX, date(2002, 2, 1), rule));
    REQUIRE(is_match(TSX, date(2002, 5, 5), rule));
    REQUIRE(is_match(TSX, date(2002, 1, 7), rule));
    REQUIRE(is_match(TSX, date(2002, 8, 12), rule));
    REQUIRE(is_match(TSX, date(2002, 3, 19), rule));
    REQUIRE(!is_match(TSX, date(2020, 7, 13), rule));
    REQUIRE(!is_match(TSX, date(2020, 8, 11), rule));
    REQUIRE(!is_match(TSX, date(2015, 7, 19), rule));
    REQUIRE(!is_match(TSX, date(1995, 7, 24), rule));
    REQUIRE(!is_match(TSX, date(2020, 5, 25), rule));
    REQUIRE(!is_match(TSX, date(2020, 6, 26), rule));
    REQUIRE(!is_match(ASX, date(2002, 7, 12), rule));
  }

  TEST_CASE("match_date") {
    auto rule = TradingSchedule::Rule{
      {ASX}, {}, {6}, {greg_month::month_enum::May}, {1990}, {}};
    REQUIRE(is_match(ASX, date(1990, 5, 6), rule));
    REQUIRE(!is_match(TSX, date(1990, 5, 6), rule));
    REQUIRE(!is_match(TSX, date(2020, 7, 13), rule));
    REQUIRE(!is_match(TSX, date(2020, 8, 11), rule));
    REQUIRE(!is_match(TSX, date(2015, 7, 19), rule));
    REQUIRE(!is_match(TSX, date(1995, 7, 24), rule));
    REQUIRE(!is_match(TSX, date(2020, 5, 25), rule));
    REQUIRE(!is_match(TSX, date(2020, 6, 26), rule));
    REQUIRE(!is_match(ASX, date(2002, 7, 12), rule));
  }

  TEST_CASE("match_weekends_2020") {
    auto rule = TradingSchedule::Rule{{ASX},
      {greg_weekday::weekday_enum::Saturday,
      greg_weekday::weekday_enum::Sunday}, {}, {}, {2020}, {}};
    REQUIRE(is_match(ASX, date(2020, 7, 4), rule));
    REQUIRE(is_match(ASX, date(2020, 7, 5), rule));
    REQUIRE(!is_match(ASX, date(2020, 7, 3), rule));
    REQUIRE(!is_match(ASX, date(2020, 7, 7), rule));
    REQUIRE(is_match(ASX, date(2020, 9, 26), rule));
    REQUIRE(is_match(ASX, date(2020, 9, 27), rule));
    REQUIRE(!is_match(ASX, date(2020, 9, 23), rule));
    REQUIRE(!is_match(ASX, date(2020, 9, 24), rule));
  }

  TEST_CASE("find_type") {
    auto events = std::vector<TradingSchedule::Event>();
    events.push_back({"O", time_from_string("1984-05-07 01:30:00")});
    events.push_back({"C", time_from_string("1984-05-07 04:30:00")});
    auto rules = std::vector<TradingSchedule::Rule>();
    rules.push_back(TradingSchedule::Rule{{TSXV}, {}, {}, {}, {}, events});
    auto schedule = TradingSchedule(rules);
    auto found_events = schedule.find(
      time_from_string("1984-05-07 12:00:00"), TSXV,
      [] (const auto& event) {
        return event.m_code == "O";
      });
    REQUIRE(found_events.size() == 1);
    REQUIRE(found_events.front() == TradingSchedule::Event(
      "O", time_from_string("1984-05-07 05:30:00")));
  }

  TEST_CASE("rule_precedence") {
    auto opening = TradingSchedule::Event(
      "OPEN", time_from_string("2026-09-21 09:30:00"));
    auto closing = TradingSchedule::Event(
      "CLOSE", time_from_string("2026-09-21 16:00:00"));
    auto exception = TradingSchedule::Rule();
    exception.m_venues = {TSX};
    exception.m_days = {21};
    exception.m_months = {9};
    exception.m_years = {2026};
    SUBCASE("closure") {}
    SUBCASE("special_hours") {
      closing.m_timestamp = time_from_string("2026-09-21 13:00:00");
      exception.m_events = {opening, closing};
    }
    SUBCASE("filtered_exception") {
      exception.m_events = {closing};
    }
    auto regular = TradingSchedule::Rule();
    regular.m_venues = {TSX};
    regular.m_events = {opening, TradingSchedule::Event(
      "CLOSE", time_from_string("2026-09-21 16:00:00"))};
    auto schedule = TradingSchedule({exception, regular});
    auto expected = exception.m_events;
    for(auto& event : expected) {
      event.m_timestamp = venue_to_utc(TSX, event.m_timestamp);
    }
    REQUIRE(schedule.find(
      time_from_string("2026-09-21 12:00:00"), TSX) == expected);
    auto openings = schedule.find(time_from_string("2026-09-21 12:00:00"), TSX,
      [] (const auto& event) { return event.m_code == "OPEN"; });
    if(exception.m_events.size() == 2) {
      REQUIRE(openings == std::vector<TradingSchedule::Event>({
        TradingSchedule::Event(
          "OPEN", time_from_string("2026-09-21 13:30:00"))}));
    } else {
      REQUIRE(openings.empty());
    }
    REQUIRE(schedule.find(
      time_from_string("2026-09-22 12:00:00"), TSX).size() == 2);
    REQUIRE(schedule.find(
      time_from_string("2026-09-21 12:00:00"), ASX).empty());
  }

  TEST_CASE("find_timestamp") {
    auto schedule = parse_trading_schedule(YAML::Load(R"(
- venues: [TSX]
  events:
    - code: OPEN
      time: 09:30:00
    - code: CLOSE
      time: 16:00:00
)"));
    auto timestamp = time_from_string("2026-09-21 16:00:00");
    auto events = schedule.find(timestamp, TSX,
      [&] (const auto& event) { return event.m_timestamp <= timestamp; });
    REQUIRE(events.size() == 1);
    REQUIRE(events.front() == TradingSchedule::Event(
      "OPEN", time_from_string("2026-09-21 13:30:00")));
  }

  TEST_CASE("utc_trading_date") {
    auto schedule = parse_trading_schedule(YAML::Load(R"(
- venues: [TSX, ASX]
  time:
    weekdays: [Sat, Sun]
- venues: [TSX]
  dates: [2026-09-07]
- venues: [ASX]
  dates: [2026-01-26]
- venues: [TSX]
  events:
    - code: OPEN
      time: 09:30:00
- venues: [ASX]
  events:
    - code: OPEN
      time: 10:00:00
)"));
    auto venue = TSX;
    auto timestamp = time_from_string("2026-09-22 03:59:59");
    auto expected = std::vector<TradingSchedule::Event>({
      TradingSchedule::Event(
        "OPEN", time_from_string("2026-09-21 13:30:00"))});
    SUBCASE("previous_date") {}
    SUBCASE("midnight") {
      timestamp = time_from_string("2026-09-22 04:00:00");
      expected.front().m_timestamp = time_from_string("2026-09-22 13:30:00");
    }
    SUBCASE("weekend") {
      timestamp = time_from_string("2026-09-21 03:59:59");
      expected.clear();
    }
    SUBCASE("canadian_holiday") {
      timestamp = time_from_string("2026-09-08 03:59:59");
      expected.clear();
    }
    SUBCASE("next_date") {
      venue = ASX;
      timestamp = time_from_string("2026-09-20 14:00:00");
      expected.front().m_timestamp = time_from_string("2026-09-21 00:00:00");
    }
    SUBCASE("australian_holiday") {
      venue = ASX;
      timestamp = time_from_string("2026-01-25 13:00:00");
      expected.clear();
    }
    SUBCASE("australian_daylight_saving") {
      venue = ASX;
      timestamp = time_from_string("2026-01-04 13:00:00");
      expected.front().m_timestamp = time_from_string("2026-01-04 23:00:00");
    }
    REQUIRE(schedule.find(timestamp, venue) == expected);
    auto events = schedule.find(timestamp, venue,
      [&] (const auto& event) { return event.m_timestamp <= timestamp; });
    std::erase_if(expected, [&] (const auto& event) {
      return event.m_timestamp > timestamp;
    });
    REQUIRE(events == expected);
  }

  TEST_CASE("utc_events") {
    auto venue = TSX;
    auto day = date(2026, 7, 6);
    auto opening = time_from_string("1900-01-01 09:30:00");
    auto expected = time_from_string("2026-07-06 13:30:00");
    SUBCASE("summer") {}
    SUBCASE("winter") {
      day = date(2026, 1, 5);
      expected = time_from_string("2026-01-05 14:30:00");
    }
    SUBCASE("daylight_saving_start") {
      day = date(2026, 3, 9);
      expected = time_from_string("2026-03-09 13:30:00");
    }
    SUBCASE("daylight_saving_end") {
      day = date(2026, 11, 2);
      expected = time_from_string("2026-11-02 14:30:00");
    }
    SUBCASE("previous_utc_date") {
      venue = ASX;
      day = date(2026, 1, 5);
      opening = time_from_string("1900-01-01 10:00:00");
      expected = time_from_string("2026-01-04 23:00:00");
    }
    auto rule = TradingSchedule::Rule();
    rule.m_venues = {venue};
    rule.m_days = {day.day()};
    rule.m_months = {day.month()};
    rule.m_years = {day.year()};
    rule.m_events = {TradingSchedule::Event("OPEN", opening)};
    auto schedule = TradingSchedule({rule});
    auto events = schedule.find(ptime(day, hours(12)), venue);
    REQUIRE(events == std::vector<TradingSchedule::Event>({
      TradingSchedule::Event("OPEN", expected)}));
    REQUIRE(schedule.find(ptime(day, hours(12)), venue,
      [&] (const auto& event) {
        return event.m_timestamp < expected;
      }).empty());
    REQUIRE(schedule.find(ptime(day, hours(12)), venue,
      [&] (const auto& event) {
        return event.m_timestamp <= expected;
      }) == events);
  }

  TEST_CASE("parse_schedule") {
    auto schedule = parse_trading_schedule(YAML::Load(R"(
- venues: [TSXV]
  time:
    weekdays: [Sat, Sun]
  events:
    - code: CLOSE
      time: 16:00:00
    - code: OPEN
      time: 09:30:00
)"));
    auto empty_events = schedule.find(
      time_from_string("2020-01-15 12:00:00"), TSXV);
    REQUIRE(empty_events.empty());
    auto events = schedule.find(time_from_string("2020-07-18 12:00:00"), TSXV);
    REQUIRE(events == std::vector<TradingSchedule::Event>({
      TradingSchedule::Event(
        "OPEN", time_from_string("2020-07-18 13:30:00")),
      TradingSchedule::Event(
        "CLOSE", time_from_string("2020-07-18 20:00:00"))}));
    REQUIRE(schedule.find(
      time_from_string("2020-07-18 12:00:00"), ASX).empty());
  }

  TEST_CASE("parse_dates") {
    auto schedule = parse_trading_schedule(YAML::Load(R"(
- venues: [TSX, TSXV]
  dates: [2026-07-01, 2026-12-25]
- venues: [TSX, TSXV]
  time:
    weekdays: [Sat, Sun]
- venues: [TSX, TSXV]
  dates: [2026-12-24]
  events:
    - code: OPEN
      time: 09:30:00
    - code: CLOSE
      time: 13:00:00
- venues: [TSX, TSXV]
  events:
    - code: OPEN
      time: 09:30:00
    - code: CLOSE
      time: 16:00:00
)"));
    for(auto venue : {TSX, TSXV}) {
      REQUIRE(schedule.find(
        time_from_string("2026-07-01 12:00:00"), venue).empty());
      REQUIRE(schedule.find(
        time_from_string("2026-12-25 12:00:00"), venue).empty());
      REQUIRE(schedule.find(
        time_from_string("2026-09-19 12:00:00"), venue).empty());
      REQUIRE(schedule.find(
        time_from_string("2026-09-20 12:00:00"), venue).empty());
      auto shortened = schedule.find(
        time_from_string("2026-12-24 12:00:00"), venue);
      REQUIRE(shortened.size() == 2);
      REQUIRE(shortened.back() == TradingSchedule::Event(
        "CLOSE", time_from_string("2026-12-24 18:00:00")));
      for(auto day : {date(2026, 7, 25), date(2026, 12, 1), date(2027, 7, 1)}) {
        auto events = schedule.find(ptime(day, hours(12)), venue);
        if(day.day_of_week() == Saturday || day.day_of_week() == Sunday) {
          REQUIRE(events.empty());
        } else {
          REQUIRE(events.size() == 2);
          REQUIRE(events.back().m_timestamp ==
            venue_to_utc(venue, ptime(day, hours(16))));
        }
      }
    }
  }

  TEST_CASE("canadian_venues") {
    auto schedule = parse_trading_schedule(YAML::Load(R"(
      - venues: [XATX, ADRK, XICX, LICA, NEON]
        time:
          weekdays: [Sat, Sun]
      - venues: [XATX, ADRK, XICX, LICA, NEON]
        dates: [2026-01-01, 2027-01-01]
      - venues: [XATX, ADRK]
        dates: [2026-12-24, 2027-12-24]
        events:
          - {code: OPEN, time: "08:00:00"}
          - {code: CLOSE, time: "13:30:00"}
      - venues: [XICX]
        dates: [2026-12-24, 2027-12-24]
        events:
          - {code: OPEN, time: "09:30:00"}
          - {code: CLOSE, time: "13:00:00"}
      - venues: [LICA, NEON]
        dates: [2026-12-24, 2027-12-24]
        events:
          - {code: OPEN, time: "08:00:00"}
          - {code: CLOSE, time: "13:00:00"}
      - venues: [XATX, ADRK, LICA, NEON]
        events:
          - {code: OPEN, time: "08:00:00"}
          - {code: CLOSE, time: "17:00:00"}
      - venues: [XICX]
        events:
          - {code: OPEN, time: "09:30:00"}
          - {code: CLOSE, time: "16:00:00"})"));
    for(auto venue : {ALX, ALD, ICX, LIQ, NEON}) {
      CAPTURE(venue);
      auto opening = [&] () -> time_duration {
        if(venue == ICX) {
          return duration_from_string("09:30:00");
        }
        return hours(8);
      }();
      auto closing = [&] {
        if(venue == ICX) {
          return hours(16);
        }
        return hours(17);
      }();
      {
        for(auto timestamp : {time_from_string("2026-09-21 12:00:00"),
            time_from_string("2027-01-04 12:00:00")}) {
          auto offset = [&] {
            if(timestamp.date().year() == 2026) {
              return hours(4);
            }
            return hours(5);
          }();
          auto events = schedule.find(timestamp, venue);
          REQUIRE(events == std::vector<TradingSchedule::Event>({
            {"OPEN", ptime(timestamp.date(), opening + offset)},
            {"CLOSE", ptime(timestamp.date(), closing + offset)}}));
        }
      }
      {
        for(auto timestamp : {time_from_string("2026-01-01 12:00:00"),
            time_from_string("2027-01-01 12:00:00"),
            time_from_string("2026-09-19 12:00:00"),
            time_from_string("2026-09-20 12:00:00")}) {
          REQUIRE(schedule.find(timestamp, venue).empty());
        }
      }
      {
        auto close = [&] () -> time_duration {
          if(venue == ALX || venue == ALD) {
            return duration_from_string("18:30:00");
          }
          return hours(18);
        }();
        for(auto timestamp : {time_from_string("2026-12-24 12:00:00"),
            time_from_string("2027-12-24 12:00:00")}) {
          auto events = schedule.find(timestamp, venue);
          REQUIRE(events == std::vector<TradingSchedule::Event>({
            {"OPEN", ptime(timestamp.date(), opening + hours(5))},
            {"CLOSE", ptime(timestamp.date(), close)}}));
        }
      }
    }
  }

  TEST_CASE("stream") {
    auto event = TradingSchedule::Event(
      "EV", time_from_string("1900-01-01 09:30:00"));
    REQUIRE(to_string(event) == "(EV 1900-Jan-01 09:30:00)");
  }

  TEST_CASE("shuttle") {
    auto events = std::vector<TradingSchedule::Event>();
    events.push_back({"O", time_from_string("2025-07-01 01:30:00")});
    auto rules = std::vector<TradingSchedule::Rule>();
    rules.push_back(TradingSchedule::Rule(
      {TSXV}, {Tuesday}, {1}, {7}, {2025}, events));
    auto schedule = TradingSchedule(rules);
    test_round_trip_shuttle(schedule, [&] (const auto& schedule) {
      auto e1 = schedule.find(time_from_string("2025-07-01 12:00:00"), TSXV);
      REQUIRE(e1 == std::vector<TradingSchedule::Event>({
        TradingSchedule::Event(
          "O", time_from_string("2025-07-01 05:30:00"))}));
    });
  }
}
