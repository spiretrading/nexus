#ifndef NEXUS_TICKER_STATUS_HPP
#define NEXUS_TICKER_STATUS_HPP
#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Represents the current trading status of a ticker on a venue. */
  struct TickerStatus {

    /** Bit flags indicating what trading operations are currently permitted. */
    enum class Flag : std::uint8_t {

      /** No operations are permitted. */
      NONE = 0x00,

      /** Continuous order matching is active. */
      IS_MATCHING = 0x01,

      /** New orders can be submitted. */
      IS_ACCEPTING_ORDERS = 0x02,

      /** Cancellations are accepted. */
      IS_ACCEPTING_CANCELS = 0x04,

      /** An auction phase is in progress. */
      IS_AUCTION = 0x08,

      /**
       * Continuous trading is active (matching, accepting orders and cancels).
       */
      IS_CONTINUOUS = std::uint8_t(IS_MATCHING) |
        std::uint8_t(IS_ACCEPTING_ORDERS) | std::uint8_t(IS_ACCEPTING_CANCELS)
    };

    /** The venue this status applies to. */
    Venue m_venue;

    /** The venue's native state identifier. */
    std::string m_state;

    /** The abstract trading flags. */
    Flag m_flags = Flag::NONE;

    /** The timestamp of the status change. */
    boost::posix_time::ptime m_timestamp;

    bool operator ==(const TickerStatus&) const = default;
  };

  constexpr TickerStatus::Flag operator |(
      TickerStatus::Flag lhs, TickerStatus::Flag rhs) {
    return static_cast<TickerStatus::Flag>(
      static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
  }

  constexpr TickerStatus::Flag operator &(
      TickerStatus::Flag lhs, TickerStatus::Flag rhs) {
    return static_cast<TickerStatus::Flag>(
      static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
  }

  constexpr bool has(TickerStatus::Flag flags, TickerStatus::Flag flag) {
    return (flags & flag) == flag;
  }

  inline std::ostream& operator <<(
      std::ostream& out, TickerStatus::Flag flags) {
    auto count = 0;
    auto stream = std::ostringstream();
    auto print = [&] (TickerStatus::Flag flag, const char* name) {
      if(has(flags, flag)) {
        if(count > 0) {
          stream << " | ";
        }
        stream << name;
        ++count;
      }
    };
    print(TickerStatus::Flag::IS_MATCHING, "IS_MATCHING");
    print(TickerStatus::Flag::IS_ACCEPTING_ORDERS, "IS_ACCEPTING_ORDERS");
    print(TickerStatus::Flag::IS_ACCEPTING_CANCELS, "IS_ACCEPTING_CANCELS");
    print(TickerStatus::Flag::IS_AUCTION, "IS_AUCTION");
    if(count == 0) {
      out << "NONE";
    } else if(count == 1) {
      out << stream.str();
    } else {
      out << '(' << stream.str() << ')';
    }
    return out;
  }

  inline std::ostream& operator <<(
      std::ostream& out, const TickerStatus& value) {
    return out << '(' << value.m_venue << ' ' << value.m_state << ' ' <<
      value.m_flags << ' ' << value.m_timestamp << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::TickerStatus> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::TickerStatus& value, unsigned int version) const {
      shuttle.shuttle("venue", value.m_venue);
      shuttle.shuttle("state", value.m_state);
      shuttle.shuttle("flags", value.m_flags);
      shuttle.shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
