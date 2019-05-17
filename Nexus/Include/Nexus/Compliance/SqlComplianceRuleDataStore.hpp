#ifndef NEXUS_SQL_COMPLIANCE_RULE_DATA_STORE_HPP
#define NEXUS_SQL_COMPLIANCE_RULE_DATA_STORE_HPP
#include <memory>
#include <string>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStoreException.hpp"
#include "Nexus/Compliance/SqlDefinitions.hpp"

namespace Nexus::Compliance {

  /** Implements a ComplianceRuleDataStore backed using MySQL.
    \tparam C The SQL connection to use.
   */
  template<typename C>
  class SqlComplianceRuleDataStore : private boost::noncopyable {
    public:

      //! The SQL connection to use.
      using Connection = C;

      //! Constructs a MySqlComplianceRuleDataStore.
      /*!
        \param connection The SQL connection to use.
      */
      SqlComplianceRuleDataStore(std::unique_ptr<Connection> connection);

      ~SqlComplianceRuleDataStore();

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
      std::unique_ptr<Connection> m_connection;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename C>
  SqlComplianceRuleDataStore<C>::SqlComplianceRuleDataStore(
      std::unique_ptr<Connection> connection)
      : m_connection(std::move(connection)) {}

  template<typename C>
  SqlComplianceRuleDataStore<C>::~SqlComplianceRuleDataStore() {
    Close();
  }

  template<typename C>
  std::vector<ComplianceRuleEntry> SqlComplianceRuleDataStore<C>::
      LoadAllComplianceRuleEntries() {
    return {};
  }

  template<typename C>
  ComplianceRuleId SqlComplianceRuleDataStore<C>::
      LoadNextComplianceRuleEntryId() {
    return {};
  }

  template<typename C>
  boost::optional<ComplianceRuleEntry> SqlComplianceRuleDataStore<C>::
      LoadComplianceRuleEntry(ComplianceRuleId id) {
    return {};
  }

  template<typename C>
  std::vector<ComplianceRuleEntry> SqlComplianceRuleDataStore<C>::
      LoadComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    return {};
  }

  template<typename C>
  void SqlComplianceRuleDataStore<C>::Store(
      const ComplianceRuleEntry& entry) {}

  template<typename C>
  void SqlComplianceRuleDataStore<C>::Delete(ComplianceRuleId id) {}

  template<typename C>
  void SqlComplianceRuleDataStore<C>::Store(
      const ComplianceRuleViolationRecord& violationRecord) {}

  template<typename C>
  void SqlComplianceRuleDataStore<C>::Open() {}

  template<typename C>
  void SqlComplianceRuleDataStore<C>::Close() {}

  template<typename C>
  void SqlComplianceRuleDataStore<C>::Shutdown() {}
}

#endif
