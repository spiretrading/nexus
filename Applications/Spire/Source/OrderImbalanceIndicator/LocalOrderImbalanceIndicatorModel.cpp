#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost;
using namespace boost::icl;
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
  auto [first, last] = get_iterators_from_interval(interval);
  return std::vector<OrderImbalance>(first, last);
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::load(const Security& security,
    const TimeInterval& interval) {
  auto imbalances = std::vector<OrderImbalance>();
  auto [first, last] = get_iterators_from_interval(interval);
  for(auto i = first; i != last; ++i) {
    if(i->m_security == security) {
      imbalances.push_back(*i);
    }
  }
  return imbalances;
}

SubscriptionResult<optional<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::subscribe(
    const OrderImbalanceSignal::slot_type& slot) {
  auto last_imbalance = [&] () -> optional<Nexus::OrderImbalance> {
    if(m_imbalances.empty()) {
      return none;
    }
    return m_imbalances.back();
  }();
  return {m_imbalance_published_signal.connect(slot),
    QtPromise(std::move(last_imbalance))};
}

void LocalOrderImbalanceIndicatorModel::insert_sorted(
    const OrderImbalance& imbalance) {
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

std::tuple<std::vector<OrderImbalance>::iterator,
    std::vector<OrderImbalance>::iterator>
    LocalOrderImbalanceIndicatorModel::get_iterators_from_interval(
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
  return {first, last};
}
