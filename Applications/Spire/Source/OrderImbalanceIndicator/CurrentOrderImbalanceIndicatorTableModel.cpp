#include "Spire/OrderImbalanceIndicator/CurrentOrderImbalanceIndicatorTableModel.hpp"
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Threading/Timer.hpp>

using namespace Beam;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

CurrentOrderImbalanceIndicatorTableModel::CallbackTimer::CallbackTimer(
    std::unique_ptr<TimerBox> timer, std::function<void ()> expiration_callback)
    : m_timer(std::move(timer)),
      m_expiration_callback(std::move(expiration_callback)) {
  m_timer->GetPublisher().Monitor(
    m_queue.GetSlot<Timer::Result>([=] (auto result) {
      if(result == Timer::Result::EXPIRED) { m_expiration_callback(); }
    }));
  m_timer->Start();
}

CurrentOrderImbalanceIndicatorTableModel::CurrentOrderImbalanceIndicatorTableModel(
    time_duration offset, TimeClientBox clock, TimerFactory timer_factory,
    std::shared_ptr<OrderImbalanceIndicatorModel> source)
    : m_source(std::move(source)),
      m_offset(offset),
      m_clock(std::move(clock)),
      m_timer_factory(std::move(timer_factory)) {
  m_operation_connection = m_table.connect_operation_signal(std::bind_front(
    &CurrentOrderImbalanceIndicatorTableModel::on_operation, this));
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
  return m_transaction.connect_operation_signal(slot);
}

void CurrentOrderImbalanceIndicatorTableModel::update_next_expiring() {
  if(m_table.get_row_size() == 0) {
    m_next_expiring = none;
    return;
  }
  auto next_expiration = std::numeric_limits<ptime>::max();
  auto row = 0;
  auto time_interval_lower = m_clock.GetTime() - m_offset;
  for(auto i = 0; i < m_table.get_row_size();) {
    auto time =
      ptime(m_table.get<date>(i, 5), m_table.get<time_duration>(i, 6));
    if(time <= time_interval_lower) {
      m_table.remove(m_table.get<Security>(i, 0));
      continue;
    } else if(time < next_expiration) {
      next_expiration = time;
      row = i;
    }
    ++i;
  }
  if(m_table.get_row_size() == 0) {
    m_next_expiring = none;
    return;
  }
  m_next_expiring = Expiring{m_table.get<Security>(row, 0),
    ptime(m_table.get<date>(row, 5), m_table.get<time_duration>(row, 6))};
  m_timers.emplace_back(std::make_unique<TimerBox>(m_timer_factory(
    (m_next_expiring->m_timestamp + m_offset) - m_clock.GetTime())),
    std::bind_front(
      &CurrentOrderImbalanceIndicatorTableModel::on_expiration_timeout, this));
}

void CurrentOrderImbalanceIndicatorTableModel::on_expiration_timeout() {
  m_transaction.transact([&] {
    m_table.remove(m_next_expiring->m_security);
    update_next_expiring();
  });
}

void CurrentOrderImbalanceIndicatorTableModel::on_imbalance(
    const OrderImbalance& imbalance) {
  if(imbalance.m_timestamp < m_clock.GetTime() - m_offset) {
    return;
  }
  auto is_update_required =
    !m_next_expiring || m_next_expiring->m_timestamp > imbalance.m_timestamp ||
    m_next_expiring->m_security == imbalance.m_security;
  m_transaction.transact([&] {
    m_table.add(imbalance);
    if(is_update_required) {
      m_timers.clear();
      update_next_expiring();
    }
  });
}

void CurrentOrderImbalanceIndicatorTableModel::on_load(
    const std::vector<OrderImbalance>& imbalances) {
  m_transaction.transact([&] {
    auto oldest = std::numeric_limits<ptime>::max();
    auto time_interval_lower = m_clock.GetTime() - m_offset;
    for(auto& imbalance : imbalances) {
      if(imbalance.m_timestamp > time_interval_lower) {
        m_table.add(imbalance);
        oldest = std::min(oldest, imbalance.m_timestamp);
      }
    }
    if(!m_next_expiring || m_next_expiring->m_timestamp > oldest) {
      m_timers.clear();
      update_next_expiring();
    }
  });
}

void CurrentOrderImbalanceIndicatorTableModel::on_operation(
    TableModel::Operation operation) {
  m_transaction.push(std::move(operation));
}
