#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorTableModel.hpp"
#include <chrono>
#include <boost/date_time/gregorian/gregorian.hpp>

using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto EXPIRATION_TIMEOUT_MS = 1000;

  template<typename T>
  auto get_update(const T& potential,
      const T& current) -> optional<std::any> {
    if(potential == current) {
      return none;
    }
    return std::make_any<T>(potential);
  }
}

OrderImbalanceIndicatorTableModel::OrderImbalanceIndicatorTableModel(
    std::shared_ptr<OrderImbalanceIndicatorModel> source,
    TimeClientBox clock)
    : m_source(std::move(source)),
      m_clock(std::move(clock)) {
  m_expiration_timer.setInterval(EXPIRATION_TIMEOUT_MS);
  QObject::connect(&m_expiration_timer, &QTimer::timeout,
    [=] { on_expiration_timeout(); });
}

void OrderImbalanceIndicatorTableModel::set_interval(
    const TimeInterval& interval) {
  m_interval = interval;
  m_expiration_timer.stop();
  auto subscription = m_source->subscribe(m_interval,
    [=] (const auto& imbalance) { on_imbalance(imbalance); });
  m_subscription_connection = std::move(subscription.m_connection);
  m_load = std::move(subscription.m_snapshot);
  m_load.then([=] (auto&& result) { on_load(result.Get()); });
}

void OrderImbalanceIndicatorTableModel::set_offset(
    const time_duration& offset) {
  m_offset = offset;
  set_interval(TimeInterval::closed(
    m_clock.GetTime() - m_offset, std::numeric_limits<ptime>::max()));
  if(!m_expiration_timer.isActive()) {
    m_expiration_timer.start();
  }
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

void OrderImbalanceIndicatorTableModel::insert_imbalance(
    std::unordered_map<Security, Imbalance>& imbalances,
    const OrderImbalance& imbalance) {
  auto& previous_imbalance = imbalances[imbalance.m_security];
  if(previous_imbalance.m_imbalance != OrderImbalance()) {
    if(previous_imbalance.m_imbalance.m_timestamp < imbalance.m_timestamp) {
      set_row(imbalance, previous_imbalance);
    }
  } else {
    previous_imbalance = {get_row_size(), imbalance};
    m_table_model.push(make_row(imbalance));
  }
}

std::vector<std::any> OrderImbalanceIndicatorTableModel::make_row(
    const Nexus::OrderImbalance& imbalance) const {
  return {imbalance.m_security, imbalance.m_side, imbalance.m_size,
    imbalance.m_referencePrice, imbalance.m_size * imbalance.m_referencePrice,
    imbalance.m_timestamp.date(), imbalance.m_timestamp.time_of_day()};
}

std::vector<optional<std::any>>
    OrderImbalanceIndicatorTableModel::make_row_update(
      const OrderImbalance& current, const OrderImbalance& previous) const {
  return {none, get_update(current.m_side, previous.m_side),
    get_update(current.m_size, previous.m_size),
    get_update(current.m_referencePrice, previous.m_referencePrice),
    get_update(current.m_size * current.m_referencePrice,
      previous.m_size * previous.m_referencePrice),
    get_update(current.m_timestamp.date(), previous.m_timestamp.date()),
    get_update(current.m_timestamp.time_of_day(),
      previous.m_timestamp.time_of_day())};
}

void OrderImbalanceIndicatorTableModel::set_row(
    const OrderImbalance& current, Imbalance& previous) {
  m_table_model.transact([&] {
    auto row = make_row_update(current, previous.m_imbalance);
    for(auto i = std::size_t(1); i < row.size(); ++i) {
      if(auto value = row.at(i)) {
        m_table_model.set(previous.m_row_index, i, *value);
      }
    }
    previous.m_imbalance = current;
  });
}

void OrderImbalanceIndicatorTableModel::on_expiration_timeout() {
  auto removed_rows = 0;
  std::erase_if(m_imbalances, [&] (const auto& imbalance) {
    auto& current_imbalance = m_imbalances[imbalance.first];
    if(current_imbalance.m_imbalance.m_timestamp <
        m_clock.GetTime() - m_offset) {
      m_table_model.remove(current_imbalance.m_row_index - removed_rows);
      ++removed_rows;
      return true;
    }
    current_imbalance.m_row_index -= removed_rows;
    return false;
  });
}

void OrderImbalanceIndicatorTableModel::on_imbalance(
    const Nexus::OrderImbalance& imbalance) {
  insert_imbalance(m_imbalances, imbalance);
}

void OrderImbalanceIndicatorTableModel::on_load(
    const std::vector<OrderImbalance>& imbalances) {
  m_table_model.transact([&] {
    while(m_table_model.get_row_size() > 0) {
      m_table_model.remove(m_table_model.get_row_size() - 1);
    }
    m_imbalances = [&] {
      auto updated_imbalances = std::unordered_map<Security, Imbalance>();
      for(auto& imbalance : imbalances) {
        insert_imbalance(updated_imbalances, imbalance);
      }
      return updated_imbalances;
    }();
  });
}
