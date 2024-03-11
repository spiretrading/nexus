#include "Spire/KeyBindings/KeyBindingsModel.hpp"

using namespace Nexus;
using namespace Spire;

KeyBindingsModel::KeyBindingsModel(MarketDatabase markets)
    : m_markets(std::move(markets)),
      m_order_task_arguments(
        std::make_shared<ArrayListModel<OrderTaskArguments>>()),
      m_cancel_key_bindings(std::make_shared<CancelKeyBindingsModel>()) {
  m_interactions.insert(std::pair(
    Region::Global(QObject::tr("Global").toStdString()),
    std::make_shared<InteractionsKeyBindingsModel>()));
}

const std::shared_ptr<OrderTaskArgumentsListModel>&
    KeyBindingsModel::get_order_task_arguments() const {
  return m_order_task_arguments;
}

const std::shared_ptr<CancelKeyBindingsModel>&
    KeyBindingsModel::get_cancel_key_bindings() const {
  return m_cancel_key_bindings;
}

const std::shared_ptr<InteractionsKeyBindingsModel>&
    KeyBindingsModel::get_interactions_key_bindings(
      const Region& region) const {
  if(region.GetCountries().size() == 1) {
    auto country = *region.GetCountries().begin();
    auto i = m_interactions.find(country);
    if(i == m_interactions.end()) {
      auto interactions = std::make_shared<InteractionsKeyBindingsModel>(
        get_interactions_key_bindings(Region::Global()));
      i = m_interactions.insert(std::pair(country, interactions)).first;
    }
    return i->second;
  } else if(region.GetMarkets().size() == 1) {
    auto market = m_markets.FromCode(*region.GetMarkets().begin());
    auto i = m_interactions.find(market);
    if(i == m_interactions.end()) {
      auto interactions = std::make_shared<InteractionsKeyBindingsModel>(
        get_interactions_key_bindings(market.m_countryCode));
      i = m_interactions.insert(std::pair(market, interactions)).first;
    }
    return i->second;
  } else if(region.GetSecurities().size() == 1) {
    auto security = *region.GetSecurities().begin();
    auto i = m_interactions.find(security);
    if(i == m_interactions.end()) {
      auto interactions = std::make_shared<InteractionsKeyBindingsModel>(
        get_interactions_key_bindings(
          m_markets.FromCode(security.GetMarket())));
      i = m_interactions.insert(std::pair(security, interactions)).first;
    }
    return i->second;
  } else if(region.IsGlobal()) {
    return m_interactions.at(region);
  }
  static auto NONE = std::shared_ptr<InteractionsKeyBindingsModel>();
  return NONE;
}

std::vector<Region>
    KeyBindingsModel::make_interactions_key_bindings_regions() const {
  auto regions = std::vector<Region>();
  for(auto& entry : m_interactions) {
    if(entry.second->is_detached()) {
      regions.push_back(entry.first);
    }
  }
  return regions;
}
