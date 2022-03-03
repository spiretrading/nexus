#include "Spire/OrderImbalanceIndicator/CurrentOrderImbalanceIndicatorTableModel.hpp"
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Threading/Timer.hpp>

using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::gregorian;
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

void CurrentOrderImbalanceIndicatorTableModel::set_next_expiring(int row) {
  m_next_expiring = Expiring{m_table.get<Security>(row, 0),
    ptime(m_table.get<date>(row, 5), m_table.get<time_duration>(row, 6))};
}

void CurrentOrderImbalanceIndicatorTableModel::update_next_expiring() {
  if(m_table.get_row_size() == 0) {
    m_next_expiring = none;
    m_timer.reset();
    return;
  }
  auto next_expiration = std::numeric_limits<ptime>::max();
  auto row = 0;
  for(auto i = 0; i < m_table.get_row_size(); ++i) {
    auto time =
      ptime(m_table.get<date>(i, 5), m_table.get<time_duration>(i, 6));
    if(time < next_expiration) {
      row = i;
      //m_next_expiring = Expiring{m_table.get<Security>(i, 0), time};
    }
  }
  set_next_expiring(row);
  //auto a = m_clock.GetTime() - m_next_expiring->m_timestamp;
  //auto b = m_offset - a;
  auto timer = std::make_unique<TimerBox>(
    m_timer_factory(m_next_expiring->m_timestamp - m_clock.GetTime()));
  m_timers.push_back(std::move(timer));
  m_timers.back()->GetPublisher().Monitor(m_timer_queue.GetSlot<Timer::Result>(
    [=] (auto result) { on_expiration_timeout(result); }));
  m_timers.back()->Start();
}

void CurrentOrderImbalanceIndicatorTableModel::on_expiration_timeout(
    Timer::Result result) {
  // TODO:
  //if(result != Timer::Result::EXPIRED) {
  //  return;
  //}
  m_table.remove(m_next_expiring->m_security);
  update_next_expiring();
}

void CurrentOrderImbalanceIndicatorTableModel::on_imbalance(
    const OrderImbalance& imbalance) {
  auto is_update_required =
    !m_next_expiring || m_next_expiring->m_timestamp > imbalance.m_timestamp ||
    m_next_expiring->m_security == imbalance.m_security;
  m_table.add(imbalance);
  if(is_update_required) {
    update_next_expiring();
  }
}

void CurrentOrderImbalanceIndicatorTableModel::on_load(
    const std::vector<OrderImbalance>& imbalances) {
  for(auto& imbalance : imbalances) {
    m_table.add(imbalance);
  }
  update_next_expiring();
}
