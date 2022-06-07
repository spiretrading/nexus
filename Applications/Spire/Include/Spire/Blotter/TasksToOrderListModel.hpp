#ifndef SPIRE_TASKS_TO_ORDER_LIST_MODEL_HPP
#define SPIRE_TASKS_TO_ORDER_LIST_MODEL_HPP
#include <unordered_map>
#include <unordered_set>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/QtTaskQueue.hpp"

namespace Spire {

  /**
   * Implements a ListModel listing out all distinct orders published by a
   * specified list of tasks.
   */
  class TasksToOrderListModel : public OrderListModel {
    public:

      /**
       * Constructs a TasksToOrderListModel.
       * @param tasks The list of tasks producing the orders that this model is
       *        to publish.
       */
      explicit TasksToOrderListModel(
        std::shared_ptr<BlotterTaskListModel> tasks);

      int get_size() const override;

      const Type& get(int index) const override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      QtTaskQueue m_queue;
      std::shared_ptr<BlotterTaskListModel> m_tasks;
      std::shared_ptr<
        ArrayListModel<const Nexus::OrderExecutionService::Order*>> m_orders;
      std::unordered_map<const BlotterTaskEntry*, std::unordered_set<
        const Nexus::OrderExecutionService::Order*>> m_task_to_orders;
      std::unordered_map<const Nexus::OrderExecutionService::Order*, int>
        m_order_to_count;
      boost::signals2::scoped_connection m_connection;

      void add(const std::shared_ptr<BlotterTaskEntry>& entry);
      void remove(const std::shared_ptr<BlotterTaskEntry>& entry);
      void on_order(const BlotterTaskEntry& entry,
        const Nexus::OrderExecutionService::Order* order);
      void on_operation(const BlotterTaskListModel::Operation& operation);
  };
}

#endif
