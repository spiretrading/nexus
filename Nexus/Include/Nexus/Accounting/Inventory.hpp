#ifndef NEXUS_INVENTORY_HPP
#define NEXUS_INVENTORY_HPP
#include <ostream>
#include <utility>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/Accounting/Position.hpp"

namespace Nexus::Accounting {

  /**
   * Stores bookkeeping info for a single inventory.
   * @param <P> The type used to manage inventory Positions.
   */
  template<typename P>
  struct Inventory {

    /** The type used to manage inventory Positions. */
    using Position = P;

    /** The currently held Position. */
    Position m_position;

    /** The Inventory's gross profit and loss. */
    Money m_grossProfitAndLoss;

    /** The transaction fees. */
    Money m_fees;

    /** The total quantity that was transacted. */
    Quantity m_volume;

    /** The number of transactions made. */
    int m_transactionCount;

    /** Constructs an empty Inventory. */
    Inventory();

    /**
     * Constructs an Inventory.
     * @param key The Key uniquely identifying this Inventory.
     */
    explicit Inventory(typename Position::Key key);

    /**
     * Constructs an Inventory.
     * @param position The currently held Position.
     * @param grossProfitAndLoss The Inventory's gross profit and loss.
     * @param fees The transaction fees.
     * @param volume The total quantity that was transacted.
     * @param transactionCount The number of transactions made.
     */
    Inventory(Position position, Money grossProfitAndLoss, Money fees,
      Quantity volume, int transactionCount);

    bool operator ==(const Inventory& inventory) const = default;
  };

  /**
  * Tests if an Inventory is empty, ie. has no position, volume, or
  * transactions.
  * @param inventory The inventory to test.
  * @return <code>true</code> iff the <i>inventory</i> is empty.
  */
  template<typename P>
  bool IsEmpty(const Inventory<P>& inventory) {
    return inventory == Inventory<P>(inventory.m_position.m_key);
  }

  template<typename P>
  Inventory<P>::Inventory()
    : m_volume(0),
      m_transactionCount(0) {}

  template<typename P>
  Inventory<P>::Inventory(typename P::Key key)
    : m_position(std::move(key)),
      m_volume(0),
      m_transactionCount(0) {}

  template<typename P>
  Inventory<P>::Inventory(Position position, Money grossProfitAndLoss,
    Money fees, Quantity volume, int transactionCount)
    : m_position(std::move(position)),
      m_grossProfitAndLoss(grossProfitAndLoss),
      m_fees(fees),
      m_volume(volume),
      m_transactionCount(transactionCount) {}

  template<typename Position>
  std::ostream& operator <<(
      std::ostream& out, const Inventory<Position>& inventory) {
    return out << '(' << inventory.m_position << ' ' <<
      inventory.m_grossProfitAndLoss << ' ' << inventory.m_fees << ' ' <<
      inventory.m_volume << ' ' << inventory.m_transactionCount << ')';
  }
}

namespace Beam::Serialization {
  template<typename P>
  struct Shuttle<Nexus::Accounting::Inventory<P>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::Accounting::Inventory<P>& value,
        unsigned int version) {
      shuttle.Shuttle("position", value.m_position);
      shuttle.Shuttle("gross_profit_and_loss", value.m_grossProfitAndLoss);
      shuttle.Shuttle("fees", value.m_fees);
      shuttle.Shuttle("volume", value.m_volume);
      shuttle.Shuttle("transaction_count", value.m_transactionCount);
    }
  };
}

#endif
