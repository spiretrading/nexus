#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorTableModel.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>

using namespace Beam::TimeService;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto EXPIRATION_TIMEOUT_MS = 1000;
}

OrderImbalanceIndicatorTableModel::OrderImbalanceIndicatorTableModel(
    std::shared_ptr<OrderImbalanceIndicatorModel> source)
    : m_source(std::move(source)) {
  m_expiration_timer.setTimerType(Qt::CoarseTimer);
  m_expiration_timer.setInterval(EXPIRATION_TIMEOUT_MS);
  QObject::connect(&m_expiration_timer, &QTimer::timeout,
    [=] { on_expiration_timeout(); });
}

void OrderImbalanceIndicatorTableModel::set_interval(
    const TimeInterval& interval) {
  if(!m_expiration_timer.isActive()) {
    m_expiration_timer.start();
  }
  auto subscription = m_source->subscribe(interval,
    [=] (const auto& imbalance) { on_imbalance(imbalance); });
  m_subscription_connection = std::move(subscription.m_connection);
  m_load = std::move(subscription.m_snapshot);
  m_load.then([=] (auto&& result) { on_load(result.Get()); });
}

void OrderImbalanceIndicatorTableModel::set_offset(
    const time_duration& offset) {
  set_interval(TimeInterval::closed(
    microsec_clock::local_time() - offset, std::numeric_limits<ptime>::max()));
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
    imbalance.m_referencePrice, imbalance.m_size * imbalance.m_referencePrice,
    imbalance.m_timestamp.date(), imbalance.m_timestamp.time_of_day()};
}

void OrderImbalanceIndicatorTableModel::set_row(
    int index, const Nexus::OrderImbalance& imbalance) {
  m_table_model.transact([&] {
    auto row = make_row(imbalance);
    for(auto i = std::size_t(1); i < row.size(); ++i) {
      m_table_model.set(index, i, row.at(i));
    }
  });
}

void OrderImbalanceIndicatorTableModel::on_expiration_timeout() {

}

void OrderImbalanceIndicatorTableModel::on_imbalance(
    const Nexus::OrderImbalance& imbalance) {
  auto& current_imbalance = m_imbalances[imbalance.m_security];
  if(current_imbalance.m_imbalance != OrderImbalance()) {
    if(current_imbalance.m_imbalance.m_timestamp <
      imbalance.m_timestamp) {
      current_imbalance.m_imbalance = imbalance;
      set_row(current_imbalance.m_row_index, imbalance);
    }
  } else {
    current_imbalance = {get_row_size(), imbalance};
    m_table_model.push(make_row(imbalance));
  }
}

void OrderImbalanceIndicatorTableModel::on_load(
    const std::vector<OrderImbalance>& imbalances) {
  m_table_model.transact([&] {
    // TODO: works for now, improve
    auto current = std::unordered_set<Security>();
    for(auto& imbalance : imbalances) {
      current.insert(imbalance.m_security);
      on_imbalance(imbalance);
    }
    auto removed_rows = 0;
    std::erase_if(m_imbalances, [&] (const auto& imbalance) {
      auto& current_imbalance = m_imbalances[imbalance.first];
      if(!current.contains(current_imbalance.m_imbalance.m_security)) {
        m_table_model.remove(current_imbalance.m_row_index - removed_rows);
        ++removed_rows;
        return true;
      }
      current_imbalance.m_row_index -= removed_rows;
      return false;
    });
  });
}
