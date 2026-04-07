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

  inline std::ostream& operator <<(
      std::ostream& out, const TickerInfo& value) {
    return out << '(' << value.m_ticker << ' ' << value.m_name << ' ' <<
      value.m_sector << ' ' << value.m_board_lot << ')';
  }
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
    size_t operator()(const Nexus::TickerInfo& value) const {
      return Nexus::hash_value(value.m_ticker);
    }
  };
}

#endif
