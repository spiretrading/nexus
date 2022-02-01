#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"

using namespace boost::icl;
using namespace Nexus;
using namespace Spire;

void LocalOrderImbalanceIndicatorModel::publish(
    const Nexus::OrderImbalance& imbalance) {
  if(m_imbalances.contains(imbalance.m_security)) {
    auto& current_imbalance = m_imbalances.at(imbalance.m_security);
    if(current_imbalance.m_timestamp < imbalance.m_timestamp) {
      current_imbalance = imbalance;
    } else {
      return;
    }
  } else {
    m_imbalances.insert_or_assign(imbalance.m_security, imbalance);
  }
  for(const auto& subscription : m_subscriptions) {
    if(contains(subscription.m_interval, imbalance.m_timestamp)) {
      subscription.m_signal(imbalance);
    }
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
