#ifndef NEXUS_DEFINITIONS_TICKER_INFO_HPP
#define NEXUS_DEFINITIONS_TICKER_INFO_HPP
#include <ostream>
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Instrument.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Ticker.hpp"

namespace Nexus {

  /** Stores the trading properties of a ticker. */
  struct TickerInfo {

    /** The ticker. */
    Ticker m_ticker;

    /** The display name. */
    std::string m_name;

    /** The instrument. */
    Instrument m_instrument;

    /** The minimum price increment. */
    Money m_tick_size;

    /** The minimum order quantity. */
    Quantity m_lot_size;

    /** The standard trading unit. */
    Quantity m_board_lot;

    /** The price granularity. */
    Money m_price_resolution;

    /** The quantity granularity. */
    Quantity m_quantity_resolution;

    /** The contract multiplier. */
    Quantity m_multiplier;

    bool operator ==(const TickerInfo&) const = default;
  };

  inline std::ostream& operator <<(std::ostream& out, const TickerInfo& value) {
    return out << '(' << value.m_ticker << ' ' << value.m_name << ' ' <<
      value.m_instrument << ' ' << value.m_tick_size << ' ' <<
      value.m_lot_size << ' ' << value.m_board_lot << ' ' <<
      value.m_price_resolution << ' ' << value.m_quantity_resolution << ' ' <<
      value.m_multiplier << ')';
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
      shuttle.shuttle("instrument", value.m_instrument);
      shuttle.shuttle("tick_size", value.m_tick_size);
      shuttle.shuttle("lot_size", value.m_lot_size);
      shuttle.shuttle("board_lot", value.m_board_lot);
      shuttle.shuttle("price_resolution", value.m_price_resolution);
      shuttle.shuttle("quantity_resolution", value.m_quantity_resolution);
      shuttle.shuttle("multiplier", value.m_multiplier);
    }
  };
}

#endif
