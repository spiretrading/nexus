#include "Spire/OrderImbalanceIndicator/CachedOrderImbalanceIndicatorModel.hpp"

using namespace boost;
using namespace boost::icl;
using namespace boost::posix_time;
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
  if(m_ranges.find(interval) != m_ranges.end()) {
    return load_from_cache(interval);
  }
  return load_from_model(interval);
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
  //auto first = std::find_if(m_imbalances.begin(), m_imbalances.end(),
  //  [=] (const auto& imbalance) {
  //    return start <= imbalance.m_timestamp;
  //  });
  //auto last = std::find_if(m_imbalances.begin(), m_imbalances.end(),
  //  [=] (const auto& imbalance) { return end < imbalance.m_timestamp; });
  //return QtPromise(
  //  [imbalances = std::vector<OrderImbalance>(first, last)] () mutable {
  //    return std::move(imbalances);
  //  });
  return QtPromise([] { return std::vector<OrderImbalance>(); });
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    CachedOrderImbalanceIndicatorModel::load_from_model(
    const TimeInterval& interval) {
  auto promises = std::vector<QtPromise<std::vector<OrderImbalance>>>();
  auto load_ranges = m_ranges.subtract(interval);
  for(auto& range : load_ranges) {
    promises.push_back(m_source_model->load(range));
  }
  return all(std::move(promises)).then([=] (const auto& loaded_imbalances) {
    //m_ranges.add(interval);
    //for(auto& list : loaded_imbalances.Get()) {
    //    auto first = std::find_if(m_imbalances.begin(), m_imbalances.end(),
    //      [=] (const auto& imbalance) {
    //        return start < imbalance.m_timestamp;
    //      });
    //    auto last = std::find_if(m_imbalances.begin(), m_imbalances.end(),
    //      [=] (const auto& imbalance) {
    //        return end < imbalance.m_timestamp;
    //      });
    //    auto iter = m_imbalances.erase(first, last);
    //    m_imbalances.insert(iter, list.begin(), list.end());
    //}
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
