#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"

using namespace Nexus;
using namespace Spire;

OrderImbalanceIndicatorModel::SubscriptionResult
    LocalOrderImbalanceIndicatorModel::subscribe(
    const boost::posix_time::ptime& start, const boost::posix_time::ptime& end,
    const OrderImbalanceSignal::slot_type& slot) {
  m_signals.push_back(Subscription(start, end));
  auto imbalances = std::vector<OrderImbalance>();
  for(auto& imbalance : m_imbalances) {
    if(imbalance.m_timestamp >= start &&
        imbalance.m_timestamp <= end) {
      imbalances.push_back(imbalance);
    }
  }
  return {m_signals.back().m_imbalance_signal.connect(slot),
    QtPromise([imbalances = std::move(imbalances)] () mutable {
      return std::move(imbalances);
  })};
}

void LocalOrderImbalanceIndicatorModel::insert(
    const OrderImbalance& imbalance) {
  m_imbalances.push_back(imbalance);
  if(!m_signals.empty()) {
    update_signals(0, m_signals.size(), m_signals.size() - 1, imbalance);
  }
}

LocalOrderImbalanceIndicatorModel::Subscription::Subscription(
  const boost::posix_time::ptime& start,
  const boost::posix_time::ptime& end)
  : m_start_time(start), m_end_time(end) {}

void LocalOrderImbalanceIndicatorModel::update_signals(
    std::size_t current_index, std::size_t swap_index, std::size_t end_index,
    const OrderImbalance& imbalance) {
  if(m_signals[current_index].m_start_time <= imbalance.m_timestamp &&
      imbalance.m_timestamp <= m_signals[current_index].m_end_time) {
    m_signals[current_index].m_imbalance_signal(imbalance);
  }
  if(current_index == swap_index || current_index == end_index) {
    if(swap_index <= end_index ||
        m_signals[end_index].m_imbalance_signal.num_slots() == 0) {
      m_signals.erase(m_signals.begin() + current_index,
        m_signals.begin() + end_index + 1);
    }
    return;
  } else if(m_signals[current_index].m_imbalance_signal.num_slots() == 0) {
    std::swap(m_signals[current_index], m_signals[--swap_index]);
    --current_index;
  }
  update_signals(++current_index, swap_index, end_index, imbalance);
}
