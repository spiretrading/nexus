#ifndef NEXUS_POSITION_HPP
#define NEXUS_POSITION_HPP
#include <cstdint>
#include <ostream>
#include <utility>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/functional/hash.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"

namespace Nexus {

  /** Stores information about a single position. */
  struct Position {

    /** Stores a key that can be used to identify a position. */
    struct Key {

      /** The position's security. */
      Security m_security;

      /** The currency used to trade the position. */
      CurrencyId m_currency;

      bool operator ==(const Key&) const = default;
    };

    /** The security held. */
    Security m_security;

    /** The position's currency. */
    CurrencyId m_currency;

    /** The quantity of inventory held. */
    Quantity m_quantity;

    /** The total cost of the currently held inventory. */
    Money m_cost_basis;

    bool operator ==(const Position&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const Position& position) {
    return out << '(' << position.m_security << ' ' << position.m_currency <<
      ' ' << position.m_quantity << ' ' << position.m_cost_basis << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const Position::Key& key) {
    return out << '(' << key.m_security << ' ' << key.m_currency << ')';
  }

  /** Returns a Position's key. */
  inline Position::Key get_key(const Position& position) {
    return Position::Key(position.m_security, position.m_currency);
  }

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
      shuttle.shuttle("security", value.m_security);
      shuttle.shuttle("currency", value.m_currency);
      shuttle.shuttle("quantity", value.m_quantity);
      shuttle.shuttle("cost_basis", value.m_cost_basis);
    }
  };

  template<>
  struct Shuttle<Nexus::Position::Key> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::Position::Key& key, unsigned int version) const {
      shuttle.shuttle("security", key.m_security);
      shuttle.shuttle("currency", key.m_currency);
    }
  };
}

namespace std {
  template<>
  struct hash<Nexus::Position::Key> {
    size_t operator()(const Nexus::Position::Key& value) const {
      auto seed = size_t(0);
      boost::hash_combine(seed, std::hash<Nexus::Security>()(value.m_security));
      boost::hash_combine(seed, value.m_currency);
      return seed;
    }
  };
}

#endif
