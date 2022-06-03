#ifndef SPIRE_BLOTTER_MODEL_HPP
#define SPIRE_BLOTTER_MODEL_HPP
#include <memory>
#include <string>
#include <Beam/Queues/ScopedQueueReader.hpp>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Canvas/Task.hpp"
#include "Spire/Spire/ListModel.hpp"
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
    Beam::ScopedQueueReader<const Nexus::OrderExecutionService::Order*> m_orders;
  };

  /** The type of list model used for orders displayed in the blotter. */
  using OrderListModel = ListModel<Nexus::OrderExecutionService::Order*>;

  /** The type of list model used for tasks displayed in the blotter. */
  using BlotterTaskListModel = ListModel<std::shared_ptr<BlotterTaskEntry>>;

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

      /** Returns this blotter's tasks. */
      virtual std::shared_ptr<BlotterTaskListModel> get_tasks() = 0;

      /** Returns the list of selected tasks. */
      virtual std::shared_ptr<ListModel<int>> get_task_selection() const = 0;

      /** Returns the blotter's positions. */
      virtual std::shared_ptr<BlotterPositionsModel> get_positions() = 0;

      /** Returns the blotter's orders. */
      virtual std::shared_ptr<OrderListModel> get_orders() = 0;

      /** Returns the blotter's profit and loss. */
      virtual std::shared_ptr<BlotterProfitAndLossModel>
        get_profit_and_loss() = 0;

      /** Returns the blotter's status. */
      virtual std::shared_ptr<BlotterStatusModel> get_status() = 0;

    protected:

      /** Constructs a BlotterModel. */
      BlotterModel() = default;

    private:
      BlotterModel(const BlotterModel&) = delete;
      BlotterModel& operator =(const BlotterModel&) = delete;
  };
}

#endif
