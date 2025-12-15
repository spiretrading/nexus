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
using namespace Nexus::DefaultVenues;

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
    auto found_events = schedule.find(date(1984, 5, 7), TSXV,
      [] (const auto& event) {
        return event.m_code == "O";
      });
    REQUIRE(found_events.size() == 1);
    REQUIRE(found_events.front() == events.front());
  }

  TEST_CASE("parse_schedule") {
    auto ss = std::stringstream();
    ss << "- venue: TSXV\n"
          "  time:\n"
          "    weekdays: [Sat, Sun]\n"
          "  events:\n"
          "    - code: OPEN\n"
          "      time: 9:30:00";
    auto node = YAML::Load(ss);
    auto schedule = parse_trading_schedule(node, DEFAULT_VENUES);
    auto empty_events = schedule.find(date(2020, 1, 15), TSXV);
    REQUIRE(empty_events.empty());
    auto event = schedule.find(date(2020, 7, 18), TSXV);
    REQUIRE(event.size() == 1);
    REQUIRE(event.front().m_code == "OPEN");
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
      auto e1 = schedule.find(date(2025, 7, 1), TSXV);
      REQUIRE(e1 == events);
    });
  }
}
