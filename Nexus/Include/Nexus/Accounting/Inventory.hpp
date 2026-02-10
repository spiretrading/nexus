#ifndef NEXUS_INVENTORY_HPP
#define NEXUS_INVENTORY_HPP
#include <ostream>
#include <utility>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Accounting/Position.hpp"

namespace Nexus {

  /** Stores bookkeeping info for a single inventory. */
  struct Inventory {

    /** The currently held Position. */
    Position m_position;

    /** The Inventory's gross profit and loss. */
    Money m_gross_profit_and_loss;

    /** The transaction fees. */
    Money m_fees;

    /** The total quantity that was transacted. */
    Quantity m_volume;

    /** The number of transactions made. */
    int m_transaction_count = 0;

    /** Constructs an empty Inventory. */
    Inventory() = default;

    /**
     * Constructs an Inventory.
     * @param ticker The Ticker being managed.
     * @param currency The currency used to value the Inventory.
     */
    Inventory(Ticker ticker, Asset currency) noexcept;

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
  * Tests if an Inventory is empty, ie. has no position, volume, or
  * transactions.
  * @param inventory The inventory to test.
  * @return <code>true</code> iff the <i>inventory</i> is empty.
  */
  inline bool is_empty(const Inventory& inventory) {
    return inventory == Inventory(
      inventory.m_position.m_ticker, inventory.m_position.m_currency);
  }

  inline Inventory::Inventory(Ticker ticker, Asset currency) noexcept
    : m_position(std::move(ticker), currency) {}

  inline Inventory::Inventory(Position position, Money gross_profit_and_loss,
    Money fees, Quantity volume, int transaction_count)
    : m_position(std::move(position)),
      m_gross_profit_and_loss(gross_profit_and_loss),
      m_fees(fees),
      m_volume(volume),
      m_transaction_count(transaction_count) {}

  inline std::ostream& operator <<(
      std::ostream& out, const Inventory& inventory) {
    return out << '(' << inventory.m_position << ' ' <<
      inventory.m_gross_profit_and_loss << ' ' << inventory.m_fees << ' ' <<
      inventory.m_volume << ' ' << inventory.m_transaction_count << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::Inventory> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::Inventory& value, unsigned int version) const {
      shuttle.shuttle("position", value.m_position);
      shuttle.shuttle("gross_profit_and_loss", value.m_gross_profit_and_loss);
      shuttle.shuttle("fees", value.m_fees);
      shuttle.shuttle("volume", value.m_volume);
      shuttle.shuttle("transaction_count", value.m_transaction_count);
    }
  };
}

#endif
