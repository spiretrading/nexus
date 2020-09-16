#ifndef NEXUS_CACHED_COMPLIANCE_RULE_DATA_STORE_HPP
#define NEXUS_CACHED_COMPLIANCE_RULE_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"

namespace Nexus::Compliance {

  /**
   * Caches a ComplianceRuleDataStore.
   * @param <D> The type of ComplianceRuleDataStore to cache.
   */
  template<typename D>
  class CachedComplianceRuleDataStore : private boost::noncopyable {
    public:

      /** The type of ComplianceRuleDataStore to cache. */
      using DataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a CachedComplianceRuleDataStore.
       * @param dataStore The ComplianceRuleDataStore to cache.
       */
      template<typename DF>
      explicit CachedComplianceRuleDataStore(DF&& dataStore);

      ~CachedComplianceRuleDataStore();

      std::vector<ComplianceRuleEntry> LoadAllComplianceRuleEntries();

      ComplianceRuleId LoadNextComplianceRuleEntryId();

      boost::optional<ComplianceRuleEntry> LoadComplianceRuleEntry(
        ComplianceRuleId id);

      std::vector<ComplianceRuleEntry> LoadComplianceRuleEntries(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry);

      void Store(const ComplianceRuleEntry& entry);

      void Delete(ComplianceRuleId id);

      void Store(const ComplianceRuleViolationRecord& violationRecord);

      void Close();

    private:
      mutable Beam::Threading::Mutex m_mutex;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      LocalComplianceRuleDataStore m_cache;
      Beam::IO::OpenState m_openState;
  };

  template<typename D>
  template<typename DF>
  CachedComplianceRuleDataStore<D>::CachedComplianceRuleDataStore(
      DF&& dataStore)
      : m_dataStore(std::forward<DF>(dataStore)) {
    try {
      auto entries = m_dataStore->LoadAllComplianceRuleEntries();
      for(auto& entry : entries) {
        m_cache.Store(entry);
      }
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename D>
  CachedComplianceRuleDataStore<D>::~CachedComplianceRuleDataStore() {
    Close();
  }

  template<typename D>
  std::vector<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<D>::LoadAllComplianceRuleEntries() {
    auto lock = boost::lock_guard(m_mutex);
    return m_cache.LoadAllComplianceRuleEntries();
  }

  template<typename D>
  ComplianceRuleId CachedComplianceRuleDataStore<D>::
      LoadNextComplianceRuleEntryId() {
    auto lock = boost::lock_guard(m_mutex);
    return m_dataStore->LoadNextComplianceRuleEntryId();
  }

  template<typename D>
  boost::optional<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<D>::LoadComplianceRuleEntry(
      ComplianceRuleId id) {
    auto lock = boost::lock_guard(m_mutex);
    return m_cache.LoadComplianceRuleEntry(id);
  }

  template<typename D>
  std::vector<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<D>::LoadComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    auto lock = boost::lock_guard(m_mutex);
    return m_cache.LoadComplianceRuleEntries(directoryEntry);
  }

  template<typename D>
  void CachedComplianceRuleDataStore<D>::Store(
      const ComplianceRuleEntry& entry) {
    auto lock = boost::lock_guard(m_mutex);
    m_dataStore->Store(entry);
    m_cache.Store(entry);
  }

  template<typename D>
  void CachedComplianceRuleDataStore<D>::Delete(ComplianceRuleId id) {
    auto lock = boost::lock_guard(m_mutex);
    m_dataStore->Delete(id);
    m_cache.Delete(id);
  }

  template<typename D>
  void CachedComplianceRuleDataStore<D>::Store(
      const ComplianceRuleViolationRecord& violationRecord) {
    auto lock = boost::lock_guard(m_mutex);
    m_dataStore->Store(violationRecord);
    m_cache.Store(violationRecord);
  }

  template<typename D>
  void CachedComplianceRuleDataStore<D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_dataStore->Close();
    m_cache.Close();
    m_openState.Close();
  }
}

#endif
