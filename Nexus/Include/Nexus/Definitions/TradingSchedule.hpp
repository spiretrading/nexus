#ifndef NEXUS_TRADING_SCHEDULE_HPP
#define NEXUS_TRADING_SCHEDULE_HPP
#include <ostream>
#include <string>
#include <vector>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus {

  /** Stores a database of trading events. */
  class TradingSchedule {
    public:

      /** Lists the abstract types of scheduled events. */
      enum class Type {

        /** The market enters its pre-opening phase. */
        PRE_OPEN,

        /** The market enters its opening phase. */
        OPEN,

        /** The market enters its closing phase. */
        CLOSE,

        /** Used for events specific to an exchange. */
        OTHER
      };

      /** Stores an event. */
      struct Event {

        /** The type of event taking place. */
        Type m_type;

        /** The exchange specific code identifying the event. */
        std::string m_code;

        /**
         * The point in time when the event occurs in the time zone of the
         * market.
         */
        boost::posix_time::ptime m_timestamp;

        /** Tests if two objects denote the same event. */
        bool operator ==(const Event& event) const;

        /** Tests if two objects denote different events. */
        bool operator !=(const Event& event) const;
      };

      /**
       * A rule that specifies a list of events that take place if a date
       * matches a pattern.
       */
      struct Rule {

        /** The market to apply the rule to. */
        MarketCode m_market;

        /** The days of the week to match, leave empty to match every day. */
        std::vector<boost::gregorian::greg_weekday> m_daysOfWeek;

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

        /** Tests if two objects specify the same rule. */
        bool operator ==(const Rule& rule) const;

        /** Tests if two objects specify different rules. */
        bool operator !=(const Rule& rule) const;
      };

      /** Constructs an empty TradingSchedule. */
      TradingSchedule() = default;

      /**
       * Constructs a TradingSchedule.
       * @param rules The list of rules in order of precedence to match against.
       */
      explicit TradingSchedule(std::vector<Rule> rules);

      /**
       * Returns a list of events matching a date and market.
       * @param date The date to match.
       * @param market The market to match.
       * @return A list of events taking place on the specified <i>date</i> and
       *         <i>market</i>.
       */
      std::vector<Event> Find(boost::gregorian::date date,
        MarketCode market) const;

      /**
       * Returns a list of events matching a date, market, and predicate.
       * @param date The date to match.
       * @param market The market to match.
       * @param f The predicate the event must satisfy.
       * @return A list of events taking place on the specified <i>date</i> and
       *         <i>market</i> and matching <i>f</i>.
       */
      template<typename F>
      std::vector<Event> Find(boost::gregorian::date date,
        MarketCode market, F&& f) const;

    private:
      friend struct Beam::Serialization::Shuttle<TradingSchedule>;
      std::vector<Rule> m_rules;
  };

  /**
   * Tests if a market and date match a rule.
   * @param market The market to test.
   * @param date The date to test.
   * @param rule The rule to match.
   * @return <code>true</code> iff the <i>market</i> is equal to the
   *         <i>rule</i>'s market and the <i>date</i> is a subset of the
   *         dates specified by the <i>rule</i>.
   */
  inline bool IsMatch(MarketCode market, boost::gregorian::date date,
      const TradingSchedule::Rule& rule) {
    if(market != rule.m_market) {
      return false;
    }
    if(!rule.m_daysOfWeek.empty()) {
      if(std::find(rule.m_daysOfWeek.begin(), rule.m_daysOfWeek.end(),
          date.day_of_week()) == rule.m_daysOfWeek.end()) {
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
   * Parses a TradingSchedule from a YAML node.
   * @param node The node to parse the TradingSchedule from.
   * @return The TradingSchedule represented by the <i>node</i>.
   */
  inline TradingSchedule ParseTradingSchedule(const YAML::Node& node) {
    auto database = TradingSchedule();
    return database;
  }

  inline std::ostream& operator <<(std::ostream& out,
      TradingSchedule::Type type) {
    if(type == TradingSchedule::Type::PRE_OPEN) {
      return out << "PRE_OPEN";
    } else if(type == TradingSchedule::Type::OPEN) {
      return out << "OPEN";
    } else if(type == TradingSchedule::Type::CLOSE) {
      return out << "CLOSE";
    } else {
      return out << "OTHER";
    }
  }

  inline std::ostream& operator <<(std::ostream& out,
      const TradingSchedule::Event& event) {
    return out << '(' << event.m_type << " " << event.m_code << " " <<
      event.m_timestamp << ')';
  }

  inline bool TradingSchedule::Event::operator ==(const Event& event) const {
    return m_type == event.m_type && m_code == event.m_code &&
      m_timestamp == event.m_timestamp;
  }

  inline bool TradingSchedule::Event::operator !=(const Event& event) const {
    return !(*this == event);
  }

  inline bool TradingSchedule::Rule::operator ==(const Rule& rule) const {
    return m_market == rule.m_market && m_daysOfWeek == rule.m_daysOfWeek &&
      m_days == rule.m_days && m_months == rule.m_months &&
      m_years == rule.m_years && m_events == rule.m_events;
  }

  inline bool TradingSchedule::Rule::operator !=(const Rule& rule) const {
    return !(*this == rule);
  }

  inline TradingSchedule::TradingSchedule(std::vector<Rule> rules)
    : m_rules(std::move(rules)) {}

  inline std::vector<TradingSchedule::Event> TradingSchedule::Find(
      boost::gregorian::date date, MarketCode market) const {
    return Find(date, market, [] (const auto&) { return true; });
  }

  template<typename F>
  std::vector<TradingSchedule::Event> TradingSchedule::Find(
      boost::gregorian::date date, MarketCode market, F&& f) const {
    for(auto& rule : m_rules) {
      if(IsMatch(market, date, rule)) {
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
      shuttle.Shuttle("type", value.m_type);
      shuttle.Shuttle("code", value.m_code);
      shuttle.Shuttle("timestamp", value.m_timestamp);
    }
  };

  template<>
  struct Shuttle<Nexus::TradingSchedule::Rule> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::TradingSchedule::Rule& value,
        unsigned int version) {
      shuttle.Shuttle("market", value.m_market);
      shuttle.Shuttle("days_of_week", value.m_daysOfWeek);
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
