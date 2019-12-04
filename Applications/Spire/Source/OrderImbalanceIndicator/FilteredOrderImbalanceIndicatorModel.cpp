#include "Spire/OrderImbalanceIndicator/FilteredOrderImbalanceIndicatorModel.hpp"

using namespace Nexus;
using namespace Spire;

FilteredOrderImbalanceIndicatorModel::FilteredOrderImbalanceIndicatorModel(
  std::shared_ptr<OrderImbalanceIndicatorModel> source_model,
  std::vector<Filter> filters)
  : m_source_model(std::move(source_model)),
    m_filters(std::move(filters)) {}

QtPromise<std::vector<Nexus::OrderImbalance>>
    FilteredOrderImbalanceIndicatorModel::load(
    const TimeInterval& interval) {

}

SubscriptionResult<boost::optional<Nexus::OrderImbalance>>
    FilteredOrderImbalanceIndicatorModel::subscribe(
    const OrderImbalanceSignal::slot_type& slot) {
  
}

std::shared_ptr<OrderImbalanceChartModel>
    FilteredOrderImbalanceIndicatorModel::get_chart_model(
    const Nexus::Security& security) {
  throw std::runtime_error("method not implemented");
}

bool FilteredOrderImbalanceIndicatorModel::is_imbalance_accepted(
    const Nexus::OrderImbalance& imbalance) const {
  
}

std::vector<Nexus::OrderImbalance>
    FilteredOrderImbalanceIndicatorModel::filter_imbalances(
    const std::vector<Nexus::OrderImbalance>& imbalances) const {
  
}
