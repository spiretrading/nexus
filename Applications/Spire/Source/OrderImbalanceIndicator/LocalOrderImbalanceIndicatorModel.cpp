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
  auto end_index = m_signals.size();
  auto swap_index = end_index;
  for(auto i = std::size_t(0); i < end_index && i != swap_index; ++i) {
    if(m_signals[i].m_imbalance_signal.num_slots() != 0) {
      if(m_signals[i].m_start_time <= imbalance.m_timestamp &&
          imbalance.m_timestamp <= m_signals[i].m_end_time) {
        m_signals[i].m_imbalance_signal(imbalance);
      }
    } else {
      std::swap(m_signals[i], m_signals[--swap_index]);
      --i;
    }
  }
  m_signals.erase(m_signals.begin() + swap_index,
    m_signals.begin() + end_index);
}

LocalOrderImbalanceIndicatorModel::Subscription::Subscription(
  const boost::posix_time::ptime& start,
  const boost::posix_time::ptime& end)
  : m_start_time(start), m_end_time(end) {}
