#ifndef NEXUS_POSITION_HPP
#define NEXUS_POSITION_HPP
#include <cstdint>
#include <ostream>
#include <utility>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Ticker.hpp"

namespace Nexus {

  /** Stores information about a single position. */
  struct Position {

    /** The ticker held. */
    Ticker m_ticker;

    /** The position's currency. */
    CurrencyId m_currency;

    /** The quantity of inventory held. */
    Quantity m_quantity;

    /** The total cost of the currently held inventory. */
    Money m_cost_basis;

    bool operator ==(const Position&) const = default;
  };

  /**
   * Returns the average price of a Position.
   * @param position The Position to measure.
   * @return The average price of the <i>position</i>.
   */
  inline Money get_average_price(const Position& position) {
    if(position.m_quantity == 0) {
      return Money::ZERO;
    }
    return position.m_cost_basis / position.m_quantity;
  }

  /**
   * Returns the Position's Side.
   * @param position The Position to measure.
   * @return The Side corresponding to the <i>position</i>.
   */
  inline Side get_side(const Position& position) {
    return get_side(position.m_quantity);
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::Position> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::Position& value, unsigned int version) const {
      shuttle.shuttle("ticker", value.m_ticker);
      shuttle.shuttle("currency", value.m_currency);
      shuttle.shuttle("quantity", value.m_quantity);
      shuttle.shuttle("cost_basis", value.m_cost_basis);
    }
  };
}

template<>
class std::formatter<Nexus::Position> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::Position& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {} {} {})", value.m_ticker,
          value.m_currency, value.m_quantity, value.m_cost_basis), context);
    }
};

namespace Nexus {
  inline std::ostream& operator <<(std::ostream& out, const Position& value) {
    auto locale = out.getloc();
    if(auto database = static_cast<const CurrencyDatabase*>(
        out.pword(Beam::ScopedStreamManipulator<CurrencyDatabase>::ID))) {
      locale = make_locale(locale, *database);
    }
    if(auto database = static_cast<const VenueDatabase*>(
        out.pword(Beam::ScopedStreamManipulator<VenueDatabase>::ID))) {
      locale = make_locale(locale, *database);
    }
    return out << std::format(locale, "{}", value);
  }
}

#endif
