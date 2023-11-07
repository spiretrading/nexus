#ifndef NEXUS_POSITION_HPP
#define NEXUS_POSITION_HPP
#include <cstdint>
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/functional/hash.hpp>
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Side.hpp"

namespace Nexus::Accounting {
namespace Details {

  /**
   * Identifies an inventory managed in a specific Currency.
   * @param I The type of index used.
   */
  template<typename I>
  struct Key {

    /** The type of index used. */
    using Index = I;

    /** The inventory's index. */
    Index m_index;

    /** The Currency used to value the inventory. */
    CurrencyId m_currency;

    bool operator ==(const Key& key) const = default;
  };

  template<typename Index>
  std::ostream& operator <<(std::ostream& out, const Key<Index>& key) {
    return out << '(' << key.m_index << ' ' << key.m_currency << ')';
  }
}

  /**
   * Stores information about a single Inventory position.
   * @param I Used to identify the Position.
   */
  template<typename I>
  struct Position {

    /** Used to identify the Position. */
    using Index = I;

    /** The type used to uniquely identifier this Position. */
    using Key = Details::Key<Index>;

    /** Uniquely identifies this Position. */
    Key m_key;

    /** The quantity of inventory held. */
    Quantity m_quantity;

    /** The total cost of the currently held inventory. */
    Money m_costBasis;

    bool operator ==(const Position& position) const = default;
  };

  template<typename Index>
  std::ostream& operator <<(std::ostream& out,
      const Position<Index>& position) {
    return out << '(' << position.m_key << ' ' << position.m_quantity << ' ' <<
      position.m_costBasis << ')';
  }

  /**
   * Returns the average price of a Position.
   * @param position The Position to measure.
   * @return The average price of the <i>position</i>.
   */
  template<typename I>
  Money GetAveragePrice(const Position<I>& position) {
    if(position.m_quantity == 0) {
      return Money::ZERO;
    }
    return position.m_costBasis / position.m_quantity;
  }

  /**
   * Returns the Position's Side.
   * @param position The Position to measure.
   * @return The Side corresponding to the <i>position</i>.
   */
  template<typename I>
  Side GetSide(const Position<I>& position) {
    if(position.m_quantity == 0) {
      return Side::NONE;
    } else if(position.m_quantity > 0) {
      return Side::BID;
    }
    return Side::ASK;
  }
}

namespace Beam::Serialization {
  template<typename I>
  struct Shuttle<Nexus::Accounting::Details::Key<I>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::Accounting::Details::Key<I>& value, unsigned int version) {
      shuttle.Shuttle("index", value.m_index);
      shuttle.Shuttle("currency", value.m_currency);
    }
  };

  template<typename I>
  struct Shuttle<Nexus::Accounting::Position<I>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::Accounting::Position<I>& value, unsigned int version) {
      shuttle.Shuttle("key", value.m_key);
      shuttle.Shuttle("quantity", value.m_quantity);
      shuttle.Shuttle("cost_basis", value.m_costBasis);
    }
  };
}

namespace std {
  template <typename I>
  struct hash<Nexus::Accounting::Details::Key<I>> {
    size_t operator()(const Nexus::Accounting::Details::Key<I>& value) const {
      std::size_t seed = 0;
      boost::hash_combine(seed, value.m_index);
      boost::hash_combine(seed, value.m_currency);
      return seed;
    }
  };
}

#endif
