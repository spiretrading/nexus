#ifndef NEXUS_TICKER_INFO_HPP
#define NEXUS_TICKER_INFO_HPP
#include <functional>
#include <ostream>
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Ticker.hpp"

namespace Nexus {

  /** Stores info about the company/entity represented by a Ticker. */
  struct TickerInfo {

    /** The Ticker represented. */
    Ticker m_ticker;

    /** The full name of the entity. */
    std::string m_name;

    /** The sector the Ticker belongs to. */
    std::string m_sector;

    /** The board lot. */
    Quantity m_board_lot;

    bool operator ==(const TickerInfo&) const = default;
  };
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::TickerInfo> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::TickerInfo& value, unsigned int version) const {
      shuttle.shuttle("ticker", value.m_ticker);
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("sector", value.m_sector);
      shuttle.shuttle("board_lot", value.m_board_lot);
    }
  };
}

namespace std {
  template <>
  struct hash<Nexus::TickerInfo> {
    size_t operator()(const Nexus::TickerInfo& value) const noexcept {
      return Nexus::hash_value(value.m_ticker);
    }
  };
}

template<>
class std::formatter<Nexus::TickerInfo> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::TickerInfo& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {} {} {})", value.m_ticker,
          value.m_name, value.m_sector, value.m_board_lot), context);
    }
};

namespace Nexus {
  inline std::ostream& operator <<(std::ostream& out, const TickerInfo& value) {
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
