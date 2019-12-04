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
  auto first = [&] {
    if(is_left_closed(interval.bounds())) {
      return std::lower_bound(m_imbalances.begin(), m_imbalances.end(),
        interval.lower(), [] (const auto& imbalance, const auto& timestamp) {
          return imbalance.m_timestamp < timestamp;
        });
    }
    return std::upper_bound(m_imbalances.begin(), m_imbalances.end(),
      interval.lower(), [] (const auto& timestamp, const auto& imbalance) {
        return imbalance.m_timestamp > timestamp;
      });
  }();
  auto last = [&] {
    if(is_right_closed(interval.bounds())) {
     return std::upper_bound(first, m_imbalances.end(),
        interval.upper(), [] (const auto& timestamp, const auto& imbalance) {
          return imbalance.m_timestamp > timestamp;
        });
    }
    return std::lower_bound(first, m_imbalances.end(),
      interval.upper(), [] (const auto& imbalance, const auto& timestamp) {
        return imbalance.m_timestamp < timestamp;
      });
  }();
  return QtPromise(
    [imbalances = std::vector<OrderImbalance>(first, last)] () mutable {
      return std::move(imbalances);
    });
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
      auto first = std::lower_bound(m_imbalances.begin(), m_imbalances.end(),
        interval.lower(), [] (const auto& imbalance, const auto& timestamp) {
          return imbalance.m_timestamp < timestamp;
        });
      auto last = std::upper_bound(first, m_imbalances.end(),
        interval.upper(), [] (const auto& timestamp, const auto& imbalance) {
          return imbalance.m_timestamp > timestamp;
        });
      auto iter = m_imbalances.erase(first, last);
      m_imbalances.insert(iter, list.begin(), list.end());
    }
    return load_from_cache(interval);
  });
}

void CachedOrderImbalanceIndicatorModel::on_imbalance_published(
    const OrderImbalance& imbalance) {
  m_ranges.add(TimeInterval::closed(imbalance.m_timestamp,
    imbalance.m_timestamp));
  if(m_imbalances.empty() ||
      m_imbalances.back().m_timestamp < imbalance.m_timestamp) {
    m_imbalances.push_back(imbalance);
    return;
  }
  auto index = std::lower_bound(m_imbalances.begin(), m_imbalances.end(),
    imbalance.m_timestamp,
    [] (const auto& stored_imbalance, const auto& timestamp) {
      return stored_imbalance.m_timestamp < timestamp;
    });
  m_imbalances.insert(index, imbalance);
}
