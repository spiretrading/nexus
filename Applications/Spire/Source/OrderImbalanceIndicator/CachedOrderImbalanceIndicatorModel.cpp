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

std::tuple<boost::signals2::connection,
    QtPromise<std::vector<Nexus::OrderImbalance>>>
    CachedOrderImbalanceIndicatorModel::get_subscription(
      const boost::posix_time::ptime& start,
      const boost::posix_time::ptime& end,
      const OrderImbalanceSignal::slot_type& slot) {
  m_signals.emplace_back(OrderImbalanceSignalConnection{
    OrderImbalanceSignal(), start, end});
  auto imbalances = std::vector<OrderImbalance>();
  std::copy_if(m_imbalances.begin(), m_imbalances.end(),
    std::back_inserter(imbalances), [=] (auto& imbalance) {
        return start <= imbalance.m_timestamp && imbalance.m_timestamp <= end;
      });
  return {m_signals.back().m_imbalance_signal.connect(slot),
    QtPromise([requested_imbalances = std::move(imbalances)] {
        return requested_imbalances;
      })};
}

std::tuple<boost::signals2::connection,
    QtPromise<std::vector<Nexus::OrderImbalance>>>
    CachedOrderImbalanceIndicatorModel::load_imbalances(
      const boost::posix_time::ptime& start,
      const boost::posix_time::ptime& end,
      const OrderImbalanceSignal::slot_type& slot) {
  auto promises = std::vector<QtPromise<std::vector<OrderImbalance>>>();
  auto ranges = interval_set<ptime>(
    {continuous_interval(start, end)}) - m_ranges;
  for(auto& range : ranges) {
    auto [connection, promise] = m_source_model->subscribe(range.lower(),
      range.upper(), [=] (auto& imbalance) { on_order_imbalance(imbalance); });
    promises.push_back(std::move(promise));
  }
  m_ranges.add({start, end});
  m_signals.emplace_back(OrderImbalanceSignalConnection{
    OrderImbalanceSignal(), start, end});
  return {m_signals.back().m_imbalance_signal.connect(slot),
    all(std::move(promises)).then(
      [=, &imbalances = m_imbalances] (auto& imbalances_lists) {
          for(auto& list : imbalances_lists.Get()) {
            std::copy(list.begin(), list.end(), std::inserter(imbalances,
              imbalances.end()));
          }
          auto requested_imbalances = std::vector<OrderImbalance>();
          std::copy_if(imbalances.begin(), imbalances.end(),
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
  for(auto& signal : m_signals) {
    if(signal.m_start_time <= imbalance.m_timestamp &&
        imbalance.m_timestamp <= signal.m_end_time) {
      signal.m_imbalance_signal(imbalance);
    }
  }
}
