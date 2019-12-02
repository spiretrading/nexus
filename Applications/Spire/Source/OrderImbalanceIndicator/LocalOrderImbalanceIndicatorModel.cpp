#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"

using namespace Nexus;
using namespace Spire;

void LocalOrderImbalanceIndicatorModel::publish(
    const OrderImbalance& imbalance) {
  m_last_published_imbalance = imbalance;
  m_imbalances.push_back(std::move(imbalance));
  m_imbalance_published_signal(imbalance);
}

void LocalOrderImbalanceIndicatorModel::insert(
    const OrderImbalance& imbalance) {
  m_imbalances.push_back(std::move(imbalance));
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::load(const TimeInterval& interval) {
  auto imbalances = std::vector<OrderImbalance>();
  for(auto& imbalance : m_imbalances) {
    if(imbalance.m_timestamp >= interval.left_open &&
        imbalance.m_timestamp <= interval.right_open) {
      imbalances.push_back(imbalance);
    }
  }
  return QtPromise([imbalances = std::move(imbalances)] () mutable {
      return std::move(imbalances);
  });
}

SubscriptionResult<boost::optional<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::subscribe(
    const OrderImbalanceSignal::slot_type& slot) {
  return {m_imbalance_published_signal.connect(slot),
    QtPromise([=] { return m_last_published_imbalance; })};
}

std::shared_ptr<OrderImbalanceChartModel>
    LocalOrderImbalanceIndicatorModel::get_chart_model(
    const Security& security) {
  throw std::runtime_error("method not implemented");
}
