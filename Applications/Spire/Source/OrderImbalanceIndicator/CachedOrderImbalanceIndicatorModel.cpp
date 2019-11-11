#include "Spire/OrderImbalanceIndicator/CachedOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"

using namespace boost::icl;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

CachedOrderImbalanceIndicatorModel::CachedOrderImbalanceIndicatorModel(
  std::shared_ptr<OrderImbalanceIndicatorModel> m_source_model)
  : m_source_model(std::move(m_source_model)) {}

std::tuple<boost::signals2::connection,
    QtPromise<std::vector<Nexus::OrderImbalance>>>
    CachedOrderImbalanceIndicatorModel::subscribe(
        const boost::posix_time::ptime& start,
        const boost::posix_time::ptime& end,
        const OrderImbalanceSignal::slot_type& slot) {
  if(contains(m_ranges, continuous_interval<ptime>::closed(start, end))) {
    return get_subscription(start, end, slot);
  }
  return load_imbalances(start, end, slot);
}

std::tuple<std::vector<Nexus::OrderImbalance>::const_iterator,
    std::vector<Nexus::OrderImbalance>::const_iterator>
    CachedOrderImbalanceIndicatorModel::get_imbalance_iterators(
      const boost::posix_time::ptime& start,
      const boost::posix_time::ptime& end) {
  auto lower = std::find_if(m_imbalances.begin(), m_imbalances.end(),
    [=] (auto& imbalance) { return imbalance.m_timestamp == start; });
  auto upper = std::find_if(m_imbalances.begin(), m_imbalances.end(),
    [=] (auto& imbalance) { return imbalance.m_timestamp == end; });
  return {lower, upper};
}

std::tuple<boost::signals2::connection,
    QtPromise<std::vector<Nexus::OrderImbalance>>>
    CachedOrderImbalanceIndicatorModel::get_subscription(
      const boost::posix_time::ptime& start,
      const boost::posix_time::ptime& end,
      const OrderImbalanceSignal::slot_type& slot) {
  m_signals.emplace_back(OrderImbalanceSignalConnection{
    OrderImbalanceSignal(), start, end});
  auto [lower, upper] = get_imbalance_iterators(start, end);
  return {m_signals.back().m_imbalance_signal.connect(slot),
    QtPromise([=] { return std::vector<OrderImbalance>(lower, upper); })};
}

std::tuple<boost::signals2::connection,
    QtPromise<std::vector<Nexus::OrderImbalance>>>
    CachedOrderImbalanceIndicatorModel::load_imbalances(
      const boost::posix_time::ptime& start,
      const boost::posix_time::ptime& end,
      const OrderImbalanceSignal::slot_type& slot) {
  auto [connection, promise] = m_source_model->subscribe(start, end,
    [=] (auto& imbalance) { on_order_imbalance(imbalance); });
  m_connections.push_back(std::move(connection));
  m_signals.emplace_back(OrderImbalanceSignalConnection{
    OrderImbalanceSignal(), start, end});
  return {m_signals.back().m_imbalance_signal.connect(slot),
    promise.then([=] (auto& imbalances) {
        auto [lower, upper] = get_imbalance_iterators(start, end);
        return std::vector<OrderImbalance>(lower, upper);
      })};
}

void CachedOrderImbalanceIndicatorModel::on_order_imbalance(
    const OrderImbalance& imbalance) {
  if(std::find(m_imbalances.rbegin(), m_imbalances.rend(), imbalance) !=
      m_imbalances.rend()) {
    return;
  }
  m_ranges.add(continuous_interval<ptime>::closed(imbalance.m_timestamp,
    imbalance.m_timestamp));
  m_imbalances.insert(std::lower_bound(m_imbalances.begin(),
    m_imbalances.end(), imbalance,
      [] (auto& current, auto& value) {
        return current.m_timestamp < value.m_timestamp; }), imbalance);
  for(auto& signal : m_signals) {
    if(signal.m_start_time <= imbalance.m_timestamp &&
        imbalance.m_timestamp <= signal.m_end_time) {
      signal.m_imbalance_signal(imbalance);
    }
  }
}
