#ifndef NEXUS_MARKET_DATA_TICKER_INFO_HPP
#define NEXUS_MARKET_DATA_TICKER_INFO_HPP
#include <ostream>
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Instrument.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"

namespace Nexus {

  /** Stores the trading properties of a ticker. */
  struct TickerInfo {

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

    /** The number of decimal places for prices. */
    int m_price_precision;

    /** The number of decimal places for quantities. */
    int m_quantity_precision;

    /** The contract multiplier. */
    Quantity m_multiplier;

    bool operator ==(const TickerInfo&) const = default;
  };

  inline std::ostream& operator <<(std::ostream& out, const TickerInfo& value) {
    return out << '(' << value.m_name << ' ' << value.m_instrument << ' ' <<
      value.m_tick_size << ' ' << value.m_lot_size << ' ' <<
      value.m_board_lot << ' ' << value.m_price_precision << ' ' <<
      value.m_quantity_precision << ' ' << value.m_multiplier << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::TickerInfo> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::TickerInfo& value, unsigned int version) const {
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("instrument", value.m_instrument);
      shuttle.shuttle("tick_size", value.m_tick_size);
      shuttle.shuttle("lot_size", value.m_lot_size);
      shuttle.shuttle("board_lot", value.m_board_lot);
      shuttle.shuttle("price_precision", value.m_price_precision);
      shuttle.shuttle("quantity_precision", value.m_quantity_precision);
      shuttle.shuttle("multiplier", value.m_multiplier);
    }
  };
}

#endif
