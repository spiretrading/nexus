#ifndef NEXUS_STANDARD_TRADING_SCHEDULE_HPP
#define NEXUS_STANDARD_TRADING_SCHEDULE_HPP
#include <string>
#include <utility>
#include <vector>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/TradingSchedule.hpp"

namespace Nexus {
namespace Details {
  inline const TradingSchedule& get_base_trading_schedule() {
    static auto schedule = [] {
      using Date = boost::gregorian::date;
      using Event = TradingSchedule::Event;
      auto rules = std::vector<TradingSchedule::Rule>();
      auto make_event = [] (
          std::string code, int hours, int minutes, int seconds = 0) {
        return Event(std::move(code), boost::posix_time::ptime(Date(1900, 1, 1),
          boost::posix_time::time_duration(hours, minutes, seconds)));
      };
      auto add = [&] (std::vector<Venue> venues, std::vector<Event> events) {
        auto rule = TradingSchedule::Rule();
        rule.m_venues = std::move(venues);
        rule.m_events = std::move(events);
        rules.push_back(std::move(rule));
      };
      auto add_dates = [&] (const std::vector<Venue>& venues,
          const std::vector<Date>& dates, const std::vector<Event>& events) {
        for(auto& date : dates) {
          auto rule = TradingSchedule::Rule();
          rule.m_venues = venues;
          rule.m_days.push_back(date.day());
          rule.m_months.push_back(date.month());
          rule.m_years.push_back(date.year());
          rule.m_events = events;
          rules.push_back(std::move(rule));
        }
      };
      auto australian_venues = std::vector{Venue("XASX"), Venue("CHIA")};
      auto canadian_venues = std::vector{
        Venue("XATS"), Venue("CHIC"), Venue("XCNQ"), Venue("CSE2"),
        Venue("XCXD"), Venue("XCX2"), Venue("LYNX"), Venue("MATN"),
        Venue("NEOE"), Venue("OMGA"), Venue("PURE"), Venue("XTSE"),
        Venue("XTSX"), Venue("XATX"), Venue("ADRK"), Venue("XICX"),
        Venue("LICA"), Venue("NEON")};
      {
        auto rule = TradingSchedule::Rule();
        rule.m_venues = australian_venues;
        rule.m_venues.insert(rule.m_venues.end(), canadian_venues.begin(),
          canadian_venues.end());
        rule.m_weekdays.push_back(boost::gregorian::Saturday);
        rule.m_weekdays.push_back(boost::gregorian::Sunday);
        rules.push_back(std::move(rule));
      }
      add_dates(australian_venues, {
        Date(2026, 1, 1), Date(2026, 1, 26), Date(2026, 4, 3), Date(2026, 4, 6),
        Date(2026, 6, 8), Date(2026, 12, 25), Date(2026, 12, 28),
        Date(2027, 1, 1), Date(2027, 1, 26), Date(2027, 3, 26),
        Date(2027, 3, 29), Date(2027, 6, 14), Date(2027, 12, 27),
        Date(2027, 12, 28)}, {});
      add_dates(canadian_venues, {
        Date(2026, 1, 1), Date(2026, 2, 16), Date(2026, 4, 3),
        Date(2026, 5, 18), Date(2026, 7, 1), Date(2026, 8, 3), Date(2026, 9, 7),
        Date(2026, 10, 12), Date(2026, 12, 25), Date(2026, 12, 28),
        Date(2027, 1, 1), Date(2027, 2, 15), Date(2027, 3, 26),
        Date(2027, 5, 24), Date(2027, 7, 1), Date(2027, 8, 2), Date(2027, 9, 6),
        Date(2027, 10, 11), Date(2027, 12, 27), Date(2027, 12, 28)}, {});
      auto canadian_half_days =
        std::vector{Date(2026, 12, 24), Date(2027, 12, 24)};
      auto australian_half_days = std::vector{Date(2026, 12, 24),
        Date(2026, 12, 31), Date(2027, 12, 24), Date(2027, 12, 31)};
      add_dates({Venue("XTSE"), Venue("XTSX")}, canadian_half_days, {
        make_event("PRE_OPEN", 7, 0), make_event("OPEN", 9, 30),
        make_event("CLOSE", 13, 0)});
      add_dates({Venue("LICA"), Venue("NEON")}, canadian_half_days, {
        make_event("OPEN", 8, 0), make_event("CLOSE", 13, 0)});
      add_dates({Venue("XATS"), Venue("XATX"), Venue("ADRK")},
        canadian_half_days, {make_event("PRE_OPEN", 7, 0),
        make_event("OPEN", 8, 0), make_event("CLOSE", 13, 30)});
      add_dates({Venue("CHIC"), Venue("XCX2")}, canadian_half_days, {
        make_event("OPEN", 8, 0), make_event("CLOSE", 13, 30)});
      add_dates({Venue("XCXD")}, canadian_half_days, {
        make_event("OPEN", 9, 30), make_event("CLOSE", 13, 30)});
      add_dates({Venue("XASX")}, australian_half_days, {
        make_event("PRE_OPEN", 7, 0), make_event("OPEN", 9, 59, 45),
        make_event("CLOSE", 14, 0)});
      add_dates({Venue("CHIA")}, australian_half_days, {
        make_event("OPEN", 10, 0), make_event("CLOSE", 14, 13)});
      add_dates({Venue("XCNQ"), Venue("NEOE"), Venue("MATN")},
        canadian_half_days, {make_event("PRE_OPEN", 7, 0),
        make_event("OPEN", 9, 30), make_event("CLOSE", 13, 0)});
      add_dates({Venue("LYNX"), Venue("XICX")}, canadian_half_days, {
        make_event("OPEN", 9, 30), make_event("CLOSE", 13, 0)});
      add_dates({Venue("PURE")}, canadian_half_days, {
        make_event("PRE_OPEN", 7, 0), make_event("OPEN", 8, 0),
        make_event("CLOSE", 13, 30)});
      add_dates({Venue("CSE2"), Venue("OMGA")}, canadian_half_days, {
        make_event("OPEN", 8, 0), make_event("CLOSE", 13, 30)});
      add({Venue("XTSE"), Venue("XTSX"), Venue("XCNQ"), Venue("NEOE"),
        Venue("MATN")}, {make_event("PRE_OPEN", 7, 0),
        make_event("OPEN", 9, 30), make_event("CLOSE", 16, 0)});
      add({Venue("XATS"), Venue("XATX"), Venue("ADRK"), Venue("PURE")}, {
        make_event("PRE_OPEN", 7, 0), make_event("OPEN", 8, 0),
        make_event("CLOSE", 17, 0)});
      add({Venue("CHIC"), Venue("XCX2"), Venue("CSE2"), Venue("OMGA"),
        Venue("LICA"), Venue("NEON")}, {make_event("OPEN", 8, 0),
        make_event("CLOSE", 17, 0)});
      add({Venue("XCXD"), Venue("LYNX"), Venue("XICX")}, {
        make_event("OPEN", 9, 30), make_event("CLOSE", 16, 0)});
      add({Venue("XASX")}, {
        make_event("PRE_OPEN", 7, 0), make_event("OPEN", 9, 59, 45),
        make_event("CLOSE", 16, 0)});
      add({Venue("CHIA")}, {make_event("OPEN", 10, 0),
        make_event("CLOSE", 16, 13)});
      return TradingSchedule(std::move(rules));
    }();
    return schedule;
  }

  inline auto trading_schedule = get_base_trading_schedule();
}

  /**
   * Returns the default TradingSchedule, typically used for testing purposes.
   */
  inline const TradingSchedule& TRADING_SCHEDULE = Details::trading_schedule;

  /** Updates the default TradingSchedule. */
  inline void set_trading_schedule(TradingSchedule schedule) {
    Details::trading_schedule = std::move(schedule);
  }
}

#endif
