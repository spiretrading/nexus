#ifndef NEXUS_CACHEDCOMPLIANCERULEDATASTORE_HPP
#define NEXUS_CACHEDCOMPLIANCERULEDATASTORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class CachedComplianceRuleDataStore
      \brief Caches a ComplianceRuleDataStore.
      \tparam DataStoreType The type of ComplianceRuleDataStore to cache.
   */
  template<typename DataStoreType>
  class CachedComplianceRuleDataStore : private boost::noncopyable {
    public:

      //! The type of ComplianceRuleDataStore to cache.
      using DataStore = Beam::GetTryDereferenceType<DataStoreType>;

      //! Constructs a CachedComplianceRuleDataStore.
      /*!
        \param dataStore The ComplianceRuleDataStore to cache.
      */
      template<typename DataStoreForward>
      CachedComplianceRuleDataStore(DataStoreForward&& dataStore);

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

      void Open();

      void Close();

    private:
      mutable Beam::Threading::Mutex m_mutex;
      Beam::GetOptionalLocalPtr<DataStoreType> m_dataStore;
      LocalComplianceRuleDataStore m_cache;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename DataStoreType>
  template<typename DataStoreForward>
  CachedComplianceRuleDataStore<DataStoreType>::CachedComplianceRuleDataStore(
      DataStoreForward&& dataStore)
      : m_dataStore{std::forward<DataStoreForward>(dataStore)} {}

  template<typename DataStoreType>
  CachedComplianceRuleDataStore<DataStoreType>::
      ~CachedComplianceRuleDataStore() {
    Close();
  }

  template<typename DataStoreType>
  std::vector<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<DataStoreType>::
      LoadAllComplianceRuleEntries() {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    return m_cache.LoadAllComplianceRuleEntries();
  }

  template<typename DataStoreType>
  ComplianceRuleId CachedComplianceRuleDataStore<DataStoreType>::
      LoadNextComplianceRuleEntryId() {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    return m_dataStore->LoadNextComplianceRuleEntryId();
  }

  template<typename DataStoreType>
  boost::optional<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<DataStoreType>::LoadComplianceRuleEntry(
      ComplianceRuleId id) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    return m_cache.LoadComplianceRuleEntry(id);
  }

  template<typename DataStoreType>
  std::vector<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<DataStoreType>::LoadComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    return m_cache.LoadComplianceRuleEntries(directoryEntry);
  }

  template<typename DataStoreType>
  void CachedComplianceRuleDataStore<DataStoreType>::Store(
      const ComplianceRuleEntry& entry) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    m_dataStore->Store(entry);
    m_cache.Store(entry);
  }

  template<typename DataStoreType>
  void CachedComplianceRuleDataStore<DataStoreType>::Delete(
      ComplianceRuleId id) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    m_dataStore->Delete(id);
    m_cache.Delete(id);
  }

  template<typename DataStoreType>
  void CachedComplianceRuleDataStore<DataStoreType>::Store(
      const ComplianceRuleViolationRecord& violationRecord) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    m_dataStore->Store(violationRecord);
    m_cache.Store(violationRecord);
  }

  template<typename DataStoreType>
  void CachedComplianceRuleDataStore<DataStoreType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStore->Open();
      m_cache.Open();
      auto entries = m_dataStore->LoadAllComplianceRuleEntries();
      for(auto& entry : entries) {
        m_cache.Store(entry);
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename DataStoreType>
  void CachedComplianceRuleDataStore<DataStoreType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename DataStoreType>
  void CachedComplianceRuleDataStore<DataStoreType>::Shutdown() {
    m_dataStore->Close();
    m_cache.Close();
    m_openState.SetClosed();
  }
}
}

#endif
