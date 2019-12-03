#include "Spire/OrderImbalanceIndicator/CachedOrderImbalanceIndicatorModel.hpp"

using namespace boost;
using namespace boost::icl;
using namespace Nexus;
using namespace Spire;

CachedOrderImbalanceIndicatorModel::CachedOrderImbalanceIndicatorModel(
    std::shared_ptr<OrderImbalanceIndicatorModel> source)
    : m_source_model(std::move(source)) {
  m_source_model->subscribe([=] (const auto& imbalance) {
    on_imbalance_published(imbalance);
  });
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    CachedOrderImbalanceIndicatorModel::load(
    const TimeInterval& interval) {
  if(contains(m_ranges, interval)) {
    return load_from_model(interval);
  }
  return load_from_cache(interval);
}

SubscriptionResult<optional<Nexus::OrderImbalance>>
    CachedOrderImbalanceIndicatorModel::subscribe(
    const OrderImbalanceSignal::slot_type& slot) {
  return m_source_model->subscribe(slot);
}

std::shared_ptr<OrderImbalanceChartModel>
    CachedOrderImbalanceIndicatorModel::get_chart_model(
    const Security& security) {
  throw std::runtime_error("method not implemented");
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    CachedOrderImbalanceIndicatorModel::load_from_cache(
    const TimeInterval& interval) {
  
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    CachedOrderImbalanceIndicatorModel::load_from_model(
    const TimeInterval& interval) {
  auto promises = std::vector<QtPromise<std::vector<OrderImbalance>>>();
  auto load_ranges = interval - m_ranges;
  for(auto& range : load_ranges) {
    promises.push_back(m_source_model->load(range));
  }
  return all(promises).then([=] (const auto& loaded_imbalances) {
    m_ranges.insert(interval);
    // insert into cache
    return load_from_cache(interval);
  });
}

void CachedOrderImbalanceIndicatorModel::on_imbalance_published(
    const OrderImbalance& imbalance) {
  m_ranges.add({imbalance.m_timestamp, imbalance.m_timestamp});
  auto index = std::find_if(m_imbalances.begin(), m_imbalances.end(),
    [=] (const auto& cached_imbalance) {
      return imbalance.m_timestamp < cached_imbalance.m_timestamp;
    });
  m_imbalances.insert(index, imbalance);
}
