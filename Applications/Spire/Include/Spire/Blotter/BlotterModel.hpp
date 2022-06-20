#ifndef SPIRE_BLOTTER_MODEL_HPP
#define SPIRE_BLOTTER_MODEL_HPP
#include <memory>
#include <string>
#include <Beam/Queues/Publisher.hpp>
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/OrderExecutionService/ExecutionReportPublisher.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Canvas/Task.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/TextBox.hpp"

namespace Spire {

  /** Stores a Task and the auxilliary data needed to view it in a blotter. */
  struct BlotterTaskEntry {

    /** The entry's unique id. */
    int m_id;

    /** The name of the task. */
    std::string m_name;

    /** Whether to pin this entry to the blotter. */
    bool m_is_pinned;

    /** The Task represented by this entry. */
    std::unique_ptr<Task> m_task;

    /** Publishes the orders submitted by this task. */
    std::shared_ptr<Beam::SequencePublisher<
      const Nexus::OrderExecutionService::Order*>> m_orders;
  };

  /** The type of list model used for tasks displayed in the blotter. */
  using BlotterTaskListModel = ListModel<std::shared_ptr<BlotterTaskEntry>>;

  /** The type of list model used for orders displayed in the blotter. */
  using OrderListModel = ListModel<const Nexus::OrderExecutionService::Order*>;

  /**
   * The type of list model used for execution reports displayed in the blotter.
   */
  using ExecutionReportListModel =
    ListModel<Nexus::OrderExecutionService::ExecutionReportEntry>;

  /**
   * Defines the interface representing a blotter, used to keep track of trading
   * activity.
   */
  class BlotterModel {
    public:
      virtual ~BlotterModel() = default;

      /** Returns the name of this blotter. */
      virtual std::shared_ptr<TextModel> get_name() = 0;

      /** Whether this is the application's active blotter. */
      virtual std::shared_ptr<BooleanModel> is_active() = 0;

      /** Whether this blotter persists after being closed. */
      virtual std::shared_ptr<BooleanModel> is_pinned() = 0;

      /** Returns the market database to use. */
      virtual const Nexus::MarketDatabase& get_markets() const = 0;

      /** Returns the blotter's primary currency. */
      virtual Nexus::CurrencyId get_currency() const = 0;

      /**
       * Returns the table of exchange rates used to convert currencies to the
       * blotter's primary currency.
       */
      virtual const Nexus::ExchangeRateTable& get_exchange_rates() const = 0;

      /** Returns the account's buying power. */
      virtual std::shared_ptr<MoneyModel> get_buying_power() = 0;

      /** Returns the account's allowable net loss. */
      virtual std::shared_ptr<MoneyModel> get_net_loss() = 0;

      /** Returns this blotter's tasks. */
      virtual std::shared_ptr<BlotterTaskListModel> get_tasks() = 0;

      /** Returns the list of selected tasks. */
      virtual std::shared_ptr<ListModel<int>> get_task_selection() const = 0;

      /** Returns the valuations used by this blotter. */
      virtual std::shared_ptr<ValuationModel> get_valuation() = 0;

      /** Returns the blotter's profit and loss. */
      virtual std::shared_ptr<BlotterProfitAndLossModel>
        get_profit_and_loss() = 0;

    protected:

      /** Constructs a BlotterModel. */
      BlotterModel() = default;

    private:
      BlotterModel(const BlotterModel&) = delete;
      BlotterModel& operator =(const BlotterModel&) = delete;
  };
}

#endif
