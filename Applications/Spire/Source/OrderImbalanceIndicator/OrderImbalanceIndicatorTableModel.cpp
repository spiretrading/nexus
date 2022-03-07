#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorTableModel.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderImbalanceIndicatorTableModel::OrderImbalanceIndicatorTableModel(
    TimeInterval interval,
    std::shared_ptr<OrderImbalanceIndicatorModel> source)
    : m_source(std::move(source)) {
  auto subscription = m_source->subscribe(interval, std::bind_front(
    &OrderImbalanceIndicatorTableModel::on_imbalance, this));
  m_subscription_connection = std::move(subscription.m_connection);
  m_load = subscription.m_snapshot.then(
    [=] (auto&& result) { on_load(result.Get()); });
}

int OrderImbalanceIndicatorTableModel::get_row_size() const {
  return m_table.get_row_size();
}

int OrderImbalanceIndicatorTableModel::get_column_size() const {
  return m_table.get_column_size();
}

const std::any& OrderImbalanceIndicatorTableModel::at(
    int row, int column) const {
  return m_table.at(row, column);
}

connection OrderImbalanceIndicatorTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_table.connect_operation_signal(slot);
}

void OrderImbalanceIndicatorTableModel::on_imbalance(
    const OrderImbalance& imbalance) {
  m_table.add(imbalance);
}

void OrderImbalanceIndicatorTableModel::on_load(
    const std::vector<OrderImbalance>& imbalances) {
  for(auto& imbalance : imbalances) {
    m_table.add(imbalance);
  }
}
