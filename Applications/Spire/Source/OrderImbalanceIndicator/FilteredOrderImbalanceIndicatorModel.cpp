#include "Spire/OrderImbalanceIndicator/FilteredOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"

using namespace Nexus;
using namespace Spire;

FilteredOrderImbalanceIndicatorModel::FilteredOrderImbalanceIndicatorModel(
    std::shared_ptr<OrderImbalanceIndicatorModel> source_model,
    const std::vector<std::function<bool (const OrderImbalance&)>>& filters)
    : m_source_model(source_model),
      m_filters(filters) {}

std::tuple<boost::signals2::connection,
    QtPromise<std::vector<Nexus::OrderImbalance>>>
    FilteredOrderImbalanceIndicatorModel::subscribe(
      const boost::posix_time::ptime& start,
      const boost::posix_time::ptime& end,
      const OrderImbalanceSignal::slot_type& slot) {
  m_signals.emplace_back(OrderImbalanceSignalConnection{
    OrderImbalanceSignal(), start, end});
  auto callback = [&] (auto& imbalance) {
      if(!filter_imbalances({imbalance}).empty()) {
        m_signals.back().m_imbalance_signal(imbalance);
    }};
  // should connection be stored?
  auto [connection, promise] = m_source_model->subscribe(start, end,
    callback);
  return {m_signals.back().m_imbalance_signal.connect(slot),
        promise.then([=] (auto& imbalances) {
      return std::move(filter_imbalances(imbalances));
    })};
}

std::function<bool (const Nexus::OrderImbalance&)> Spire::make_list_filter(
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

std::function<bool (const Nexus::OrderImbalance&)> Spire::make_market_filter(
    const std::string& filter_string) {
  return {[=] (const Nexus::OrderImbalance& imbalance) {
      return std::string(imbalance.m_security.GetMarket().GetData())
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
    Spire::make_time_filter(const boost::posix_time::ptime& min,
      const boost::posix_time::ptime& max) {
  return {[=] (const Nexus::OrderImbalance& imbalance) {
      return min <= imbalance.m_timestamp &&
        imbalance.m_timestamp <= max;
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
      filtered_imbalances.emplace_back(std::move(imbalance));
    }
  }
  return std::move(filtered_imbalances);
}
