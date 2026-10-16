#ifndef NEXUS_TICKER_STATUS_HPP
#define NEXUS_TICKER_STATUS_HPP
#include <cstdint>
#include <ostream>
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Utilities/ToString.hpp>
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

template<>
class std::formatter<Nexus::TickerStatus::Flag> :
    public std::formatter<std::string_view> {
  public:
    auto format(Nexus::TickerStatus::Flag flags, auto& context) const {
      auto text = [&] {
        auto count = 0;
        auto text = std::string();
        auto append = [&] (Nexus::TickerStatus::Flag flag, const char* name) {
          if(has(flags, flag)) {
            if(count != 0) {
              text += " | ";
            }
            text += name;
            ++count;
          }
        };
        using Flag = Nexus::TickerStatus::Flag;
        append(Flag::IS_MATCHING, "IS_MATCHING");
        append(Flag::IS_ACCEPTING_ORDERS, "IS_ACCEPTING_ORDERS");
        append(Flag::IS_ACCEPTING_CANCELS, "IS_ACCEPTING_CANCELS");
        append(Flag::IS_AUCTION, "IS_AUCTION");
        if(count == 0) {
          return std::string("NONE");
        } else if(count == 1) {
          return text;
        }
        return std::format("({})", text);
      }();
      return std::formatter<std::string_view>::format(text, context);
    }
};

template<>
class std::formatter<Nexus::TickerStatus> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::TickerStatus& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {} {} {})", value.m_venue,
          value.m_state, value.m_flags, Beam::to_string(value.m_timestamp)),
        context);
    }
};

namespace Nexus {
  inline std::ostream& operator <<(
      std::ostream& out, TickerStatus::Flag flags) {
    return out << std::format("{}", flags);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const TickerStatus& value) {
    auto database = static_cast<const VenueDatabase*>(
      out.pword(Beam::ScopedStreamManipulator<VenueDatabase>::ID));
    if(database) {
      return out <<
        std::format(make_locale(out.getloc(), *database), "{}", value);
    }
    return out << std::format(out.getloc(), "{}", value);
  }
}

#endif
