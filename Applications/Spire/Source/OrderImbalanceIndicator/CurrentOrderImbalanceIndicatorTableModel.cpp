#include "Spire/OrderImbalanceIndicator/CurrentOrderImbalanceIndicatorTableModel.hpp"
#include <Beam/Threading/Timer.hpp>

using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

CurrentOrderImbalanceIndicatorTableModel::CurrentOrderImbalanceIndicatorTableModel(
    time_duration offset, TimeClientBox clock, TimerFactory timer_factory,
    std::shared_ptr<OrderImbalanceIndicatorModel> source)
    : m_source(std::move(source)),
      m_offset(offset),
      m_clock(std::move(clock)),
      m_timer_factory(std::move(timer_factory)) {
  auto subscription = m_source->subscribe(
    TimeInterval::closed(m_clock.GetTime() - m_offset,
      std::numeric_limits<ptime>::max()), std::bind_front(
    &CurrentOrderImbalanceIndicatorTableModel::on_imbalance, this));
  m_subscription_connection = std::move(subscription.m_connection);
  m_load = subscription.m_snapshot.then(
    [=] (auto&& result) { on_load(result.Get()); });
}

int CurrentOrderImbalanceIndicatorTableModel::get_row_size() const {
  return m_table.get_row_size();
}

int CurrentOrderImbalanceIndicatorTableModel::get_column_size() const {
  return m_table.get_column_size();
}

const std::any& CurrentOrderImbalanceIndicatorTableModel::at(
    int row, int column) const {
  return m_table.at(row, column);
}

connection CurrentOrderImbalanceIndicatorTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_table.connect_operation_signal(slot);
}

void CurrentOrderImbalanceIndicatorTableModel::update_next_expiring() {
  if(m_table.get_row_size() == 0) {
    //m_next_expiring = none;
    m_timer.reset();
    return;
  }
  //auto next = std::min_element(m_imbalances.begin(), m_imbalances.end(),
  //  [] (const auto& first, const auto& second) {
  //    return first.second.m_imbalance.m_timestamp <
  //      second.second.m_imbalance.m_timestamp;
  //  })->second.m_imbalance;
  //if(next.m_timestamp > m_clock.GetTime()) {
  //  return;
  //}
  //m_next_expiring = next.m_security;
  //auto a = m_clock.GetTime() - next.m_timestamp;
  //auto b = m_offset - a;
  //m_timer = std::make_unique<TimerBox>(
  //  m_timer_factory(m_clock.GetTime() - next.m_timestamp));
  //m_timer->GetPublisher().Monitor(
  //  m_timer_queue.GetSlot<Beam::Threading::Timer::Result>(
  //    [=] (auto result) { on_expiration_timeout(result); }));
  //m_timer->Start();
}

void CurrentOrderImbalanceIndicatorTableModel::on_expiration_timeout(
    Timer::Result result) {
  if(result != Timer::Result::EXPIRED) {
    return;
  }
  m_table.remove(m_next_expiring);
  //auto removed_rows = 0;
  //std::erase_if(m_imbalances, [&] (const auto& imbalance) {
  //  auto& current_imbalance = m_imbalances[imbalance.first];
  //  if(current_imbalance.m_imbalance.m_timestamp <=
  //      m_clock.GetTime() - *m_offset) {
  //    m_table_model.remove(current_imbalance.m_row_index - removed_rows);
  //    ++removed_rows;
  //    return true;
  //  }
  //  current_imbalance.m_row_index -= removed_rows;
  //  return false;
  //});
  update_next_expiring();
}

void CurrentOrderImbalanceIndicatorTableModel::on_imbalance(
    const OrderImbalance& imbalance) {
  // TODO: check timestamp against expiring imbalance
  m_table.add(imbalance);
}

void CurrentOrderImbalanceIndicatorTableModel::on_load(
    const std::vector<OrderImbalance>& imbalances) {
  for(auto& imbalance : imbalances) {
    m_table.add(imbalance);
  }
}
