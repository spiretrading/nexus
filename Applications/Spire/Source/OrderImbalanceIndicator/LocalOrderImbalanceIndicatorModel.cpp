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
    const auto& current_imbalance = m_publish_queue.front();
    if(m_imbalances.contains(current_imbalance.m_security)) {
      auto& previous_imbalance = m_imbalances.at(imbalance.m_security);
      if(previous_imbalance.m_timestamp < imbalance.m_timestamp) {
        previous_imbalance = current_imbalance;
      }
    } else {
      m_imbalances.insert_or_assign(
        current_imbalance.m_security, current_imbalance);
    }
    for(const auto& subscription : m_subscriptions) {
      if(contains(subscription.m_interval, current_imbalance.m_timestamp)) {
        subscription.m_signal(current_imbalance);
      }
    }
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
  auto imbalances = std::vector<OrderImbalance>();
  for(const auto& [security, imbalance] : m_imbalances) {
    if(contains(interval, imbalance.m_timestamp)) {
      imbalances.push_back(imbalance);
    }
  }
  return imbalances;
}
