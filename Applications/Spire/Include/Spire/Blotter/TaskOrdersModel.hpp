#ifndef SPIRE_TASK_ORDERS_MODEL_HPP
#define SPIRE_TASK_ORDERS_MODEL_HPP
#include <unordered_map>
#include "Spire/Async/QtTaskQueue.hpp"
#include "Spire/Blotter/BlotterTaskEntry.hpp"
#include "Spire/Blotter/OrdersModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

namespace Spire {

  /** Collects the orders published by tasks, deduplicated by ID. */
  class TaskOrdersModel : public OrdersModel {
    public:

      /**
       * Constructs a TaskOrdersModel.
       * @param tasks The tasks whose orders are collected.
       */
      explicit TaskOrdersModel(std::shared_ptr<TasksModel> tasks);

      ~TaskOrdersModel() override;

      /** Returns the source tasks. */
      const std::shared_ptr<TasksModel>& get_tasks() const;

      int get_size() const override;
      const Type& get(int index) const override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      struct TaskEntry;
      std::shared_ptr<TasksModel> m_tasks;
      ArrayListModel<Type> m_orders;
      std::unordered_map<Nexus::OrderId, int> m_counts;
      std::vector<std::shared_ptr<TaskEntry>> m_entries;
      ListModelTransactionLog<Type> m_transaction;
      QtTaskQueue m_events;
      boost::signals2::scoped_connection m_orders_connection;
      boost::signals2::scoped_connection m_tasks_connection;

      void add(int index, std::shared_ptr<Task> task);
      void withdraw(const TaskEntry& entry);
      void on_order(TaskEntry& entry, const Type& order);
      void on_operation(const TasksModel::Operation& operation);
  };
}

#endif
