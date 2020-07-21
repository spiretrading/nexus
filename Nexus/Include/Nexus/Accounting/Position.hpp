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

    /** Constructs an empty Key. */
    Key() = default;

    /**
     * Constructs a Key.
     * @param index The inventory's index.
     * @param currency The Currency used to value the inventory.
     */
    Key(const Index& index, CurrencyId currency);

    /**
     * Tests two Keys for equality.
     * @param key The Key to test for equality.
     * @return <code>true</code> iff <i>key</i> has the same Index and Currency.
     */
    bool operator ==(const Key& key) const;

    /**
     * Tests two Keys for inequality.
     * @param key The Key to test for inequality.
     * @return <code>true</code> iff <i>key</i> has a different Index or
     *         Currency.
     */
    bool operator !=(const Key& key) const;
  };

  template<typename I>
  Key<I>::Key(const Index& index, CurrencyId currency)
    : m_index(index),
      m_currency{currency} {}

  template<typename I>
  bool Key<I>::operator ==(const Key& key) const {
    return m_index == key.m_index && m_currency == key.m_currency;
  }

  template<typename I>
  bool Key<I>::operator !=(const Key& key) const {
    return !(*this == key);
  }

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

    /** Constructs a default Position. */
    Position();

    /** Constructs a Position. */
    explicit Position(const Key& key);

    /** Tests if two Positions are equal */
    bool operator ==(const Position& position) const;

    /** Tests if two Positions are not equal. */
    bool operator !=(const Position& position) const;
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

  template<typename I>
  Position<I>::Position()
    : m_quantity(0) {}

  template<typename I>
  Position<I>::Position(const Key& key)
    : m_quantity(0),
      m_key(key) {}

  template<typename I>
  bool Position<I>::operator ==(const Position& position) const {
    return m_key == position.m_key && m_quantity == position.m_quantity &&
      m_costBasis == position.m_costBasis;
  }

  template<typename I>
  bool Position<I>::operator !=(const Position& position) const {
    return !(*this == position);
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
