#include "Spire/OrderImbalanceIndicator/FilteredOrderImbalanceIndicatorModel.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Spire/Spire/QtPromise.hpp"

using namespace Nexus;
using namespace Spire;

FilteredOrderImbalanceIndicatorModel::FilteredOrderImbalanceIndicatorModel(
    std::shared_ptr<OrderImbalanceIndicatorModel> source_model,
    std::vector<std::function<bool (const OrderImbalance&)>> filters)
    : m_source_model(std::move(source_model)),
      m_filters(std::move(filters)) {}

std::tuple<boost::signals2::connection,
    QtPromise<std::vector<Nexus::OrderImbalance>>>
    FilteredOrderImbalanceIndicatorModel::subscribe(
      const boost::posix_time::ptime& start,
      const boost::posix_time::ptime& end,
      const OrderImbalanceSignal::slot_type& slot) {
  auto signal = std::make_unique<FilteredOrderImbalanceSignalConnection>(
    FilteredOrderImbalanceSignalConnection{
      {OrderImbalanceSignal(), start, end}, {}});
  auto callback = [this, signal = signal.get()] (auto& imbalance) {
      if(is_imbalance_accepted(imbalance)) {
        signal->m_imbalance_signal(imbalance);
    }};
  m_signals.push_back(std::move(signal));
  auto [connection, promise] = m_source_model->subscribe(start, end,
    std::move(callback));
  m_signals.back()->m_source_signal_connection = connection;
  return {m_signals.back()->m_imbalance_signal.connect(slot),
        promise.then([=] (auto& imbalances) {
      return filter_imbalances(imbalances);
    })};
}

std::function<bool (const Nexus::OrderImbalance&)>
    Spire::make_security_list_filter(
      const std::set<std::string>& symbol_list) {
  return {[=] (const Nexus::OrderImbalance& imbalance) {
      return symbol_list.find(imbalance.m_security.GetSymbol()) !=
        symbol_list.end();
    }};
}

std::function<bool (const Nexus::OrderImbalance&)> Spire::make_security_filter(
    const std::string& filter_string) {
  return {[=] (const Nexus::OrderImbalance& imbalance) {
      return imbalance.m_security.GetSymbol().find(filter_string) == 0;
    }};
}

std::function<bool (const Nexus::OrderImbalance&)>
    Spire::make_market_list_filter(
      const std::set<std::string>& market_list) {
  return {[=, market_database = GetDefaultMarketDatabase()] (
        const Nexus::OrderImbalance& imbalance) {
      return market_list.find(market_database.FromCode(
          imbalance.m_security.GetMarket()).m_displayName) !=
          market_list.end();
    }};
}

std::function<bool (const Nexus::OrderImbalance&)> Spire::make_market_filter(
    const std::string& filter_string) {
  return {[=, market_database = GetDefaultMarketDatabase()] (
        const Nexus::OrderImbalance& imbalance) {
      return std::string(market_database.FromCode(
          imbalance.m_security.GetMarket()).m_displayName)
        .find(filter_string) == 0;
    }};
}

std::function<bool (const Nexus::OrderImbalance&)> Spire::make_side_filter(
    Nexus::Side side) {
  return {[=] (const Nexus::OrderImbalance& imbalance) {
      return imbalance.m_side == side;
    }};
}

std::function<bool (const Nexus::OrderImbalance&)> Spire::make_size_filter(
    const Nexus::Quantity& min, const Nexus::Quantity& max) {
  return {[=] (const Nexus::OrderImbalance& imbalance) {
      return min <= imbalance.m_size && imbalance.m_size <= max;
    }};
}

std::function<bool (const Nexus::OrderImbalance&)>
    Spire::make_reference_price_filter(const Nexus::Money& min,
      const Nexus::Money& max) {
  return {[=] (const Nexus::OrderImbalance& imbalance) {
      return min <= imbalance.m_referencePrice &&
        imbalance.m_referencePrice <= max;
    }};
}

std::function<bool (const Nexus::OrderImbalance&)>
    Spire::make_notional_value_filter(const Nexus::Money& min,
      const Nexus::Money& max) {
  return {[=] (const Nexus::OrderImbalance& imbalance) {
      return min <= imbalance.m_size * imbalance.m_referencePrice &&
        imbalance.m_size * imbalance.m_referencePrice <= max;
    }};
}

bool FilteredOrderImbalanceIndicatorModel::is_imbalance_accepted(
    const Nexus::OrderImbalance& imbalance) {
  for(auto& filter : m_filters) {
    if(!filter(imbalance)) {
      return false;
    }
  }
  return true;
}

std::vector<Nexus::OrderImbalance>
    FilteredOrderImbalanceIndicatorModel::filter_imbalances(
    const std::vector<Nexus::OrderImbalance>& imbalances) {
  auto filtered_imbalances = std::vector<Nexus::OrderImbalance>();
  for(auto& imbalance : imbalances) {
    if(is_imbalance_accepted(imbalance)) {
      filtered_imbalances.push_back(imbalance);
    }
  }
  return filtered_imbalances;
}
