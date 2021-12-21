#include "WebPortal/PortfolioModel.hpp"
#include <boost/functional/hash.hpp>
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::MarketDataService;
using namespace Nexus::RiskService;
using namespace Nexus::WebPortal;

PortfolioModel::Entry::Entry(Beam::ServiceLocator::DirectoryEntry account,
  Security security, CurrencyId currency)
  : m_account(std::move(account)),
    m_inventory({security, currency}) {}

bool PortfolioModel::Entry::operator ==(const Entry& rhs) const {
  return std::tie(m_account, m_inventory.m_position.m_key) ==
    std::tie(rhs.m_account, rhs.m_inventory.m_position.m_key);
}

PortfolioModel::PortfolioModel(ServiceClientsBox serviceClients)
    : m_serviceClients(std::move(serviceClients)) {
  m_serviceClients.GetRiskClient().GetRiskPortfolioUpdatePublisher().Monitor(
    m_tasks.GetSlot<RiskInventoryEntry>(
      std::bind_front(&PortfolioModel::OnRiskPortfolioInventoryUpdate, this)));
}

PortfolioModel::~PortfolioModel() {
  Close();
}

const Publisher<PortfolioModel::Entry>& PortfolioModel::GetPublisher() const {
  return m_publisher;
}

void PortfolioModel::Close() {}

void PortfolioModel::OnRiskPortfolioInventoryUpdate(
    const RiskInventoryEntry& inventory) {
  auto& security = inventory.m_key.m_security;
  auto entryIterator = m_entries.find(inventory.m_key);
  if(entryIterator == m_entries.end()) {
    auto entry = std::make_shared<Entry>(inventory.m_key.m_account,
      inventory.m_key.m_security,
      inventory.m_value.m_position.m_key.m_currency);
    entryIterator = m_entries.insert(std::pair(inventory.m_key, entry)).first;
    m_securityToEntries[security].push_back(entry);
  }
  auto& valuation =
    [&] () -> SecurityValuation& {
      auto valuationIterator = m_valuations.find(security);
      if(valuationIterator == m_valuations.end()) {
        auto query = Beam::Queries::MakeCurrentQuery(security);
        valuationIterator = m_valuations.insert(std::make_pair(security,
          SecurityValuation(
            inventory.m_value.m_position.m_key.m_currency))).first;
        m_serviceClients.GetMarketDataClient().QueryBboQuotes(query,
          m_tasks.GetSlot<BboQuote>(std::bind_front(&PortfolioModel::OnBboQuote,
            this, security, std::ref(valuationIterator->second))));
      }
      return valuationIterator->second;
    }();
  auto entry = entryIterator->second;
  entry->m_inventory = inventory.m_value;
  entry->m_unrealizedProfitAndLoss = GetUnrealizedProfitAndLoss(
    entry->m_inventory, valuation);
  m_publisher.Push(*entry);
}

void PortfolioModel::OnBboQuote(const Security& security,
    SecurityValuation& valuation, const BboQuote& quote) {
  if(valuation.m_bidValue == quote.m_bid.m_price &&
      valuation.m_askValue == quote.m_ask.m_price) {
    return;
  }
  valuation.m_bidValue = quote.m_bid.m_price;
  valuation.m_askValue = quote.m_ask.m_price;
  for(auto& entry : m_securityToEntries[security]) {
    entry->m_unrealizedProfitAndLoss = GetUnrealizedProfitAndLoss(
      entry->m_inventory, valuation);
    m_publisher.Push(*entry);
  }
}

std::size_t std::hash<PortfolioModel::Entry>::operator()(
    const PortfolioModel::Entry& value) const {
  auto seed = std::size_t(0);
  boost::hash_combine(seed, value.m_account);
  boost::hash_combine(seed, value.m_inventory.m_position.m_key.m_index);
  return seed;
}
