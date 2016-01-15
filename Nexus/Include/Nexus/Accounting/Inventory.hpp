#ifndef NEXUS_INVENTORY_HPP
#define NEXUS_INVENTORY_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/Accounting/Position.hpp"

namespace Nexus {
namespace Accounting {

  /*! \struct Inventory
      \brief Stores bookkeeping info for a single inventory.
      \tparam PositionType The type used to manage inventory Positions.
   */
  template<typename PositionType>
  struct Inventory {

    //! The type used to manage inventory Positions.
    typedef PositionType Position;

    //! The currently held Position.
    Position m_position;

    //! The Inventory's gross profit and loss.
    Money m_grossProfitAndLoss;

    //! The transaction fees.
    Money m_fees;

    //! The total quantity that was transacted.
    Quantity m_volume;

    //! The number of transactions made.
    int m_transactionCount;

    //! Constructs an empty Inventory.
    Inventory();

    //! Constructs an Inventory.
    /*!
      \param key The Key uniquely identifying this Inventory.
    */
    Inventory(const typename Position::Key& key);
  };

  template<typename PositionType>
  Inventory<PositionType>::Inventory()
      : m_fees(Money::ZERO),
        m_volume(0),
        m_transactionCount(0) {}

  template<typename PositionType>
  Inventory<PositionType>::Inventory(const typename PositionType::Key& key)
      : m_position(key),
        m_fees(Money::ZERO),
        m_volume(0),
        m_transactionCount(0) {}
}
}

namespace Beam {
namespace Serialization {
  template<typename PositionType>
  struct Shuttle<Nexus::Accounting::Inventory<PositionType>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::Accounting::Inventory<PositionType>& value,
        unsigned int version) {
      shuttle.Shuttle("position", value.m_position);
      shuttle.Shuttle("gross_profit_and_loss", value.m_grossProfitAndLoss);
      shuttle.Shuttle("fees", value.m_fees);
      shuttle.Shuttle("volume", value.m_volume);
      shuttle.Shuttle("transaction_count", value.m_transactionCount);
    }
  };
}
}

#endif
