#ifndef SPIRE_PORTFOLIO_MODEL_HPP
#define SPIRE_PORTFOLIO_MODEL_HPP
#include <unordered_map>
#include <vector>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/RiskService/InventorySnapshot.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Spire/QtTaskQueue.hpp"

namespace Spire {

  /** Updates a portfolio based on a list of orders and a valuation. */
  class PortfolioModel {
    public:

      /** The type of portfolio used. */
      using Portfolio = Nexus::Accounting::Portfolio<
        Nexus::Accounting::TrueAverageBookkeeper<Nexus::Accounting::Inventory<
          Nexus::Accounting::Position<Nexus::Security>>>>;

      /**
       * Signals an update to a position.
       * @param entry The updated position entry.
       */
      using UpdateSignal = Signal<void (const Portfolio::UpdateEntry& update)>;

      /**
       * Constructs a PositionsModel from an initial snapshot updated by a
       * list of orders and using a given valuation for the profit and loss.
       * @param markets The MarketDatabase used to lookup currencies.
       * @param snapshot The initial position snapshot.
       * @param orders The list of orders used to update the set of positions.
       * @param valuation The valuation used for the unrealized profit and loss.
       */
      PortfolioModel(Nexus::MarketDatabase markets,
        const Nexus::RiskService::InventorySnapshot& snapshot,
        std::shared_ptr<OrderListModel> orders,
        std::shared_ptr<ValuationModel> valuation);

      /** Returns a snapshot of the portfolio. */
      const Portfolio& get_portfolio() const;

      /** Connects a slot to the update signal. */
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const;

    private:
      mutable UpdateSignal m_update_signal;
      Nexus::MarketDatabase m_markets;
      std::shared_ptr<OrderListModel> m_orders;
      std::shared_ptr<ValuationModel> m_valuation;
      std::unordered_map<Nexus::Security, boost::signals2::scoped_connection>
        m_valuation_connections;
      Nexus::Accounting::Portfolio<Nexus::Accounting::TrueAverageBookkeeper<
        Nexus::Accounting::Inventory<Nexus::Accounting::Position<
          Nexus::Security>>>> m_portfolio;
      QtTaskQueue m_tasks;
      boost::signals2::scoped_connection m_orders_connection;

      PortfolioModel(const PortfolioModel&) = delete;
      PortfolioModel& operator =(const PortfolioModel&) = delete;
      void signal_update(const Nexus::Security& security);
      void on_report(const Nexus::OrderExecutionService::Order& order,
        const Nexus::OrderExecutionService::ExecutionReport& report);
      void on_valuation(const Nexus::Security& security,
        const Nexus::Accounting::SecurityValuation& valuation);
      void on_operation(const OrderListModel::Operation& operation);
  };
}

#endif
