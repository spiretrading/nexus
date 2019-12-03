#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Nexus;
using namespace Spire;

void LocalOrderImbalanceIndicatorModel::publish(
    const OrderImbalance& imbalance) {
  m_last_published_imbalance = imbalance;
  insert_sorted(imbalance);
  m_imbalance_published_signal(imbalance);
}

void LocalOrderImbalanceIndicatorModel::insert(
    const OrderImbalance& imbalance) {
  insert_sorted(imbalance);
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::load(const TimeInterval& interval) {
  auto first = std::find_if(m_imbalances.begin(), m_imbalances.end(),
    [&] (const auto& imbalance) {
      return interval.lower() < imbalance.m_timestamp;
    });
  auto last = std::find_if(m_imbalances.begin(), m_imbalances.end(),
    [&] (const auto& imbalance) {
      return interval.upper() < imbalance.m_timestamp;
    });
  if(last != m_imbalances.end()) {
    ++last;
  }
  return QtPromise(
    [imbalances = std::vector<OrderImbalance>(first, last)] () mutable {
      return std::move(imbalances);
  });
}

SubscriptionResult<boost::optional<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::subscribe(
    const OrderImbalanceSignal::slot_type& slot) {
  return {m_imbalance_published_signal.connect(slot),
    QtPromise([imbalance = m_last_published_imbalance] () mutable {
      return std::move(imbalance); })};
}

std::shared_ptr<OrderImbalanceChartModel>
    LocalOrderImbalanceIndicatorModel::get_chart_model(
    const Security& security) {
  throw std::runtime_error("method not implemented");
}

void LocalOrderImbalanceIndicatorModel::insert_sorted(
    const OrderImbalance& imbalance) {
  auto index = std::find_if(m_imbalances.begin(), m_imbalances.end(),
    [=] (const auto& stored_imbalance) {
      return imbalance.m_timestamp < stored_imbalance.m_timestamp;
    });
  m_imbalances.insert(index, imbalance);
}
