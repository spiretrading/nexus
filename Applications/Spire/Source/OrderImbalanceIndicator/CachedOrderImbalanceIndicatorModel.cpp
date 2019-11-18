#include "Spire/OrderImbalanceIndicator/CachedOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"

using namespace boost::icl;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

CachedOrderImbalanceIndicatorModel::CachedOrderImbalanceIndicatorModel(
  std::shared_ptr<OrderImbalanceIndicatorModel> m_source_model)
  : m_source_model(std::move(m_source_model)) {}

OrderImbalanceIndicatorModel::SubscriptionResult
    CachedOrderImbalanceIndicatorModel::subscribe(
    const boost::posix_time::ptime& start,
    const boost::posix_time::ptime& end,
    const OrderImbalanceSignal::slot_type& slot) {
  if(contains(m_ranges, continuous_interval<ptime>::closed(start, end))) {
    return get_subscription(start, end, slot);
  }
  return load_imbalances(start, end, slot);
}

OrderImbalanceIndicatorModel::SubscriptionResult
    CachedOrderImbalanceIndicatorModel::get_subscription(
    const boost::posix_time::ptime& start,
    const boost::posix_time::ptime& end,
    const OrderImbalanceSignal::slot_type& slot) {
  m_subscriptions.push_back(Subscription(start, end));
  auto imbalances = std::vector<OrderImbalance>();
  std::copy_if(m_imbalances.begin(), m_imbalances.end(),
    std::back_inserter(imbalances), [=] (auto& imbalance) {
      return start <= imbalance.m_timestamp && imbalance.m_timestamp <= end;
    });
  return {m_subscriptions.back().m_imbalance_signal.connect(slot),
    QtPromise([requested_imbalances = std::move(imbalances)] {
      return requested_imbalances;
    })};
}

OrderImbalanceIndicatorModel::SubscriptionResult
    CachedOrderImbalanceIndicatorModel::load_imbalances(
    const boost::posix_time::ptime& start,
    const boost::posix_time::ptime& end,
    const OrderImbalanceSignal::slot_type& slot) {
  auto promises = std::vector<QtPromise<std::vector<OrderImbalance>>>();
  auto ranges = interval_set<ptime>(
    {continuous_interval(start, end)}) - m_ranges;
  for(auto& range : ranges) {
    auto [connection, promise] = m_source_model->subscribe(range.lower(),
        range.upper(), [this] (auto& imbalance) {
      on_order_imbalance(imbalance);
    });
    m_connections.push_back(connection);
    promises.push_back(std::move(promise));
  }
  m_subscriptions.push_back(Subscription(start, end));
  return {m_subscriptions.back().m_imbalance_signal.connect(slot),
    all(std::move(promises)).then([=] (auto& imbalances_lists) {
      m_ranges.add({start, end});
      for(auto& list : imbalances_lists.Get()) {
        std::copy(list.begin(), list.end(), std::inserter(m_imbalances,
          m_imbalances.end()));
      }
      auto requested_imbalances = std::vector<OrderImbalance>();
      std::copy_if(m_imbalances.begin(), m_imbalances.end(),
        std::back_inserter(requested_imbalances), [=] (auto& imbalance) {
          return start <= imbalance.m_timestamp &&
            imbalance.m_timestamp <= end;
        });
      return requested_imbalances;
    })};
}

void CachedOrderImbalanceIndicatorModel::on_order_imbalance(
    const OrderImbalance& imbalance) {
  m_imbalances.insert(imbalance);
  m_ranges.add({imbalance.m_timestamp, imbalance.m_timestamp});
  for(auto& subscription : m_subscriptions) {
    if(subscription.m_start_time <= imbalance.m_timestamp &&
        imbalance.m_timestamp <= subscription.m_end_time) {
      subscription.m_imbalance_signal(imbalance);
    }
  }
}

std::size_t CachedOrderImbalanceIndicatorModel::OrderImbalanceHash::operator
    ()(const OrderImbalance& imbalance) const {
  return std::hash<std::int64_t>{}(
    imbalance.m_timestamp.time_of_day().total_milliseconds());
}

CachedOrderImbalanceIndicatorModel::Subscription::Subscription(
  const boost::posix_time::ptime& start,
  const boost::posix_time::ptime& end)
  : m_start_time(start), m_end_time(end) {}
