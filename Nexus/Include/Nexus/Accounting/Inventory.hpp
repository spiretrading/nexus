#ifndef NEXUS_INVENTORY_HPP
#define NEXUS_INVENTORY_HPP
#include <ostream>
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
    explicit Inventory(const typename Position::Key& key);
  };

  template<typename P>
  Inventory<P>::Inventory()
    : m_volume(0),
      m_transactionCount(0) {}

  template<typename P>
  Inventory<P>::Inventory(const typename P::Key& key)
    : m_position(key),
      m_volume(0),
      m_transactionCount(0) {}

  template<typename Position>
  std::ostream& operator <<(std::ostream& out,
      const Inventory<Position>& inventory) {
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
