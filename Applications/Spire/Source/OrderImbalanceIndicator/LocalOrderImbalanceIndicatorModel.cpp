#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"

using namespace boost::icl;
using namespace Nexus;
using namespace Spire;

void LocalOrderImbalanceIndicatorModel::publish(
    const Nexus::OrderImbalance& imbalance) {
  m_publish_queue.push(imbalance);
  if(m_publish_queue.size() > 1) {
    return;
  }
  while(!m_publish_queue.empty()) {
    auto& current_imbalance = m_publish_queue.front();
    auto& security_imbalances = m_imbalances[current_imbalance.m_security];
    if(security_imbalances.empty() || current_imbalance.m_timestamp >
        security_imbalances.back().m_timestamp) {
      security_imbalances.push_back(current_imbalance);
    } else {
      auto index = std::lower_bound(security_imbalances.begin(),
        security_imbalances.end(), current_imbalance,
        [&] (const auto& imbalance, const auto& current) {
          return imbalance.m_timestamp < current.m_timestamp;
        });
      security_imbalances.insert(index, current_imbalance);
    }
    std::erase_if(m_subscriptions,
      [&] (const auto& subscription) {
        if(subscription.m_signal.empty()) {
          return true;
        }
        if(contains(subscription.m_interval, current_imbalance.m_timestamp)) {
          subscription.m_signal(current_imbalance);
        }
        return false;
      });
    m_publish_queue.pop();
  }
}

SubscriptionResult<std::vector<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::subscribe(const TimeInterval& interval,
      const OrderImbalanceSignal::slot_type& slot) {
  m_subscriptions.push_back({interval});
  return {m_subscriptions.back().m_signal.connect(slot), load(interval)};
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::load(
      const TimeInterval& interval) const {
  auto loaded_imbalances = std::vector<OrderImbalance>();
  for(auto& [security, imbalances] : m_imbalances) {
    if(imbalances.empty() || !intersects(interval, TimeInterval::closed(
        imbalances.front().m_timestamp, imbalances.back().m_timestamp))) {
      continue;
    }
    auto i = std::lower_bound(imbalances.begin(), imbalances.end(),
      interval.upper(), [&] (const auto& imbalance, auto upper) {
        if(is_right_closed(interval.bounds())) {
          return imbalance.m_timestamp <= upper;
        }
        return imbalance.m_timestamp < upper;
      });
    if(i != imbalances.begin()) {
      i = std::prev(i);
      if(contains(interval, i->m_timestamp)) {
        loaded_imbalances.push_back(*i);
      }
    }
  }
  return loaded_imbalances;
}
