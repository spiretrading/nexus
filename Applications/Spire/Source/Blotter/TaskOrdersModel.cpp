#include "Spire/Blotter/TaskOrdersModel.hpp"
#include <algorithm>
#include <unordered_set>
#include "Spire/Async/EventHandler.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

struct TaskOrdersModel::TaskEntry {
  std::shared_ptr<Task> m_task;
  std::unordered_set<OrderId> m_orders;
  EventHandler m_events;

  TaskEntry(std::shared_ptr<Task> task)
    : m_task(std::move(task)) {}
};

TaskOrdersModel::TaskOrdersModel(std::shared_ptr<TasksModel> tasks)
    : m_tasks(std::move(tasks)) {
  m_orders_connection = m_orders.connect_operation_signal(
    [=, this] (const auto& operation) { m_transaction.push(operation); });
  for(auto i = 0; i != m_tasks->get_size(); ++i) {
    add(i, m_tasks->get(i).m_task);
  }
  m_tasks_connection = m_tasks->connect_operation_signal(
    std::bind_front(&TaskOrdersModel::on_operation, this));
}

TaskOrdersModel::~TaskOrdersModel() = default;

const std::shared_ptr<TasksModel>& TaskOrdersModel::get_tasks() const {
  return m_tasks;
}

int TaskOrdersModel::get_size() const {
  return m_orders.get_size();
}

const TaskOrdersModel::Type& TaskOrdersModel::get(int index) const {
  return m_orders.get(index);
}

connection TaskOrdersModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void TaskOrdersModel::transact(
    const std::function<void ()>& transaction) {
  m_transaction.transact(transaction);
}

void TaskOrdersModel::add(int index, std::shared_ptr<Task> task) {
  auto entry = std::make_unique<TaskEntry>(std::move(task));
  auto task_entry = entry.get();
  m_entries.insert(m_entries.begin() + index, std::move(entry));
  if(!task_entry->m_task) {
    return;
  }
  auto snapshot = optional<std::vector<Type>>();
  task_entry->m_task->GetContext().GetOrderPublisher().monitor(
    task_entry->m_events.get_slot<Type>([=, this] (const auto& order) {
      on_order(*task_entry, order);
    }), out(snapshot));
  if(snapshot) {
    for(auto& order : *snapshot) {
      on_order(*task_entry, order);
    }
  }
}

void TaskOrdersModel::withdraw(const TaskEntry& entry) {
  for(auto i = m_orders.get_size() - 1; i >= 0; --i) {
    auto id = m_orders.get(i)->get_info().m_id;
    if(entry.m_orders.contains(id)) {
      auto count = m_counts.find(id);
      --count->second;
      if(count->second == 0) {
        m_counts.erase(count);
        m_orders.remove(i);
      }
    }
  }
}

void TaskOrdersModel::on_order(TaskEntry& entry, const Type& order) {
  if(!order || !entry.m_orders.insert(order->get_info().m_id).second) {
    return;
  }
  auto& count = m_counts[order->get_info().m_id];
  ++count;
  if(count == 1) {
    m_orders.push(order);
  }
}

void TaskOrdersModel::on_operation(const TasksModel::Operation& operation) {
  visit(operation,
    [&] (const TasksModel::StartTransaction&) { m_transaction.start(); },
    [&] (const TasksModel::EndTransaction&) { m_transaction.end(); },
    [&] (const TasksModel::AddOperation& operation) {
      m_transaction.transact([&] {
        add(operation.m_index, m_tasks->get(operation.m_index).m_task);
      });
    },
    [&] (const TasksModel::MoveOperation& operation) {
      auto first = m_entries.begin() + operation.m_source;
      auto last = m_entries.begin() + operation.m_destination;
      if(first < last) {
        std::ranges::rotate(first, first + 1, last + 1);
      } else {
        std::ranges::rotate(last, first, first + 1);
      }
    },
    [&] (const TasksModel::RemoveOperation& operation) {
      auto entry = std::move(m_entries[operation.m_index]);
      m_entries.erase(m_entries.begin() + operation.m_index);
      m_transaction.transact([&] { withdraw(*entry); });
    },
    [&] (const TasksModel::UpdateOperation& operation) {
      if(operation.get_previous().m_task == operation.get_value().m_task) {
        return;
      }
      m_transaction.transact([&] {
        auto entry = std::move(m_entries[operation.m_index]);
        m_entries.erase(m_entries.begin() + operation.m_index);
        add(operation.m_index, operation.get_value().m_task);
        withdraw(*entry);
      });
    },
    [] (const auto&) {});
}
