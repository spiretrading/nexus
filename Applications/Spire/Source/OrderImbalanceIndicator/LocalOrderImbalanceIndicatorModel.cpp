#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Nexus;
using namespace Spire;

void LocalOrderImbalanceIndicatorModel::publish(
    const OrderImbalance& imbalance) {
  insert_sorted(imbalance);
  m_imbalance_published_signal(imbalance);
}

void LocalOrderImbalanceIndicatorModel::insert(
    const OrderImbalance& imbalance) {
  insert_sorted(imbalance);
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::load(const TimeInterval& interval) {
  auto first = std::vector<OrderImbalance>::iterator();
  auto last = std::vector<OrderImbalance>::iterator();
  if(boost::icl::is_left_closed(interval.bounds())) {
    first = std::lower_bound(m_imbalances.begin(), m_imbalances.end(),
      interval.lower(), [] (const auto& imbalance, const auto& timestamp) {
        return imbalance.m_timestamp < timestamp;
      });
  } else {
    first = std::upper_bound(m_imbalances.begin(), m_imbalances.end(),
      interval.lower(), [] (const auto& timestamp, const auto& imbalance) {
        return imbalance.m_timestamp > timestamp;
      });
  }
  if(boost::icl::is_right_closed(interval.bounds())) {
    last = std::upper_bound(m_imbalances.begin(), m_imbalances.end(),
      interval.upper(), [] (const auto& timestamp, const auto& imbalance) {
        return imbalance.m_timestamp > timestamp;
      });
  } else {
    last = std::lower_bound(m_imbalances.begin(), m_imbalances.end(),
      interval.upper(), [] (const auto& imbalance, const auto& timestamp) {
        return imbalance.m_timestamp < timestamp;
      });
  }
  return QtPromise(
    [imbalances = std::vector<OrderImbalance>(first, last)] () mutable {
      return std::move(imbalances);
  });
}

SubscriptionResult<boost::optional<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::subscribe(
    const OrderImbalanceSignal::slot_type& slot) {
  auto last_imbalance = [&] () -> boost::optional<Nexus::OrderImbalance> {
    if(m_imbalances.empty()) {
      return boost::none;
    }
    return m_imbalances.back();
  }();
  return {m_imbalance_published_signal.connect(slot),
    QtPromise([imbalance = std::move(last_imbalance)] () mutable {
      return std::move(imbalance); })};
}

std::shared_ptr<OrderImbalanceChartModel>
    LocalOrderImbalanceIndicatorModel::get_chart_model(
    const Security& security) {
  throw std::runtime_error("method not implemented");
}

void LocalOrderImbalanceIndicatorModel::insert_sorted(
    const OrderImbalance& imbalance) {
  auto index = std::lower_bound(m_imbalances.begin(), m_imbalances.end(),
    imbalance.m_timestamp,
    [] (const auto& stored_imbalance, const auto& timestamp) {
      return stored_imbalance.m_timestamp < timestamp;
    });
  m_imbalances.insert(index, imbalance);
}
