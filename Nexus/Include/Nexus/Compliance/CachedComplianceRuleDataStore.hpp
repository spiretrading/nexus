#ifndef NEXUS_CACHED_COMPLIANCE_RULE_DATA_STORE_HPP
#define NEXUS_CACHED_COMPLIANCE_RULE_DATA_STORE_HPP
#include <type_traits>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Mutex.hpp>
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"

namespace Nexus::Compliance {

  /**
   * Caches a ComplianceRuleDataStore.
   * @param <D> The type of ComplianceRuleDataStore to cache.
   */
  template<typename D>
  class CachedComplianceRuleDataStore {
    public:

      /** The type of ComplianceRuleDataStore to cache. */
      using DataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a CachedComplianceRuleDataStore.
       * @param data_store The ComplianceRuleDataStore to cache.
       */
      template<typename DF>
      explicit CachedComplianceRuleDataStore(DF&& data_store);

      ~CachedComplianceRuleDataStore();
      std::vector<ComplianceRuleEntry> load_all_compliance_rule_entries();
      ComplianceRuleEntry::Id load_next_compliance_rule_entry_id();
      boost::optional<ComplianceRuleEntry>
        load_compliance_rule_entry(ComplianceRuleEntry::Id id);
      std::vector<ComplianceRuleEntry> load_compliance_rule_entries(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry);
      void store(const ComplianceRuleEntry& entry);
      void remove(ComplianceRuleEntry::Id id);
      void store(const ComplianceRuleViolationRecord& record);
      void close();

    private:
      mutable Beam::Threading::Mutex m_mutex;
      Beam::GetOptionalLocalPtr<D> m_data_store;
      LocalComplianceRuleDataStore m_cache;
      Beam::IO::OpenState m_open_state;

      CachedComplianceRuleDataStore(
        const CachedComplianceRuleDataStore&) = delete;
      CachedComplianceRuleDataStore& operator =(
        const CachedComplianceRuleDataStore&) = delete;
  };

  template<typename D>
  CachedComplianceRuleDataStore(D&&) ->
    CachedComplianceRuleDataStore<std::decay_t<D>>;

  template<typename D>
  template<typename DF>
  CachedComplianceRuleDataStore<D>::CachedComplianceRuleDataStore(
      DF&& data_store)
      : m_data_store(std::forward<DF>(data_store)) {
    try {
      auto entries = m_data_store->load_all_compliance_rule_entries();
      for(auto& entry : entries) {
        m_cache.store(entry);
      }
    } catch(const std::exception&) {
      close();
      BOOST_RETHROW;
    }
  }

  template<typename D>
  CachedComplianceRuleDataStore<D>::~CachedComplianceRuleDataStore() {
    close();
  }

  template<typename D>
  std::vector<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<D>::load_all_compliance_rule_entries() {
    auto lock = boost::lock_guard(m_mutex);
    return m_cache.load_all_compliance_rule_entries();
  }

  template<typename D>
  ComplianceRuleEntry::Id
      CachedComplianceRuleDataStore<D>::load_next_compliance_rule_entry_id() {
    auto lock = boost::lock_guard(m_mutex);
    return m_data_store->load_next_compliance_rule_entry_id();
  }

  template<typename D>
  boost::optional<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<D>::load_compliance_rule_entry(
        ComplianceRuleEntry::Id id) {
    auto lock = boost::lock_guard(m_mutex);
    return m_cache.load_compliance_rule_entry(id);
  }

  template<typename D>
  std::vector<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<D>::load_compliance_rule_entries(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry) {
    auto lock = boost::lock_guard(m_mutex);
    return m_cache.load_compliance_rule_entries(directory_entry);
  }

  template<typename D>
  void CachedComplianceRuleDataStore<D>::store(
      const ComplianceRuleEntry& entry) {
    auto lock = boost::lock_guard(m_mutex);
    m_data_store->store(entry);
    m_cache.store(entry);
  }

  template<typename D>
  void CachedComplianceRuleDataStore<D>::remove(ComplianceRuleEntry::Id id) {
    auto lock = boost::lock_guard(m_mutex);
    m_data_store->remove(id);
    m_cache.remove(id);
  }

  template<typename D>
  void CachedComplianceRuleDataStore<D>::store(
      const ComplianceRuleViolationRecord& record) {
    auto lock = boost::lock_guard(m_mutex);
    m_data_store->store(record);
    m_cache.store(record);
  }

  template<typename D>
  void CachedComplianceRuleDataStore<D>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_data_store->close();
    m_cache.close();
    m_open_state.Close();
  }
}

#endif
