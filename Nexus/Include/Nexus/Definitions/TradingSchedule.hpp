#ifndef NEXUS_TRADING_SCHEDULE_HPP
#define NEXUS_TRADING_SCHEDULE_HPP
#include <string>
#include <vector>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus {

  /** Stores a list of trading events for a single market. */
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

      /** Constructs an empty TradingSchedule. */
      TradingSchedule() = default;

      /**
       * Constructs a TradingSchedule.
       * @param events The list of events, ordered by time.
       */
      explicit TradingSchedule(std::vector<Event> events);

      /** Returns the list of events ordered by time. */
      const std::vector<Event>& GetEvents() const;

    private:
      friend struct Beam::Serialization::Shuttle<TradingSchedule>;
      std::vector<Event> m_events;
  };

  /**
   * Filters a schedule based on a predicate.
   * @param schedule The TradingSchedule to filter.
   * @param f A callable object taking an Event and returning <code>true</code>
   *        iff that event should be included in the returned TradingSchedule.
   * @return A TradingSchedule consisting of all events in <i>schedule</i>
   *         such that the predicate <i>f</i> returns <code>true</code>.
   */
  template<typename F>
  TradingSchedule Filter(const TradingSchedule& schedule, F&& f) {
    auto subEvents = std::vector<TradingSchedule::Event>();
    std::copy_if(schedule.GetEvents().begin(), schedule.GetEvents().end(),
      std::back_inserter(subEvents), std::forward<F>(f));
    return TradingSchedule(std::move(subEvents));
  }

  /** Filters a TradingSchedule by event type. */
  inline TradingSchedule Filter(const TradingSchedule& schedule,
      TradingSchedule::Type type) {
    return Filter(schedule, [&] (const auto& event) {
      return event.m_type == type;
    });
  }

  inline bool TradingSchedule::Event::operator ==(const Event& event) const {
    return m_type == event.m_type && m_code == event.m_code &&
      m_timestamp == event.m_timestamp;
  }

  inline bool TradingSchedule::Event::operator !=(const Event& event) const {
    return !(*this == event);
  }

  inline TradingSchedule::TradingSchedule(std::vector<Event> events)
    : m_events(std::move(events)) {}

  inline const std::vector<TradingSchedule::Event>&
      TradingSchedule::GetEvents() const {
    return m_events;
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
  struct Shuttle<Nexus::TradingSchedule> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::TradingSchedule& value,
        unsigned int version) {
      shuttle.Shuttle("events", value.m_events);
    }
  };
}

#endif
