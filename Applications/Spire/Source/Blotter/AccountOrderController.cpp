#include "Spire/Blotter/AccountOrderController.hpp"
#include <algorithm>
#include <utility>
#include "Spire/Canvas/OrderExecutionNodes/OrderWrapperTaskNode.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

AccountOrderController::AccountOrderController(
    BlotterController& controller, std::shared_ptr<TasksModel> tasks,
    const std::shared_ptr<AccountOrdersModel>& account_orders,
    UserProfile& profile)
    : m_controller(&controller),
      m_tasks(std::move(tasks)),
      m_profile(&profile) {
  auto orders = controller.get_model()->get_orders();
  m_order_connection = orders->connect_operation_signal(
    std::bind_front(&AccountOrderController::on_order_operation, this));
  m_live_connection = account_orders->connect_live_signal(
    std::bind_front(&AccountOrderController::on_live_order, this));
  auto source = controller.get_model()->get_tasks();
  m_task_connection = source->connect_operation_signal(
    std::bind_front(&AccountOrderController::on_task_operation, this));
  for(auto i = 0; i != source->get_size(); ++i) {
    monitor(i);
  }
  for(auto& order : std::as_const(*orders)) {
    on_order(order);
  }
}

void AccountOrderController::recover(const std::shared_ptr<Order>& order) {
  auto id = order->get_info().m_id;
  auto wrapper = m_wrappers.find(id);
  for(auto& entry : std::as_const(*m_controller->get_model()->get_tasks())) {
    if(wrapper != m_wrappers.end() && entry.m_task == wrapper->second) {
      continue;
    }
    auto is_owner = entry.m_task->GetContext().GetOrderPublisher().with(
      [&] (const auto& orders) {
        return orders && std::ranges::any_of(
          *orders, [&] (const auto& candidate) {
            return candidate->get_info().m_id == id;
          });
      });
    if(!is_owner) {
      continue;
    }
    if(wrapper != m_wrappers.end()) {
      auto task = wrapper->second;
      m_wrappers.erase(wrapper);
      auto i = std::ranges::find(
        std::as_const(*m_tasks), task, &BlotterTaskEntry::m_task);
      if(i != m_tasks->cend()) {
        m_tasks->remove(i);
      }
    }
    return;
  }
  if(wrapper != m_wrappers.end()) {
    return;
  }
  auto is_terminal_order = order->get_publisher().with(
    [] (const auto& reports) {
      return reports && !reports->empty() &&
        is_terminal(reports->back().m_status);
    });
  if(is_terminal_order) {
    return;
  }
  auto task = m_controller->add(OrderWrapperTaskNode(order, *m_profile));
  m_wrappers.emplace(id, task);
  task->Execute();
}

void AccountOrderController::monitor(int index) {
  auto task = m_controller->get_model()->get_tasks()->get(index).m_task;
  auto subscription =
    std::make_shared<ScopedQueueWriter<std::shared_ptr<Order>>>(
      m_events.get_slot<std::shared_ptr<Order>>(
        std::bind_front(&AccountOrderController::recover, this)));
  m_subscriptions.insert(
    m_subscriptions.begin() + index, ScopedBaseQueue(subscription));
  task->GetContext().GetOrderPublisher().monitor(subscription);
}

void AccountOrderController::on_order_operation(
    const OrdersModel::Operation& operation) {
  visit(operation, [&] (const OrdersModel::AddOperation& operation) {
    on_order(m_controller->get_model()->get_orders()->get(operation.m_index));
  });
}

void AccountOrderController::on_live_order(
    const std::shared_ptr<Order>& order) {
  auto snapshot = order->get_publisher().get_snapshot();
  if(snapshot && !snapshot->empty()) {
    on_report(order->get_info().m_id, snapshot->back());
  }
}

void AccountOrderController::on_task_operation(
    const TasksModel::Operation& operation) {
  visit(operation,
    [&] (const TasksModel::AddOperation& operation) {
      monitor(operation.m_index);
    },
    [&] (const TasksModel::RemoveOperation& operation) {
      m_subscriptions.erase(m_subscriptions.begin() + operation.m_index);
    },
    [&] (const TasksModel::MoveOperation& operation) {
      move_element(m_subscriptions,
        operation.m_source, operation.m_destination);
    },
    [&] (const TasksModel::UpdateOperation& operation) {
      if(operation.get_previous().m_task != operation.get_value().m_task) {
        m_subscriptions.erase(m_subscriptions.begin() + operation.m_index);
        monitor(operation.m_index);
      }
    });
}

void AccountOrderController::on_order(const std::shared_ptr<Order>& order) {
  auto id = order->get_info().m_id;
  auto [i, inserted] = m_orders.try_emplace(id);
  if(!inserted) {
    return;
  }
  auto& entry = i->second;
  entry.m_order = order;
  auto snapshot = boost::optional<std::vector<ExecutionReport>>();
  order->get_publisher().monitor(m_events.get_slot<ExecutionReport>(
    std::bind_front(&AccountOrderController::on_report, this, id)),
    out(snapshot));
  m_events.push(std::bind_front(&AccountOrderController::recover, this, order));
}

void AccountOrderController::on_report(
    OrderId id, const ExecutionReport& report) {
  auto& entry = m_orders.at(id);
  if(report.m_status != OrderStatus::FILLED ||
      std::exchange(entry.m_is_fill_processed, true) ||
      m_controller->get_model()->get_account() !=
        m_profile->GetClients().get_service_locator_client().get_account()) {
    return;
  }
  auto& fields = entry.m_order->get_info().m_fields;
  if(fields.m_side == Side::NONE ||
      !m_profile->GetKeyBindings()->get_interactions_key_bindings(
        fields.m_ticker)->is_cancel_on_fill()->get()) {
    return;
  }
  for(auto& [candidate_id, candidate] : m_orders) {
    auto& other = candidate.m_order->get_info().m_fields;
    if(candidate_id == id || other.m_ticker != fields.m_ticker ||
        other.m_side != fields.m_side || candidate.m_is_cancel_submitted) {
      continue;
    }
    auto is_cancelable = candidate.m_order->get_publisher().with(
      [] (const auto& reports) {
        return !reports || reports->empty() ||
          (!is_terminal(reports->back().m_status) &&
            reports->back().m_status != OrderStatus::PENDING_CANCEL);
      });
    if(is_cancelable) {
      m_profile->GetClients().get_order_execution_client().cancel(
        candidate.m_order);
      candidate.m_is_cancel_submitted = true;
    }
  }
}
