#include "ClientWebPortal/ClientWebPortal/PortfolioModel.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::ClientWebPortal;
using namespace Nexus::RiskService;
using namespace std;

PortfolioModel::Entry::Entry(Beam::ServiceLocator::DirectoryEntry account,
    Security security, CurrencyId currency)
    : m_account{std::move(account)},
      m_security{std::move(security)},
      m_currency{currency} {}

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
