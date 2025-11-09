#include "WebPortal/PortfolioModel.hpp"
#include <boost/functional/hash.hpp>
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

PortfolioModel::Entry::Entry(
  Beam::DirectoryEntry m_account, Security security, CurrencyId currency)
  : m_account(std::move(m_account)),
    m_inventory(std::move(security), currency) {}

bool PortfolioModel::Entry::operator ==(const Entry& rhs) const {
  return std::tie(m_account, m_inventory.m_position.m_security,
    m_inventory.m_position.m_currency) ==
      std::tie(rhs.m_account, rhs.m_inventory.m_position.m_security,
        rhs.m_inventory.m_position.m_currency);
}

PortfolioModel::PortfolioModel(Clients clients)
  : m_clients(std::move(clients)) {
  m_clients.get_risk_client().get_risk_portfolio_update_publisher().monitor(
    m_tasks.get_slot<RiskInventoryEntry>(std::bind_front(
      &PortfolioModel::on_risk_portfolio_inventory_update, this)));
}

PortfolioModel::~PortfolioModel() {
  close();
}

const Publisher<PortfolioModel::Entry>& PortfolioModel::get_publisher() const {
  return m_publisher;
}

void PortfolioModel::close() {}

void PortfolioModel::on_risk_portfolio_inventory_update(
    const RiskInventoryEntry& m_inventory) {
  auto& security = m_inventory.m_key.m_security;
  auto entry_iterator = m_entries.find(m_inventory.m_key);
  if(entry_iterator == m_entries.end()) {
    auto entry = std::make_shared<Entry>(m_inventory.m_key.m_account,
      m_inventory.m_key.m_security, m_inventory.m_value.m_position.m_currency);
    entry_iterator =
      m_entries.insert(std::pair(m_inventory.m_key, entry)).first;
    m_security_to_entries[security].push_back(entry);
  }
  auto& valuation = [&] () -> SecurityValuation& {
    auto valuation_iterator = m_valuations.find(security);
    if(valuation_iterator == m_valuations.end()) {
      auto query = make_current_query(security);
      valuation_iterator = m_valuations.insert(std::pair(security,
        SecurityValuation(m_inventory.m_value.m_position.m_currency))).first;
      m_clients.get_market_data_client().query(query,
        m_tasks.get_slot<BboQuote>(
          std::bind_front(&PortfolioModel::on_bbo_quote,
            this, security, std::ref(valuation_iterator->second))));
    }
    return valuation_iterator->second;
  }();
  auto entry = entry_iterator->second;
  entry->m_inventory = m_inventory.m_value;
  entry->m_unrealized_profit_and_loss =
    get_unrealized_profit_and_loss(entry->m_inventory, valuation);
  m_publisher.push(*entry);
}

void PortfolioModel::on_bbo_quote(const Security& security,
    SecurityValuation& valuation, const BboQuote& quote) {
  if(valuation.m_bid_value == quote.m_bid.m_price &&
      valuation.m_ask_value == quote.m_ask.m_price) {
    return;
  }
  valuation.m_bid_value = quote.m_bid.m_price;
  valuation.m_ask_value = quote.m_ask.m_price;
  for(auto& entry : m_security_to_entries[security]) {
    entry->m_unrealized_profit_and_loss =
      get_unrealized_profit_and_loss(entry->m_inventory, valuation);
    m_publisher.push(*entry);
  }
}

std::size_t std::hash<PortfolioModel::Entry>::operator()(
    const PortfolioModel::Entry& value) const {
  auto seed = std::size_t(0);
  boost::hash_combine(seed, value.m_account);
  boost::hash_combine(seed, value.m_inventory.m_position.m_security);
  return seed;
}
