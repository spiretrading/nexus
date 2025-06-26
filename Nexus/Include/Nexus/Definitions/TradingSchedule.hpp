#ifndef NEXUS_TRADING_SCHEDULE_HPP
#define NEXUS_TRADING_SCHEDULE_HPP
#include <algorithm>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>
#include <Beam/Parsers/Parse.hpp>
#include <Beam/Parsers/TimeDurationParser.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Stores a database of trading events. */
  class TradingSchedule {
    public:

      /** Stores an event. */
      struct Event {

        /** The exchange specific code identifying the event. */
        std::string m_code;

        /**
         * The point in time when the event occurs in the time zone of the
         * venue.
         */
        boost::posix_time::ptime m_timestamp;

        bool operator ==(const Event& event) const = default;
      };

      /**
       * A rule that specifies a list of events that take place if a date
       * matches a pattern.
       */
      struct Rule {

        /** The venues to apply the rule to. */
        std::vector<Venue> m_venues;

        /** The days of the week to match, leave empty to match every day. */
        std::vector<boost::gregorian::greg_weekday> m_weekdays;

        /** The days of the month to match, leave empty to match every day. */
        std::vector<boost::gregorian::greg_day> m_days;

        /** The months to match, leave empty to match every month. */
        std::vector<boost::gregorian::greg_month> m_months;

        /** The years to match, leave empty to match every year. */
        std::vector<boost::gregorian::greg_year> m_years;

        /**
         * The events taking place matching this rule, only time of day is
         * considered.
         */
        std::vector<Event> m_events;

        bool operator ==(const Rule& rule) const = default;
      };

      /** Constructs an empty TradingSchedule. */
      TradingSchedule() = default;

      /**
       * Constructs a TradingSchedule.
       * @param rules The list of rules in order of precedence to match against.
       */
      explicit TradingSchedule(std::vector<Rule> rules);

      /**
       * Returns a list of events matching a date and venue.
       * @param date The date to match.
       * @param venue The venue to match.
       * @return A list of events taking place on the specified <i>date</i> and
       *         <i>venue</i>.
       */
      std::vector<Event> Find(boost::gregorian::date date, Venue venue) const;

      /**
       * Returns a list of events matching a date, venue, and predicate.
       * @param date The date to match.
       * @param venue The venue to match.
       * @param f The predicate the event must satisfy.
       * @return A list of events taking place on the specified <i>date</i> and
       *         <i>venue</i> and matching <i>f</i>.
       */
      template<typename F>
      std::vector<Event> Find(
        boost::gregorian::date date, Venue venue, F&& f) const;

    private:
      friend struct Beam::Serialization::Shuttle<TradingSchedule>;
      std::vector<Rule> m_rules;
  };

  /**
   * Tests if a venue and date match a rule.
   * @param venue The venue to test.
   * @param date The date to test.
   * @param rule The rule to match.
   * @return <code>true</code> iff the <i>venue</i> is equal to the
   *         <i>rule</i>'s venue and the <i>date</i> is a subset of the dates
   *         specified by the <i>rule</i>.
   */
  inline bool IsMatch(Venue venue, boost::gregorian::date date,
      const TradingSchedule::Rule& rule) {
    if(!rule.m_venues.empty()) {
      if(std::find(rule.m_venues.begin(), rule.m_venues.end(), venue) ==
          rule.m_venues.end()) {
        return false;
      }
    }
    if(!rule.m_weekdays.empty()) {
      if(std::find(rule.m_weekdays.begin(), rule.m_weekdays.end(),
          date.day_of_week()) == rule.m_weekdays.end()) {
        return false;
      }
    }
    if(!rule.m_days.empty()) {
      if(std::find(rule.m_days.begin(), rule.m_days.end(), date.day()) ==
          rule.m_days.end()) {
        return false;
      }
    }
    if(!rule.m_months.empty()) {
      if(std::find(rule.m_months.begin(), rule.m_months.end(), date.month()) ==
          rule.m_months.end()) {
        return false;
      }
    }
    if(!rule.m_years.empty()) {
      if(std::find(rule.m_years.begin(), rule.m_years.end(), date.year()) ==
          rule.m_years.end()) {
        return false;
      }
    }
    return true;
  }

  /**
   * Parses a TradingSchedule::Rule from a YAML node.
   * @param node The node to parse the TradingSchedule from.
   * @param database The database used to parse venues.
   * @return The list of TradingSchedule::Rules represented by the <i>node</i>.
   */
  inline std::vector<TradingSchedule::Rule> ParseTradingScheduleRules(
      const YAML::Node& node, const VenueDatabase& database) {
    auto rules = std::vector<TradingSchedule::Rule>();
    rules.emplace_back();
    for(auto& venueNode : node["venues"]) {
      rules.back().m_venues.push_back(
        parse_venue(Beam::Extract<std::string>(venueNode), database));
    }
    for(auto& eventNode : node["events"]) {
      auto event = TradingSchedule::Event();
      auto time = Beam::Parsers::Parse<boost::posix_time::time_duration>(
        Beam::Extract<std::string>(eventNode, "time"));
      event.m_timestamp = boost::posix_time::ptime(boost::gregorian::date(
        1900, 1, 1), time);
      event.m_code = Beam::Extract<std::string>(eventNode, "code");
      rules.back().m_events.push_back(event);
    }
    std::sort(rules.back().m_events.begin(), rules.back().m_events.end(),
      [] (auto& left, auto& right) {
        return std::tie(left.m_timestamp, left.m_code) <
          std::tie(right.m_timestamp, left.m_code);
      });
    auto& timeNode = node["time"];
    if(timeNode.IsDefined()) {
      auto& weekdaysNode = timeNode["weekdays"];
      if(weekdaysNode.IsDefined()) {
        for(auto& weekdayNode : weekdaysNode) {
          rules.back().m_weekdays.push_back(
            Beam::Extract<boost::gregorian::greg_weekday>(weekdayNode));
        }
      }
      auto& daysNode = timeNode["days"];
      if(daysNode.IsDefined()) {
        for(auto& dayNode : daysNode) {
          rules.back().m_days.push_back(
            Beam::Extract<boost::gregorian::greg_day>(dayNode));
        }
      }
      auto& monthsNode = timeNode["months"];
      if(monthsNode.IsDefined()) {
        for(auto& monthNode : monthsNode) {
          rules.back().m_months.push_back(
            Beam::Extract<boost::gregorian::greg_month>(monthNode));
        }
      }
      auto& yearsNode = timeNode["years"];
      if(yearsNode.IsDefined()) {
        for(auto& yearNode : yearsNode) {
          rules.back().m_years.push_back(
            Beam::Extract<boost::gregorian::greg_year>(yearNode));
        }
      }
    } else {
      auto& datesNode = node["dates"];
      if(datesNode.IsDefined()) {
        auto rule = rules.back();
        rules.pop_back();
        for(auto& dateNode : datesNode) {
          auto date = Beam::Extract<boost::gregorian::date>(dateNode);
          auto dateRule = rule;
          dateRule.m_days.push_back(date.day());
          dateRule.m_months.push_back(date.month());
          dateRule.m_years.push_back(date.year());
          rules.push_back(dateRule);
        }
      }
    }
    return rules;
  }

  /**
   * Parses a TradingSchedule from a YAML node.
   * @param node The node to parse the TradingSchedule from.
   * @param database The database used to parse venues.
   * @return The TradingSchedule represented by the <i>node</i>.
   */
  inline TradingSchedule ParseTradingSchedule(const YAML::Node& node,
      const VenueDatabase& database) {
    auto rules = std::vector<TradingSchedule::Rule>();
    for(auto& node : node) {
      auto subRules = ParseTradingScheduleRules(node, database);
      rules.insert(rules.end(), std::make_move_iterator(subRules.begin()),
        std::make_move_iterator(subRules.end()));
    }
    return TradingSchedule(std::move(rules));
  }

  inline std::ostream& operator <<(std::ostream& out,
      const TradingSchedule::Event& event) {
    return out << '(' << " " << event.m_code << " " << event.m_timestamp << ')';
  }

  inline TradingSchedule::TradingSchedule(std::vector<Rule> rules)
    : m_rules(std::move(rules)) {}

  inline std::vector<TradingSchedule::Event> TradingSchedule::Find(
      boost::gregorian::date date, Venue venue) const {
    return Find(date, venue, [] (const auto&) { return true; });
  }

  template<typename F>
  std::vector<TradingSchedule::Event> TradingSchedule::Find(
      boost::gregorian::date date, Venue venue, F&& f) const {
    for(auto& rule : m_rules) {
      if(IsMatch(venue, date, rule)) {
        auto events = std::vector<TradingSchedule::Event>();
        for(auto& event : rule.m_events) {
          if(f(event)) {
            events.push_back(event);
            events.back().m_timestamp = boost::posix_time::ptime(date,
              event.m_timestamp.time_of_day());
          }
        }
        return events;
      }
    }
    return {};
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::TradingSchedule::Event> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::TradingSchedule::Event& value,
        unsigned int version) {
      shuttle.Shuttle("code", value.m_code);
      shuttle.Shuttle("timestamp", value.m_timestamp);
    }
  };

  template<>
  struct Shuttle<Nexus::TradingSchedule::Rule> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::TradingSchedule::Rule& value,
        unsigned int version) {
      shuttle.Shuttle("venues", value.m_venues);
      shuttle.Shuttle("weekdays", value.m_weekdays);
      shuttle.Shuttle("days", value.m_days);
      shuttle.Shuttle("months", value.m_months);
      shuttle.Shuttle("years", value.m_years);
      shuttle.Shuttle("events", value.m_events);
    }
  };

  template<>
  struct Shuttle<Nexus::TradingSchedule> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::TradingSchedule& value,
        unsigned int version) {
      shuttle.Shuttle("rules", value.m_rules);
    }
  };
}

#endif
