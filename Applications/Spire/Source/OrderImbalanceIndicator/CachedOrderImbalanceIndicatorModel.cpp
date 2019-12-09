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
  if(boost::icl::contains(m_intervals, interval)) {
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
  auto load_intervals = interval_set<ptime>(interval);
  load_intervals = load_intervals - m_intervals;
  for(auto& range : load_intervals) {
    promises.push_back(m_source_model->load(TimeInterval::closed(range.lower(),
      range.upper())));
  }
  return all(std::move(promises)).then([=] (const auto& loaded_imbalances) {
    for(auto& list : loaded_imbalances.Get()) {
      if(!list.empty()) {
        auto unique_intervals = interval_set<ptime>({list.front().m_timestamp,
          list.back().m_timestamp});
        for(auto& unique_interval : unique_intervals) {
          auto first = std::lower_bound(list.begin(), list.end(),
            unique_interval.lower(),
            [] (const auto& imbalance, const auto& timestamp) {
              return imbalance.m_timestamp < timestamp;
            });
          while(first != list.end() && boost::icl::contains(m_intervals,
              TimeInterval::closed(first->m_timestamp, first->m_timestamp))) {
            ++first;
          }
          auto last = std::upper_bound(list.begin(), list.end(),
            unique_interval.upper(),
            [] (const auto& timestamp, const auto& imbalance) {
              return imbalance.m_timestamp > timestamp;
            });
          if(last == list.end()) {
            --last;
          }
          while(last != list.begin() && boost::icl::contains(m_intervals,
              TimeInterval::closed(last->m_timestamp, last->m_timestamp))) {
            --last;
          }
          for(auto i = first; i <= last; ++i) {
            m_cache.insert(std::move(*i));
          }
        }
      }
    }
    m_intervals.add(interval);
    return load_from_cache(interval);
  });
}

void CachedOrderImbalanceIndicatorModel::on_imbalance_published(
    const OrderImbalance& imbalance) {
  m_intervals.add(TimeInterval::closed(imbalance.m_timestamp,
    imbalance.m_timestamp));
  m_cache.insert(imbalance);
}
