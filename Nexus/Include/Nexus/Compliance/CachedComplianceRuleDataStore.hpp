#ifndef NEXUS_CACHEDCOMPLIANCERULEDATASTORE_HPP
#define NEXUS_CACHEDCOMPLIANCERULEDATASTORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
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
  ComplianceRuleId CachedComplianceRuleDataStore<DataStoreType>::
      LoadNextComplianceRuleEntryId() {
    return {};
  }

  template<typename DataStoreType>
  boost::optional<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<DataStoreType>::LoadComplianceRuleEntry(
      ComplianceRuleId id) {
    return {};
  }

  template<typename DataStoreType>
  std::vector<ComplianceRuleEntry>
      CachedComplianceRuleDataStore<DataStoreType>::LoadComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    return {};
  }

  template<typename DataStoreType>
  void CachedComplianceRuleDataStore<DataStoreType>::Store(
      const ComplianceRuleEntry& entry) {}

  template<typename DataStoreType>
  void CachedComplianceRuleDataStore<DataStoreType>::Delete(
      ComplianceRuleId id) {
    return {};
  }

  template<typename DataStoreType>
  void CachedComplianceRuleDataStore<DataStoreType>::Store(
      const ComplianceRuleViolationRecord& violationRecord) {}

  template<typename DataStoreType>
  void CachedComplianceRuleDataStore<DataStoreType>::Open() {
    if(m_openState.SetOpening()) {
      return;
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
