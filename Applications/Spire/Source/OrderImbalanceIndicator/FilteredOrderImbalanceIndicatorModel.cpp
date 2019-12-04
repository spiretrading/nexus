#include "Spire/OrderImbalanceIndicator/FilteredOrderImbalanceIndicatorModel.hpp"

using Filter = Spire::FilteredOrderImbalanceIndicatorModel::Filter;
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

Filter Spire::make_security_list_filter(
    const std::set<Security>& security_list) {
  return [=] (const Nexus::OrderImbalance& imbalance) {
    return security_list.find(imbalance.m_security) !=
      security_list.end();
  };
}

Filter Spire::make_symbol_filter(const std::string& prefix) {
  return [=] (const Nexus::OrderImbalance& imbalance) {
    return imbalance.m_security.GetSymbol().find(prefix) == 0;
  };
}

Filter Spire::make_market_list_filter(
    const std::set<std::string>& market_list,
    const MarketDatabase& market_database) {
  return [=] (const Nexus::OrderImbalance& imbalance) {
    return market_list.find(market_database.FromCode(
      imbalance.m_security.GetMarket()).m_displayName) != market_list.end();
  };
}

Filter Spire::make_market_filter(const std::string& prefix,
    const MarketDatabase& market_database) {
  return [=] (const Nexus::OrderImbalance& imbalance) {
    return std::string(market_database.FromCode(
      imbalance.m_security.GetMarket()).m_displayName).find(prefix) == 0;
  };
}

Filter Spire::make_side_filter(Nexus::Side side) {
  return [=] (const Nexus::OrderImbalance& imbalance) {
    return imbalance.m_side == side;
  };
}

Filter Spire::make_size_filter(Quantity min, Quantity max) {
  return [=] (const Nexus::OrderImbalance& imbalance) {
    return min <= imbalance.m_size && imbalance.m_size <= max;
  };
}

Filter Spire::make_reference_price_filter(Money min, Money max) {
  return [=] (const Nexus::OrderImbalance& imbalance) {
    return min <= imbalance.m_referencePrice &&
      imbalance.m_referencePrice <= max;
  };
}

Filter Spire::make_notional_value_filter(Money min, Money max) {
  return [=] (const Nexus::OrderImbalance& imbalance) {
    return min <= imbalance.m_size * imbalance.m_referencePrice &&
      imbalance.m_size * imbalance.m_referencePrice <= max;
  };
}

bool FilteredOrderImbalanceIndicatorModel::is_imbalance_accepted(
    const Nexus::OrderImbalance& imbalance) const {
  
}

std::vector<Nexus::OrderImbalance>
    FilteredOrderImbalanceIndicatorModel::filter_imbalances(
    const std::vector<Nexus::OrderImbalance>& imbalances) const {
  
}
