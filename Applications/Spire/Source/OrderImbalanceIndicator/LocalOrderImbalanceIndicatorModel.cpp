#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"

using namespace Nexus;
using namespace Spire;

std::tuple<boost::signals2::connection,
    QtPromise<std::vector<Nexus::OrderImbalance>>>
      LocalOrderImbalanceIndicatorModel::subscribe(
        const boost::posix_time::ptime& start,
        const boost::posix_time::ptime& end,
        const OrderImbalanceSignal::slot_type& slot) {
  m_signals.emplace_back(OrderImbalanceSignalConnection{
      OrderImbalanceSignal(), start, end});
  return std::tuple<boost::signals2::connection,
    QtPromise<std::vector<OrderImbalance>>>(
      m_signals.back().m_imbalance_signal.connect(slot),
      QtPromise([=, &imbalances = m_imbalances] {
        auto requested_imbalances = std::vector<OrderImbalance>();
        for(auto& imbalance : imbalances) {
          if(imbalance.m_timestamp >= start &&
              imbalance.m_timestamp <= end) {
            requested_imbalances.push_back(imbalance);
          }
        }
        return std::move(requested_imbalances);
    }, LaunchPolicy::ASYNC));
}

void LocalOrderImbalanceIndicatorModel::insert(
    const OrderImbalance& imbalance) {
  m_imbalances.push_back(imbalance);
  for(auto& signal : m_signals) {
    if(signal.m_start_time <= imbalance.m_timestamp &&
        imbalance.m_timestamp <= signal.m_end_time) {
      signal.m_imbalance_signal(imbalance);
    }
  }
}
