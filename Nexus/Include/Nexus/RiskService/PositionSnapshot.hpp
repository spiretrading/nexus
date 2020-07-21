#ifndef NEXUS_POSITION_SNAPSHOT_HPP
#define NEXUS_POSITION_SNAPSHOT_HPP
#include <vector>
#include <Beam/Queries/Sequence.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus::RiskService {

  /** Stores a snapshot of Positions. */
  struct PositionSnapshot {

    /** The list of non-empty positions. */
    std::vector<RiskPortfolioPosition> m_positions;

    /** The sequence that this snapshot is valid for (inclusive). */
    Beam::Queries::Sequence m_sequence;

    /** The list of Order ids excluded from this snapshot. */
    std::vector<OrderExecutionService::OrderId> m_excludedOrders;

    /**
     * Tests if two snapshots have identical structure, including order of
     * positions and excluded orders.
     */
    bool operator ==(const PositionSnapshot& snapshot) const;

    /** Tests if two snapshots have different structures. */
    bool operator !=(const PositionSnapshot& snapshot) const;
  };

  inline bool PositionSnapshot::operator ==(
      const PositionSnapshot& snapshot) const {
    return m_positions == snapshot.m_positions &&
      m_sequence == snapshot.m_sequence &&
      m_excludedOrders == snapshot.m_excludedOrders;
  }

  inline bool PositionSnapshot::operator !=(
      const PositionSnapshot& snapshot) const {
    return !(*this == snapshot);
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::RiskService::PositionSnapshot> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::RiskService::PositionSnapshot& value, unsigned int version) {
      shuttle.Shuttle("positions", value.m_positions);
      shuttle.Shuttle("sequence", value.m_sequence);
      shuttle.Shuttle("excluded_orders", value.m_excludedOrders);
    }
  };
}

#endif
