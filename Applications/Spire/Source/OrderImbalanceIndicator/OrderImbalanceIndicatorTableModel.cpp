#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorTableModel.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderImbalanceIndicatorTableModel::OrderImbalanceIndicatorTableModel(
    std::shared_ptr<OrderImbalanceIndicatorModel> source)
    : m_source(std::move(source)) {

}

void OrderImbalanceIndicatorTableModel::set_interval(
    const TimeInterval& interval) {
  auto subscription = m_source->subscribe(interval,
    [=] (const auto& imbalance) { on_imbalance(imbalance); });
  m_subscription_connection = std::move(subscription.m_connection);
  m_load = std::move(subscription.m_snapshot);
  m_load.then([=] (auto&& result) { on_load(result.Get()); });
}

void OrderImbalanceIndicatorTableModel::set_offset(
    const time_duration& offset) {
  // TODO: proper current time source, localization?
  set_interval(TimeInterval::closed(
    second_clock::local_time() - offset, std::numeric_limits<ptime>::max()));
}

int OrderImbalanceIndicatorTableModel::get_row_size() const {
  return m_table_model.get_row_size();
}

int OrderImbalanceIndicatorTableModel::get_column_size() const {
  return m_table_model.get_column_size();
}

const std::any&
    OrderImbalanceIndicatorTableModel::at(int row, int column) const {
  return m_table_model.at(row, column);
}

connection OrderImbalanceIndicatorTableModel::connect_operation_signal(
    const typename OperationSignal::slot_type& slot) const {
  return m_table_model.connect_operation_signal(slot);
}

std::vector<std::any> OrderImbalanceIndicatorTableModel::make_row(
    const Nexus::OrderImbalance& imbalance) {
  return {imbalance.m_security, imbalance.m_side, imbalance.m_size,
    imbalance.m_referencePrice, imbalance.m_side * imbalance.m_referencePrice,
    imbalance.m_timestamp.date(), imbalance.m_timestamp.time_of_day()};
}

void OrderImbalanceIndicatorTableModel::set_row(
    int index, const Nexus::OrderImbalance& imbalance) {
  // TODO: 'batch' these updates if possible
  auto row = make_row(imbalance);
  for(auto i = 1; i < row.size(); ++i) {
    m_table_model.set(index, i, row.at(i));
  }
}

void OrderImbalanceIndicatorTableModel::on_imbalance(
    const Nexus::OrderImbalance& imbalance) {
  if(auto existing_imbalance = m_imbalances.find(imbalance.m_security);
      existing_imbalance != m_imbalances.end()) {
    if(existing_imbalance->second.m_imbalance.m_timestamp <
      imbalance.m_timestamp) {
      existing_imbalance->second.m_imbalance = imbalance;
      set_row(existing_imbalance->second.m_row_index, imbalance);
    }
  } else {
    m_imbalances.insert({imbalance.m_security, {get_row_size(), imbalance}});
    m_table_model.push(make_row(imbalance));
  }
}

void OrderImbalanceIndicatorTableModel::on_load(
    const std::vector<OrderImbalance>& imbalances) {
  for(const auto& imbalance : imbalances) {
    on_imbalance(imbalance);
  }
}
