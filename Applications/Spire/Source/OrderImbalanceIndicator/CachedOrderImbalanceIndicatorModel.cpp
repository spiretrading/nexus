#include "Spire/OrderImbalanceIndicator/CachedOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"

using namespace boost::icl;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

CachedOrderImbalanceIndicatorModel::CachedOrderImbalanceIndicatorModel(
  std::shared_ptr<OrderImbalanceIndicatorModel> source)
  : m_source(std::move(source)) {}

OrderImbalanceIndicatorModel::SubscriptionResult
    CachedOrderImbalanceIndicatorModel::subscribe(ptime start, ptime end,
    const OrderImbalanceSignal::slot_type& slot) {
  if(contains(m_ranges, continuous_interval<ptime>::closed(start, end))) {
    return make_subscription(start, end, slot);
  }
  return load_imbalances(start, end, slot);
}

OrderImbalanceIndicatorModel::SubscriptionResult
    CachedOrderImbalanceIndicatorModel::make_subscription(ptime start,
    ptime end, const OrderImbalanceSignal::slot_type& slot) {
  m_subscriptions.push_back(Subscription(start, end));
  auto first = std::find_if(m_imbalances.begin(), m_imbalances.end(),
    [=] (const auto& imbalance) { return start <= imbalance.m_timestamp; });
  auto last = std::find_if(m_imbalances.begin(), m_imbalances.end(),
    [=] (const auto& imbalance) { return end < imbalance.m_timestamp; });
  return {m_subscriptions.back().m_imbalance_signal.connect(slot),
    QtPromise([=] {
      return std::vector<OrderImbalance>(first, last);
    })};
}

OrderImbalanceIndicatorModel::SubscriptionResult
    CachedOrderImbalanceIndicatorModel::load_imbalances(ptime start,
    ptime end, const OrderImbalanceSignal::slot_type& slot) {
  auto promises = std::vector<QtPromise<std::vector<OrderImbalance>>>();
  auto ranges = interval_set<ptime>(
    continuous_interval<ptime>::closed(start, end)) - m_ranges;
  for(auto& range : ranges) {
    auto [connection, promise] = m_source->subscribe(range.lower(),
      range.upper(), [=] (const auto& imbalance) {
        on_order_imbalance(imbalance);
      });
    m_connections.push_back(std::move(connection));
    promises.push_back(std::move(promise));
  }
  m_subscriptions.push_back(Subscription(start, end));
  return {m_subscriptions.back().m_imbalance_signal.connect(slot),
    all(std::move(promises)).then([=] (const auto& imbalances_lists) {
      m_ranges.add(continuous_interval<ptime>::closed(start, end));
      if(m_imbalances.empty()) {
        auto lists = imbalances_lists.Get();
        if(lists.empty()) {
          return std::vector<OrderImbalance>();
        }
        auto list = lists[0];
        m_imbalances.insert(m_imbalances.begin(), list.begin(), list.end());
        return m_imbalances;
      }
      for(auto& list : imbalances_lists.Get()) {
        auto first = std::find_if(m_imbalances.begin(), m_imbalances.end(),
          [=] (const auto& imbalance) {
            return start < imbalance.m_timestamp;
          });
        auto last = std::find_if(m_imbalances.begin(), m_imbalances.end(),
          [=] (const auto& imbalance) {
            return end < imbalance.m_timestamp;
          });
        auto iter = m_imbalances.erase(first, last);
        m_imbalances.insert(iter, list.begin(), list.end());
      }
      auto first = std::find_if(m_imbalances.begin(), m_imbalances.end(),
        [=] (const auto& imbalance) {
          return start <= imbalance.m_timestamp;
        });
      auto last = std::find_if(m_imbalances.begin(), m_imbalances.end(),
        [=] (const auto& imbalance) { return end < imbalance.m_timestamp; });
      return std::vector<OrderImbalance>(first, last);
    })};
}

void CachedOrderImbalanceIndicatorModel::on_order_imbalance(
    const OrderImbalance& imbalance) {
  m_ranges.add(continuous_interval<ptime>::closed(imbalance.m_timestamp,
    imbalance.m_timestamp));
  auto index = std::find_if(m_imbalances.begin(), m_imbalances.end(),
    [=] (const auto& cached_imbalance) {
      return imbalance.m_timestamp < cached_imbalance.m_timestamp;
    });
  auto end_index = m_subscriptions.size();
  auto swap_index = end_index;
  auto current_index = std::size_t(0);
  while(current_index != swap_index) {
    auto& subscription = m_subscriptions[current_index];
    if(subscription.m_imbalance_signal.num_slots() != 0) {
      if(subscription.m_start <= imbalance.m_timestamp &&
          imbalance.m_timestamp <= subscription.m_end) {
        subscription.m_imbalance_signal(imbalance);
      }
      ++current_index;
    } else {
      std::swap(subscription, m_subscriptions[--swap_index]);
    }
  }
  m_subscriptions.erase(m_subscriptions.begin() + swap_index,
    m_subscriptions.begin() + end_index);
}

CachedOrderImbalanceIndicatorModel::Subscription::Subscription(ptime start,
  ptime end)
  : m_start(start),
    m_end(end) {}
