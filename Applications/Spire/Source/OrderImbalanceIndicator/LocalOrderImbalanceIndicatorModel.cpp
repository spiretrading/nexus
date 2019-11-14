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
  for(auto i = std::size_t(0); i < m_signals.size(); ++i) {
    if(m_signals[i].m_start_time <= imbalance.m_timestamp &&
        imbalance.m_timestamp <= m_signals[i].m_end_time) {
      m_signals[i].m_imbalance_signal(imbalance);
    }
  }
}

LocalOrderImbalanceIndicatorModel::Subscription::Subscription(
  const boost::posix_time::ptime& start,
  const boost::posix_time::ptime& end)
  : m_start_time(start), m_end_time(end) {}
