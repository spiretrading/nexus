#include "Spire/AccountViewer/ComplianceModel.hpp"
#include <algorithm>
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Spire;
using namespace std;

ComplianceModel::ComplianceModel(Ref<UserProfile> userProfile,
    const DirectoryEntry& account)
    : m_userProfile{userProfile.Get()},
      m_account{account},
      m_newComplianceEntryId{1},
      m_nextComplianceEntryId{1} {}

void ComplianceModel::Load() {
  m_schemas = m_userProfile->GetServiceClients().GetDefinitionsClient().
    LoadComplianceRuleSchemas();
  m_entries = m_userProfile->GetServiceClients().GetComplianceClient().Load(
    m_account);
  m_updatedEntries.clear();
  m_deletedEntries.clear();
  m_newEntries.clear();
  m_newComplianceEntryId = 1;
  for(auto& entry : m_entries) {
    m_newComplianceEntryId = std::max(m_newComplianceEntryId,
      entry.GetId() + 1);
  }
  m_nextComplianceEntryId = m_newComplianceEntryId;
}

void ComplianceModel::Commit() {
  auto& complianceClient =
    m_userProfile->GetServiceClients().GetComplianceClient();
  for(auto& entry : m_updatedEntries) {
    complianceClient.Update(entry);
  }
  m_updatedEntries.clear();
  for(auto& id : m_deletedEntries) {
    complianceClient.Delete(id);
  }
  m_deletedEntries.clear();
  for(auto& newEntry : m_newEntries) {
    auto id = complianceClient.Add(m_account, newEntry.GetState(),
      newEntry.GetSchema());
    for(auto& previousEntry : m_entries) {
      if(previousEntry.GetId() == newEntry.GetId()) {
        previousEntry.SetId(id);
        m_complianceRuleEntryIdUpdatedSignal(newEntry.GetId(), id);
      }
    }
  }
  m_newEntries.clear();
}

const DirectoryEntry& ComplianceModel::GetAccount() const {
  return m_account;
}

const vector<ComplianceRuleSchema>& ComplianceModel::GetSchemas() const {
  return m_schemas;
}

const vector<ComplianceRuleEntry>& ComplianceModel::GetEntries() const {
  return m_entries;
}

void ComplianceModel::Add(const ComplianceRuleSchema& schema) {
  ComplianceRuleEntry entry{m_nextComplianceEntryId, m_account,
    ComplianceRuleEntry::State::ACTIVE, schema};
  ++m_nextComplianceEntryId;
  m_entries.push_back(entry);
  m_newEntries.push_back(entry);
  m_complianceRuleEntryAddedSignal(entry);
}

void ComplianceModel::Remove(ComplianceRuleId id) {
  auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
    [&] (const ComplianceRuleEntry& entry) {
      return entry.GetId() == id;
    });
  if(entryIterator == m_entries.end()) {
    return;
  }
  auto entry = std::move(*entryIterator);
  m_entries.erase(entryIterator);
  if(entry.GetId() < m_newComplianceEntryId) {
    m_deletedEntries.push_back(entry.GetId());
    auto updateIterator = std::find_if(m_updatedEntries.begin(),
      m_updatedEntries.end(),
      [&] (const ComplianceRuleEntry& entry) {
        return entry.GetId() == id;
      });
    if(updateIterator != m_updatedEntries.end()) {
      m_updatedEntries.erase(updateIterator);
    }
  }
  m_complianceRuleEntryRemovedSignal(entry);
}

void ComplianceModel::Update(const ComplianceRuleEntry& entry) {
  auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
    [&] (const ComplianceRuleEntry& previousEntry) {
      return previousEntry.GetId() == entry.GetId();
    });
  if(entryIterator == m_entries.end()) {
    return;
  }
  *entryIterator = entry;
  if(entry.GetId() < m_newComplianceEntryId) {
    auto updateIterator = std::find_if(
      m_updatedEntries.begin(), m_updatedEntries.end(),
      [&] (const ComplianceRuleEntry& previousEntry) {
        return previousEntry.GetId() == entry.GetId();
      });
    if(updateIterator == m_updatedEntries.end()) {
      m_updatedEntries.push_back(entry);
    } else {
      *updateIterator = entry;
    }
  } else {
    auto entryIterator = std::find_if(m_newEntries.begin(), m_newEntries.end(),
      [&] (const ComplianceRuleEntry& previousEntry) {
        return previousEntry.GetId() == entry.GetId();
      });
    if(entryIterator == m_newEntries.end()) {
      return;
    }
    *entryIterator = entry;
  }
}

connection ComplianceModel::ConnectComplianceRuleEntryAddedSignal(
    const ComplianceRuleEntryAddedSignal::slot_type& slot) const {
  return m_complianceRuleEntryAddedSignal.connect(slot);
}

connection ComplianceModel::ConnectComplianceRuleEntryRemovedSignal(
    const ComplianceRuleEntryAddedSignal::slot_type& slot) const {
  return m_complianceRuleEntryRemovedSignal.connect(slot);
}

connection ComplianceModel::ConnectComplianceRuleEntryIdUpdatedSignal(
    const ComplianceRuleEntryIdUpdatedSignal::slot_type& slot) const {
  return m_complianceRuleEntryIdUpdatedSignal.connect(slot);
}
