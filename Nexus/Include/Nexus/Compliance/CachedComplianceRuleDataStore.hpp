#ifndef NEXUS_CACHED_COMPLIANCE_RULE_DATA_STORE_HPP
#define NEXUS_CACHED_COMPLIANCE_RULE_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"

namespace Nexus {

  /**
   * Caches a ComplianceRuleDataStore.
   * @param <D> The type of ComplianceRuleDataStore to cache.
   */
  template<IsComplianceRuleDataStore D>
  class CachedComplianceRuleDataStore {
    public:

      /** The type of ComplianceRuleDataStore to cache. */
      using DataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a CachedComplianceRuleDataStore.
       * @param data_store The ComplianceRuleDataStore to cache.
       */
      template<Beam::Initializes<D> DF>
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
    CachedComplianceRuleDataStore<std::remove_reference_t<D>>;

  template<IsComplianceRuleDataStore D>
  template<Beam::Initializes<D> DF>
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

  template<IsComplianceRuleDataStore D>
  CachedComplianceRuleDataStore<D>::~CachedComplianceRuleDataStore() {
    close();
  }

  template<IsComplianceRuleDataStore D>
  std::vector<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<D>::load_all_compliance_rule_entries() {
    auto lock = boost::lock_guard(m_mutex);
    return m_cache.load_all_compliance_rule_entries();
  }

  template<IsComplianceRuleDataStore D>
  ComplianceRuleEntry::Id
      CachedComplianceRuleDataStore<D>::load_next_compliance_rule_entry_id() {
    auto lock = boost::lock_guard(m_mutex);
    return m_data_store->load_next_compliance_rule_entry_id();
  }

  template<IsComplianceRuleDataStore D>
  boost::optional<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<D>::load_compliance_rule_entry(
        ComplianceRuleEntry::Id id) {
    auto lock = boost::lock_guard(m_mutex);
    return m_cache.load_compliance_rule_entry(id);
  }

  template<IsComplianceRuleDataStore D>
  std::vector<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<D>::load_compliance_rule_entries(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry) {
    auto lock = boost::lock_guard(m_mutex);
    return m_cache.load_compliance_rule_entries(directory_entry);
  }

  template<IsComplianceRuleDataStore D>
  void CachedComplianceRuleDataStore<D>::store(
      const ComplianceRuleEntry& entry) {
    auto lock = boost::lock_guard(m_mutex);
    m_data_store->store(entry);
    m_cache.store(entry);
  }

  template<IsComplianceRuleDataStore D>
  void CachedComplianceRuleDataStore<D>::remove(ComplianceRuleEntry::Id id) {
    auto lock = boost::lock_guard(m_mutex);
    m_data_store->remove(id);
    m_cache.remove(id);
  }

  template<IsComplianceRuleDataStore D>
  void CachedComplianceRuleDataStore<D>::store(
      const ComplianceRuleViolationRecord& record) {
    auto lock = boost::lock_guard(m_mutex);
    m_data_store->store(record);
    m_cache.store(record);
  }

  template<IsComplianceRuleDataStore D>
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
