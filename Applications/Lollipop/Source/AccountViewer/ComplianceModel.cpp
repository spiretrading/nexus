#include "Spire/AccountViewer/ComplianceModel.hpp"
#include <algorithm>
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::mp11;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace std;

ComplianceModel::ComplianceModel(Ref<UserProfile> userProfile,
    const DirectoryEntry& account)
    : m_userProfile{userProfile.get()},
      m_account{account},
      m_newComplianceEntryId{1},
      m_nextComplianceEntryId{1} {}

void ComplianceModel::Load() {
  m_schemas = m_userProfile->GetClients().get_definitions_client().
    load_compliance_rule_schemas();
  m_entries =
    m_userProfile->GetClients().get_compliance_client().load(m_account);
  m_updatedEntries.clear();
  m_deletedEntries.clear();
  m_newEntries.clear();
  m_newComplianceEntryId = 1;
  for(auto& entry : m_entries) {
    m_newComplianceEntryId =
      std::max(m_newComplianceEntryId, entry.get_id() + 1);
  }
  m_nextComplianceEntryId = m_newComplianceEntryId;
}

void ComplianceModel::Commit() {
  auto& complianceClient = m_userProfile->GetClients().get_compliance_client();
  for(auto& entry : m_updatedEntries) {
    complianceClient.update(entry);
  }
  m_updatedEntries.clear();
  for(auto& id : m_deletedEntries) {
    complianceClient.remove(id);
  }
  m_deletedEntries.clear();
  for(auto& newEntry : m_newEntries) {
    auto id = complianceClient.add(m_account, newEntry.get_state(),
      newEntry.get_schema());
    for(auto& previousEntry : m_entries) {
      if(previousEntry.get_id() == newEntry.get_id()) {
        previousEntry = ComplianceRuleEntry(id,
          previousEntry.get_directory_entry(), previousEntry.get_state(),
          previousEntry.get_schema());
        m_complianceRuleEntryIdUpdatedSignal(newEntry.get_id(), id);
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

void ComplianceModel::Remove(ComplianceRuleEntry::Id id) {
  auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
    [&] (const ComplianceRuleEntry& entry) {
      return entry.get_id() == id;
    });
  if(entryIterator == m_entries.end()) {
    return;
  }
  auto entry = std::move(*entryIterator);
  m_entries.erase(entryIterator);
  if(entry.get_id() < m_newComplianceEntryId) {
    m_deletedEntries.push_back(entry.get_id());
    auto updateIterator = std::find_if(m_updatedEntries.begin(),
      m_updatedEntries.end(),
      [&] (const ComplianceRuleEntry& entry) {
        return entry.get_id() == id;
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
      return previousEntry.get_id() == entry.get_id();
    });
  if(entryIterator == m_entries.end()) {
    return;
  }
  *entryIterator = entry;
  if(entry.get_id() < m_newComplianceEntryId) {
    auto updateIterator = std::find_if(
      m_updatedEntries.begin(), m_updatedEntries.end(),
      [&] (const ComplianceRuleEntry& previousEntry) {
        return previousEntry.get_id() == entry.get_id();
      });
    if(updateIterator == m_updatedEntries.end()) {
      m_updatedEntries.push_back(entry);
    } else {
      *updateIterator = entry;
    }
  } else {
    auto entryIterator = std::find_if(m_newEntries.begin(), m_newEntries.end(),
      [&] (const ComplianceRuleEntry& previousEntry) {
        return previousEntry.get_id() == entry.get_id();
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

bool Spire::IsWrapped(const ComplianceRuleSchema& schema) {
  if(schema.get_parameters().size() < 2) {
    return false;
  }
  auto& name = *(schema.get_parameters().rbegin() + 1);
  auto& arguments = *schema.get_parameters().rbegin();
  return name.m_name == "name" &&
    name.m_value.type() == typeid(std::string) &&
    arguments.m_name == "arguments" && arguments.m_value.which() ==
      mp_size<ComplianceValue::types>::value - 1;
}

QString Spire::GetUnwrappedName(const ComplianceRuleSchema& schema) {
  if(IsWrapped(schema)) {
    return GetUnwrappedName(unwrap(schema));
  }
  return QString::fromStdString(schema.get_name());
}
