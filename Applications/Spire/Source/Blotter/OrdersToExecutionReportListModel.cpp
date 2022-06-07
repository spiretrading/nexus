#include "Spire/Blotter/OrdersToExecutionReportListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

OrdersToExecutionReportListModel::OrdersToExecutionReportListModel(
    std::shared_ptr<OrderListModel> orders)
    : m_orders(std::move(orders)),
      m_reports(std::make_shared<ArrayListModel<ExecutionReportEntry>>()),
      m_connection(m_orders->connect_operation_signal(std::bind_front(
        &OrdersToExecutionReportListModel::on_operation, this))) {
  for(auto i = 0; i != m_orders->get_size(); ++i) {
    add(*m_orders->get(i));
  }
}

int OrdersToExecutionReportListModel::get_size() const {
  return m_reports->get_size();
}

const OrdersToExecutionReportListModel::Type&
    OrdersToExecutionReportListModel::get(int index) const {
  return m_reports->get(index);
}

connection OrdersToExecutionReportListModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_reports->connect_operation_signal(slot);
}

void OrdersToExecutionReportListModel::transact(
    const std::function<void ()>& transaction) {
  transaction();
}

void OrdersToExecutionReportListModel::add(const Order& order) {
  auto& count = m_order_to_count[&order];
  ++count;
  if(count == 1) {
    order.GetPublisher().Monitor(m_queue.get_slot<ExecutionReport>(
      std::bind_front(&OrdersToExecutionReportListModel::on_report, this,
        std::cref(order))));
  }
}

void OrdersToExecutionReportListModel::remove(const Order& order) {
  auto& count = m_order_to_count[&order];
  --count;
  if(count != 0) {
    return;
  }
  m_reports->transact([&] {
    for(auto i = m_reports->get_size() - 1; i >= 0; --i) {
      if(m_reports->get(i).m_order == &order) {
        m_orders->remove(i);
      }
    }
  });
}

void OrdersToExecutionReportListModel::on_report(
    const Order& order, const ExecutionReport& report) {
  m_reports->push(ExecutionReportEntry(&order, report));
}

void OrdersToExecutionReportListModel::on_operation(
    const OrderListModel::Operation& operation) {
  visit(operation,
    [&] (const OrderListModel::AddOperation& operation) {
      add(*operation.get_value());
    },
    [&] (const OrderListModel::RemoveOperation& operation) {
      remove(*operation.get_value());
    });
}
