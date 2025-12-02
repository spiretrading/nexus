#ifndef NEXUS_LOCAL_COMPLIANCE_RULE_DATA_STORE_HPP
#define NEXUS_LOCAL_COMPLIANCE_RULE_DATA_STORE_HPP
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"

namespace Nexus {

  /** Implements a ComplianceRuleDataStore in memory. */
  class LocalComplianceRuleDataStore {
    public:

      /** Constructs an empty LocalComplianceRuleDataStore. */
      LocalComplianceRuleDataStore() = default;

      ~LocalComplianceRuleDataStore();

      std::vector<ComplianceRuleEntry> load_all_compliance_rule_entries();
      ComplianceRuleEntry::Id load_next_compliance_rule_entry_id();
      boost::optional<ComplianceRuleEntry>
        load_compliance_rule_entry(ComplianceRuleEntry::Id id);
      std::vector<ComplianceRuleEntry> load_compliance_rule_entries(
        const Beam::DirectoryEntry& directory_entry);
      void store(const ComplianceRuleEntry& entry);
      void remove(ComplianceRuleEntry::Id id);
      void store(const ComplianceRuleViolationRecord& record);
      void close();

    private:
      mutable boost::mutex m_mutex;
      std::unordered_map<ComplianceRuleEntry::Id,
        std::shared_ptr<ComplianceRuleEntry>> m_entries_by_id;
      std::unordered_map<Beam::DirectoryEntry,
        std::vector<std::shared_ptr<ComplianceRuleEntry>>>
          m_entries_by_directory_entry;
      Beam::OpenState m_open_state;

      LocalComplianceRuleDataStore(
        const LocalComplianceRuleDataStore&) = delete;
      LocalComplianceRuleDataStore& operator =(
        const LocalComplianceRuleDataStore&) = delete;
  };

  inline LocalComplianceRuleDataStore::~LocalComplianceRuleDataStore() {
    close();
  }

  inline std::vector<ComplianceRuleEntry>
      LocalComplianceRuleDataStore::load_all_compliance_rule_entries() {
    auto entries = std::vector<ComplianceRuleEntry>();
    auto lock = boost::lock_guard(m_mutex);
    entries.reserve(m_entries_by_id.size());
    std::transform(m_entries_by_id.begin(), m_entries_by_id.end(),
      std::back_inserter(entries), [] (const auto& entry) {
        return *entry.second;
      });
    return entries;
  }

  inline ComplianceRuleEntry::Id
      LocalComplianceRuleDataStore::load_next_compliance_rule_entry_id() {
    auto id = ComplianceRuleEntry::Id(1);
    auto lock = boost::lock_guard(m_mutex);
    for(auto& entry : m_entries_by_id) {
      id = std::max(entry.second->get_id() + 1, id);
    }
    return id;
  }

  inline boost::optional<ComplianceRuleEntry>
      LocalComplianceRuleDataStore::load_compliance_rule_entry(
        ComplianceRuleEntry::Id id) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_entries_by_id.find(id);
    if(i == m_entries_by_id.end()) {
      return boost::none;
    }
    return *i->second;
  }

  inline std::vector<ComplianceRuleEntry>
      LocalComplianceRuleDataStore::load_compliance_rule_entries(
        const Beam::DirectoryEntry& directory_entry) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_entries_by_directory_entry.find(directory_entry);
    if(i == m_entries_by_directory_entry.end()) {
      return {};
    }
    auto& entries = i->second;
    auto result = std::vector<ComplianceRuleEntry>();
    result.reserve(entries.size());
    std::transform(entries.begin(), entries.end(), std::back_inserter(result),
      [] (const auto& entry) {
        return *entry;
      });
    return result;
  }

  inline void LocalComplianceRuleDataStore::store(
      const ComplianceRuleEntry& entry) {
    auto new_entry = std::make_shared<ComplianceRuleEntry>(entry);
    auto lock = boost::lock_guard(m_mutex);
    auto& previous_entry = m_entries_by_id[entry.get_id()];
    if(previous_entry) {
      auto& previous_account_entries =
        m_entries_by_directory_entry[previous_entry->get_directory_entry()];
      previous_account_entries.erase(std::find_if(
        previous_account_entries.begin(), previous_account_entries.end(),
        [&] (const auto& account_entry) {
          return account_entry->get_id() == entry.get_id();
        }));
    }
    previous_entry = new_entry;
    auto& account_entries =
      m_entries_by_directory_entry[entry.get_directory_entry()];
    account_entries.push_back(new_entry);
  }

  inline void LocalComplianceRuleDataStore::remove(ComplianceRuleEntry::Id id) {
    auto lock = boost::lock_guard(m_mutex);
    auto entry_by_id_iterator = m_entries_by_id.find(id);
    if(entry_by_id_iterator == m_entries_by_id.end()) {
      return;
    }
    auto& entry = entry_by_id_iterator->second;
    auto& account_entries =
      m_entries_by_directory_entry[entry->get_directory_entry()];
    account_entries.erase(std::find_if(account_entries.begin(),
      account_entries.end(), [&] (const auto& account_entry) {
        return account_entry->get_id() == id;
      }));
    m_entries_by_id.erase(entry_by_id_iterator);
  }

  inline void LocalComplianceRuleDataStore::store(
    const ComplianceRuleViolationRecord& record) {}

  inline void LocalComplianceRuleDataStore::close() {
    m_open_state.close();
  }
}

#endif
