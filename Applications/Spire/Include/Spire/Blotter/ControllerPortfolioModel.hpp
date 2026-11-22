#ifndef SPIRE_CONTROLLER_PORTFOLIO_MODEL_HPP
#define SPIRE_CONTROLLER_PORTFOLIO_MODEL_HPP
#include <unordered_map>
#include "Nexus/Accounting/PortfolioController.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Spire/Async/QtTaskQueue.hpp"
#include "Spire/Blotter/OrdersModel.hpp"
#include "Spire/Blotter/PortfolioModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

namespace Spire {

  /** Maintains portfolio entries from a portfolio controller. */
  class ControllerPortfolioModel : public PortfolioModel {
    public:

      /** The controller supplying portfolio entries. */
      using Controller = Nexus::PortfolioController<
        Nexus::Portfolio<Nexus::TrueAverageBookkeeper>,
        Nexus::MarketDataClient>;

      /**
       * Constructs a ControllerPortfolioModel.
       * @param controller The controller whose portfolio to model.
       */
      explicit ControllerPortfolioModel(
        std::shared_ptr<Controller> controller);

      /**
       * Builds a portfolio from an order list.
       * @param orders The orders to include in the portfolio.
       * @param client Supplies market prices for valuation.
       */
      ControllerPortfolioModel(
        std::shared_ptr<OrdersModel> orders, Nexus::MarketDataClient client);

      /**
       * Builds a portfolio from a snapshot and order list.
       * @param orders The orders to apply to the snapshot.
       * @param initial The snapshot, excluding the supplied orders.
       * @param client Supplies market prices for valuation.
       */
      ControllerPortfolioModel(std::shared_ptr<OrdersModel> orders,
        Controller::Portfolio initial, Nexus::MarketDataClient client);

      ~ControllerPortfolioModel() override;

      int get_size() const override;
      const Type& get(int index) const override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;
      void start_transaction() override;
      void end_transaction() override;

    private:
      struct Source;
      std::shared_ptr<Controller> m_controller;
      std::unique_ptr<Source> m_source;
      ArrayListModel<Type> m_entries;
      std::unordered_map<Nexus::Ticker, int> m_indices;
      QtTaskQueue m_events;
      boost::signals2::scoped_connection m_orders_connection;

      void monitor();
      void rebuild();
      void flush();
      void on_orders(const OrdersModel::Operation& operation);
      void on_update(const Nexus::PortfolioUpdateEntry& update);
  };
}

#endif
