#ifndef SPIRE_POSITIONS_MODEL_HPP
#define SPIRE_POSITIONS_MODEL_HPP
#include <vector>
#include "Nexus/RiskService/InventorySnapshot.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"

namespace Spire {

  /** Represents a set of open positions. */
  class PositionsModel {
    public:

      /** Stores a position and its profit and loss. */
      struct Entry {

        /** The entry's position. */
        Nexus::Accounting::Position<Nexus::Security> m_position;

        /** The position's profit and loss. */
        Nexus::Money m_profit_and_loss;
      };

      /**
       * Signals an update to a position.
       * @param entry The updated position entry.
       */
      using UpdateSignal = Signal<void (const Entry& entry)>;

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

      /** Returns a snapshot of all open positions. */
      std::vector<Entry> get_positions() const;

      /** Connects a slot to the update signal. */
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const;

    private:
      mutable UpdateSignal m_update_signal;

      PositionsModel(const PositionsModel&) = delete;
      PositionsModel& operator =(const PositionsModel&) = delete;
  };
}

#endif
