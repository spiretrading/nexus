#ifndef NEXUS_INVENTORY_HPP
#define NEXUS_INVENTORY_HPP
#include <ostream>
#include <utility>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/Accounting/Position.hpp"

namespace Nexus::Accounting {

  /**
   * Stores bookkeeping info for a single inventory.
   * @param <P> The type used to manage inventory Positions.
   */
  template<IsPosition P>
  struct Inventory {

    /** The type used to manage inventory Positions. */
    using Position = P;

    /** The currently held Position. */
    Position m_position;

    /** The Inventory's gross profit and loss. */
    Money m_gross_profit_and_loss;

    /** The transaction fees. */
    Money m_fees;

    /** The total quantity that was transacted. */
    Quantity m_volume;

    /** The number of transactions made. */
    int m_transaction_count;

    /** Constructs an empty Inventory. */
    Inventory() noexcept;

    /**
     * Constructs an Inventory.
     * @param key The Key uniquely identifying this Inventory.
     */
    explicit Inventory(typename Position::Key key) noexcept;

    /**
     * Constructs an Inventory.
     * @param position The currently held Position.
     * @param gross_profit_and_loss The Inventory's gross profit and loss.
     * @param fees The transaction fees.
     * @param volume The total quantity that was transacted.
     * @param transaction_count The number of transactions made.
     */
    Inventory(Position position, Money gross_profit_and_loss, Money fees,
      Quantity volume, int transaction_count);

    bool operator ==(const Inventory&) const = default;
  };

  /**
   * Concept that evaluates to true if a type is an Inventory instantiation.
   * @param <T> The type to test.
   */
  template<typename T>
  concept IsInventory = Beam::is_instance_v<T, Inventory>;

  /**
  * Tests if an Inventory is empty, ie. has no position, volume, or
  * transactions.
  * @param inventory The inventory to test.
  * @return <code>true</code> iff the <i>inventory</i> is empty.
  */
  template<typename P>
  bool is_empty(const Inventory<P>& inventory) {
    return inventory == Inventory<P>(inventory.m_position.m_key);
  }

  template<IsPosition P>
  Inventory<P>::Inventory() noexcept
    : m_volume(0),
      m_transaction_count(0) {}

  template<IsPosition P>
  Inventory<P>::Inventory(typename P::Key key) noexcept
    : m_position(std::move(key)),
      m_volume(0),
      m_transaction_count(0) {}

  template<IsPosition P>
  Inventory<P>::Inventory(Position position, Money gross_profit_and_loss,
    Money fees, Quantity volume, int transaction_count)
    : m_position(std::move(position)),
      m_gross_profit_and_loss(gross_profit_and_loss),
      m_fees(fees),
      m_volume(volume),
      m_transaction_count(transaction_count) {}

  template<typename Position>
  std::ostream& operator <<(
      std::ostream& out, const Inventory<Position>& inventory) {
    return out << '(' << inventory.m_position << ' ' <<
      inventory.m_gross_profit_and_loss << ' ' << inventory.m_fees << ' ' <<
      inventory.m_volume << ' ' << inventory.m_transaction_count << ')';
  }
}

namespace Beam::Serialization {
  template<typename P>
  struct Shuttle<Nexus::Accounting::Inventory<P>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::Accounting::Inventory<P>& value,
        unsigned int version) const {
      shuttle.Shuttle("position", value.m_position);
      shuttle.Shuttle("gross_profit_and_loss", value.m_gross_profit_and_loss);
      shuttle.Shuttle("fees", value.m_fees);
      shuttle.Shuttle("volume", value.m_volume);
      shuttle.Shuttle("transaction_count", value.m_transaction_count);
    }
  };
}

#endif
