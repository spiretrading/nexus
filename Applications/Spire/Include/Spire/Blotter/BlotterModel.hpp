#ifndef SPIRE_BLOTTER_MODEL_HPP
#define SPIRE_BLOTTER_MODEL_HPP
#include <memory>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/TextBox.hpp"

namespace Spire {

  /** Stores a Task and the auxilliary data needed to view it in a blotter. */
  struct TaskEntry {

    /** The Task represented by this entry. */
    std::shared_ptr<Task> m_task;

    /** Whether to pin this entry to the blotter. */
    std::shared_ptr<BooleanModel> m_is_pinned;
  };

  /** The type of list model used for orders displayed in the blotter. */
  using OrderListModel = ListModel<Nexus::OrderExecutionService::Order*>;

  /** The type of list model used for tasks displayed in the blotter. */
  using TaskListModel = ListModel<TaskEntry>;

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
      virtual std::shared_ptr<TaskListModel> get_tasks() = 0;

      /** Returns the blotter's positions. */
      virtual std::shared_ptr<BlotterPositionsModel> get_positions() = 0;

      /** Returns the blotter's orders. */
      virtual std::shared_ptr<OrderListModel> get_orders() = 0;

      /** Returns the blotter's profit and loss. */
      virtual std::shared_ptr<BlotterProfitAndLossModel> get_profit_and_loss() =
        0;

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
