#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

SubscriptionResult<std::vector<OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::subscribe(ptime start, ptime end,
    const OrderImbalanceSignal::slot_type& slot) {
  m_signals.push_back(Subscription(start, end));
  auto imbalances = std::vector<OrderImbalance>();
  for(auto& imbalance : m_imbalances) {
    if(imbalance.m_timestamp >= start && imbalance.m_timestamp <= end) {
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
  auto end_index = m_signals.size();
  auto swap_index = end_index;
  auto current_index = std::size_t(0);
  while(current_index != swap_index) {
    auto& signal = m_signals[current_index];
    if(signal.m_imbalance_signal.num_slots() != 0) {
      if(signal.m_start <= imbalance.m_timestamp &&
          imbalance.m_timestamp <= signal.m_end) {
        signal.m_imbalance_signal(imbalance);
      }
      ++current_index;
    } else {
      std::swap(signal, m_signals[--swap_index]);
    }
  }
  m_signals.erase(m_signals.begin() + swap_index,
    m_signals.begin() + end_index);
}

LocalOrderImbalanceIndicatorModel::Subscription::Subscription(ptime start,
    ptime end)
  : m_start(start),
    m_end(end) {}
