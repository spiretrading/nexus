#include "ClientWebPortal/ClientWebPortal/PortfolioModel.hpp"
#include <boost/functional/hash.hpp>

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Nexus;
using namespace Nexus::ClientWebPortal;
using namespace Nexus::RiskService;
using namespace std;

PortfolioModel::Entry::Entry(Beam::ServiceLocator::DirectoryEntry account,
    Security security, CurrencyId currency)
    : m_account{std::move(account)},
      m_security{std::move(security)},
      m_currency{currency} {}

bool PortfolioModel::Entry::operator ==(const Entry& rhs) const {
  return tie(m_account, m_security, m_currency) ==
    tie(rhs.m_account, rhs.m_security, rhs.m_currency);
}

PortfolioModel::PortfolioModel(
    RefType<ApplicationServiceClients> serviceClients)
    : m_serviceClients{serviceClients.Get()} {}

PortfolioModel::~PortfolioModel() {
  Close();
}

void PortfolioModel::Open() {
  m_serviceClients->GetRiskClient().GetRiskPortfolioUpdatePublisher().Monitor(
    m_tasks.GetSlot<RiskPortfolioInventoryEntry>(
    std::bind(&PortfolioModel::OnRiskPortfolioInventoryUpdate, this,
    std::placeholders::_1)));
}

void PortfolioModel::Close() {}

const Publisher<PortfolioModel::Entry>& PortfolioModel::GetPublisher() const {
  return m_publisher;
}

void PortfolioModel::OnRiskPortfolioInventoryUpdate(
    const RiskPortfolioInventoryEntry& inventory) {
  auto& security = inventory.m_key.m_security;
  auto entryIterator = m_entries.find(inventory.m_key);
  if(entryIterator == m_entries.end()) {
    auto entry = std::make_shared<Entry>(inventory.m_key.m_account,
      inventory.m_key.m_security,
      inventory.m_value.m_position.m_key.m_currency);
    entryIterator = m_entries.insert(
      std::make_pair(inventory.m_key, entry)).first;
    m_securityToEntries[security].push_back(entry);
  }
  auto entry = entryIterator->second;
  entry->m_openQuantity = inventory.m_value.m_position.m_quantity;
  entry->m_averagePrice = GetAveragePrice(inventory.m_value.m_position);
  entry->m_realizedProfitAndLoss = GetRealizedProfitAndLoss(inventory.m_value);
  entry->m_fees = inventory.m_value.m_fees;
  entry->m_costBasis = inventory.m_value.m_position.m_costBasis;
  entry->m_volume = inventory.m_value.m_volume;
  entry->m_trades = inventory.m_value.m_transactionCount;
  m_publisher.Push(*entry);
}

size_t std::hash<PortfolioModel::Entry>::operator()(
    const PortfolioModel::Entry& value) const {
  std::size_t seed = 0;
  boost::hash_combine(seed, value.m_account);
  boost::hash_combine(seed, value.m_security);
  boost::hash_combine(seed, value.m_currency);
  return seed;
}
