#include "Spire/Blotter/TasksToOrderListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

TasksToOrderListModel::TasksToOrderListModel(
    std::shared_ptr<BlotterTaskListModel> tasks)
    : m_tasks(std::move(tasks)),
      m_orders(std::make_shared<ArrayListModel<const Order*>>()),
      m_connection(m_tasks->connect_operation_signal(
        std::bind_front(&TasksToOrderListModel::on_operation, this))) {
  for(auto i = 0; i != m_tasks->get_size(); ++i) {
    add(m_tasks->get(i));
  }
}

int TasksToOrderListModel::get_size() const {
  return m_orders->get_size();
}

const TasksToOrderListModel::Type& TasksToOrderListModel::get(int index) const {
  return m_orders->get(index);
}

connection TasksToOrderListModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_orders->connect_operation_signal(slot);
}

void TasksToOrderListModel::transact(
    const std::function<void ()>& transaction) {
  transaction();
}

void TasksToOrderListModel::add(
    const std::shared_ptr<BlotterTaskEntry>& entry) {
  entry->m_orders->Monitor(m_queue.get_slot<const Order*>(std::bind_front(
    &TasksToOrderListModel::on_order, this, std::cref(*entry))));
}

void TasksToOrderListModel::remove(
    const std::shared_ptr<BlotterTaskEntry>& entry) {
  auto& orders = m_task_to_orders[entry.get()];
  auto removals = std::unordered_set<const Order*>();
  for(auto& order : orders) {
    auto& count = m_order_to_count[order];
    --count;
    if(count == 0) {
      removals.insert(order);
      m_order_to_count.erase(order);
    }
  }
  m_task_to_orders.erase(entry.get());
  auto removal_count = removals.size();
  if(removal_count == 0) {
    return;
  }
  m_orders->transact([&] {
    for(auto i = m_orders->get_size() - 1; i >= 0; --i) {
      if(removals.contains(m_orders->get(i))) {
        m_orders->remove(i);
        --removal_count;
        if(removal_count == 0) {
          return;
        }
      }
    }
  });
}

void TasksToOrderListModel::on_order(
    const BlotterTaskEntry& entry, const Order* order) {
  m_task_to_orders[&entry].insert(order);
  auto& count = m_order_to_count[order];
  ++count;
  if(count == 1) {
    m_orders->push(order);
  }
}

void TasksToOrderListModel::on_operation(
    const BlotterTaskListModel::Operation& operation) {
  visit(operation,
    [&] (const BlotterTaskListModel::AddOperation& operation) {
      add(operation.get_value());
    },
    [&] (const BlotterTaskListModel::RemoveOperation& operation) {
      remove(operation.get_value());
    });
}
