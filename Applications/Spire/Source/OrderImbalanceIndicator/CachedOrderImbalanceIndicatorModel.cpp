#include "Spire/OrderImbalanceIndicator/CachedOrderImbalanceIndicatorModel.hpp"

using namespace boost;
using namespace boost::icl;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

CachedOrderImbalanceIndicatorModel::CachedOrderImbalanceIndicatorModel(
    std::shared_ptr<OrderImbalanceIndicatorModel> source)
    : m_source_model(std::move(source)) {
  auto [connection, promise] = m_source_model->subscribe(
    [=] (const auto& imbalance) {
      on_imbalance_published(imbalance);
    });
  m_subscription_connection = std::move(connection);
  m_subscribe_promise = std::move(promise);
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    CachedOrderImbalanceIndicatorModel::load(
    const TimeInterval& interval) {
  if(boost::icl::contains(m_ranges, interval)) {
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
  return m_cache.load(interval);
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    CachedOrderImbalanceIndicatorModel::load_from_model(
    const TimeInterval& interval) {
  auto promises = std::vector<QtPromise<std::vector<OrderImbalance>>>();
  auto load_ranges = interval_set<ptime>(interval);
  load_ranges = load_ranges - m_ranges;
  for(auto& range : load_ranges) {
    promises.push_back(m_source_model->load(TimeInterval::closed(range.lower(),
      range.upper())));
  }
  return all(std::move(promises)).then([=] (const auto& loaded_imbalances) {
    m_ranges.add(interval);
    for(auto& list : loaded_imbalances.Get()) {
      for(auto i = std::size_t(0); i < list.size(); ++i) {
        m_cache.insert(std::move(list[i]));
      }
    }
    return load_from_cache(interval);
  });
}

void CachedOrderImbalanceIndicatorModel::on_imbalance_published(
    const OrderImbalance& imbalance) {
  m_ranges.add(TimeInterval::closed(imbalance.m_timestamp,
    imbalance.m_timestamp));
  m_cache.insert(imbalance);
}
