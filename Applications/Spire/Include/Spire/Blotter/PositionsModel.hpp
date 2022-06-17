#ifndef SPIRE_POSITIONS_MODEL_HPP
#define SPIRE_POSITIONS_MODEL_HPP
#include <unordered_map>
#include <vector>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/RiskService/InventorySnapshot.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Spire/QtTaskQueue.hpp"

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
       * @param markets The MarketDatabase used to lookup currencies.
       * @param snapshot The initial position snapshot.
       * @param orders The list of orders used to update the set of positions.
       * @param valuation The valuation used for the unrealized profit and loss.
       */
      PositionsModel(Nexus::MarketDatabase markets,
        const Nexus::RiskService::InventorySnapshot& snapshot,
        std::shared_ptr<OrderListModel> orders,
        std::shared_ptr<ValuationModel> valuation);

      /** Returns a snapshot of all open positions. */
      std::vector<Entry> get_positions() const;

      /** Connects a slot to the update signal. */
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const;

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<OrderListModel> m_orders;
      std::shared_ptr<ValuationModel> m_valuation;
      std::unordered_map<Nexus::Security, Entry> m_positions;
      std::unordered_map<Nexus::Security, boost::signals2::scoped_connection>
        m_valuation_connections;
      Nexus::Accounting::Portfolio<Nexus::Accounting::TrueAverageBookkeeper<
        Nexus::Accounting::Inventory<Nexus::Accounting::Position<
          Nexus::Security>>>> m_portfolio;
      QtTaskQueue m_tasks;

      PositionsModel(const PositionsModel&) = delete;
      PositionsModel& operator =(const PositionsModel&) = delete;
      void on_report(const Nexus::OrderExecutionService::Order& order,
        const Nexus::OrderExecutionService::ExecutionReport& report);
      void on_valuation(const Nexus::Security& security,
        const Nexus::Accounting::SecurityValuation& valuation);
  };
}

#endif
