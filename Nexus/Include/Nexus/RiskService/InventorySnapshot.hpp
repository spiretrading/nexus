#ifndef NEXUS_INVENTORY_SNAPSHOT_HPP
#define NEXUS_INVENTORY_SNAPSHOT_HPP
#include <vector>
#include <Beam/Queries/Sequence.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus::RiskService {

  /** Stores a snapshot of inventories. */
  struct InventorySnapshot {

    /** The list of non-empty inventories. */
    std::vector<RiskInventory> m_inventories;

    /** The sequence that this snapshot is valid for (inclusive). */
    Beam::Queries::Sequence m_sequence;

    /** The list of Order ids excluded from this snapshot. */
    std::vector<OrderExecutionService::OrderId> m_excludedOrders;

    /**
     * Tests if two snapshots have identical structure, including order of
     * inventories and excluded orders.
     */
    bool operator ==(const InventorySnapshot& snapshot) const;

    /** Tests if two snapshots have different structures. */
    bool operator !=(const InventorySnapshot& snapshot) const;
  };

  inline bool InventorySnapshot::operator ==(
      const InventorySnapshot& snapshot) const {
    return m_inventories == snapshot.m_inventories &&
      m_sequence == snapshot.m_sequence &&
      m_excludedOrders == snapshot.m_excludedOrders;
  }

  inline bool InventorySnapshot::operator !=(
      const InventorySnapshot& snapshot) const {
    return !(*this == snapshot);
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::RiskService::InventorySnapshot> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::RiskService::InventorySnapshot& value, unsigned int version) {
      shuttle.Shuttle("inventories", value.m_inventories);
      shuttle.Shuttle("sequence", value.m_sequence);
      shuttle.Shuttle("excluded_orders", value.m_excludedOrders);
    }
  };
}

#endif
