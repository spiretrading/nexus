#ifndef SPIRE_POSITIONS_MODEL_HPP
#define SPIRE_POSITIONS_MODEL_HPP
#include "Nexus/RiskService/InventorySnapshot.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"

namespace Spire {

  /** Represents a set of open positions. */
  class PositionsModel {
    public:

      /**
       * Constructs a PositionsModel from an initial snapshot updated by a
       * list of orders and using a given valuation for the profit and loss.
       * @param snapshot The initial position snapshot.
       * @param orders The list of orders used to update the set of positions.
       * @param valuation The valuation used for the unrealized profit and loss.
       */
      PositionsModel(const Nexus::RiskService::InventorySnapshot& snapshot,
        std::shared_ptr<OrderListModel> orders,
        std::shared_ptr<ValuationModel> valuation);

    private:
      PositionsModel(const PositionsModel&) = delete;
      PositionsModel& operator =(const PositionsModel&) = delete;
  };
}

#endif
