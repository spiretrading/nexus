#include "Spire/KeyBindings/KeyBindingsModel.hpp"

using namespace Nexus;
using namespace Spire;

KeyBindingsModel::KeyBindingsModel(MarketDatabase markets)
  : m_markets(std::move(markets)),
    m_order_task_arguments(
      std::make_shared<ArrayListModel<OrderTaskArguments>>()),
    m_cancel_key_bindings(std::make_shared<CancelKeyBindingsModel>()),
    m_global_interactions(
      std::make_shared<InteractionsKeyBindingsModel>()) {}

const std::shared_ptr<OrderTaskArgumentsListModel>&
    KeyBindingsModel::get_order_task_arguments() const {
  return m_order_task_arguments;
}

const std::shared_ptr<CancelKeyBindingsModel>&
    KeyBindingsModel::get_cancel_key_bindings() const {
  return m_cancel_key_bindings;
}

const std::shared_ptr<InteractionsKeyBindingsModel>&
    KeyBindingsModel::get_interactions_key_bindings() const {
  return m_global_interactions;
}

const std::shared_ptr<InteractionsKeyBindingsModel>&
    KeyBindingsModel::get_interactions_key_bindings(CountryCode country) const {
  auto i = m_country_interactions.find(country);
  if(i == m_country_interactions.end()) {
    auto interactions = std::make_shared<InteractionsKeyBindingsModel>(
      get_interactions_key_bindings());
    i = m_country_interactions.insert(std::pair(country, interactions)).first;
  }
  return i->second;
}

const std::shared_ptr<InteractionsKeyBindingsModel>&
    KeyBindingsModel::get_interactions_key_bindings(MarketCode market) const {
  auto i = m_market_interactions.find(market);
  if(i == m_market_interactions.end()) {
    auto interactions = std::make_shared<InteractionsKeyBindingsModel>(
      get_interactions_key_bindings(m_markets.FromCode(market).m_countryCode));
    i = m_market_interactions.insert(std::pair(market, interactions)).first;
  }
  return i->second;
}

const std::shared_ptr<InteractionsKeyBindingsModel>&
    KeyBindingsModel::get_interactions_key_bindings(
      const Security& security) const {
  auto i = m_security_interactions.find(security);
  if(i == m_security_interactions.end()) {
    auto interactions = std::make_shared<InteractionsKeyBindingsModel>(
      get_interactions_key_bindings(security.GetMarket()));
    i = m_security_interactions.insert(std::pair(security, interactions)).first;
  }
  return i->second;
}
