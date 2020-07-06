#ifndef NEXUS_SQL_COMPLIANCE_RULE_DATA_STORE_HPP
#define NEXUS_SQL_COMPLIANCE_RULE_DATA_STORE_HPP
#include <memory>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStoreException.hpp"
#include "Nexus/Compliance/SqlDefinitions.hpp"

namespace Nexus::Compliance {

  /**
   * Implements a ComplianceRuleDataStore backed by an SQL database.
   * @param <C> The SQL connection to use.
   */
  template<typename C>
  class SqlComplianceRuleDataStore : private boost::noncopyable {
    public:

      /** The SQL connection to use. */
      using Connection = C;

      /**
       * Constructs a MySqlComplianceRuleDataStore.
       * @param connection The SQL connection to use.
       */
      explicit SqlComplianceRuleDataStore(
        std::unique_ptr<Connection> connection);

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
    auto rows = std::vector<ComplianceRuleEntriesRow>();
    {
      auto lock = std::lock_guard(m_mutex);
      try {
        m_connection->execute(Viper::select(GetComplianceRuleEntriesRow(),
          "compliance_rule_entries", std::back_inserter(rows)));
      } catch(const Viper::ExecuteException& e) {
        BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException(e.what()));
      }
    }
    auto entries = std::vector<ComplianceRuleEntry>();
    std::transform(rows.begin(), rows.end(), std::back_inserter(entries),
      &ConvertRow);
    return entries;
  }

  template<typename C>
  ComplianceRuleId SqlComplianceRuleDataStore<C>::
      LoadNextComplianceRuleEntryId() {
    auto entryId = std::optional<ComplianceRuleId>();
    auto lock = std::lock_guard(m_mutex);
    try {
      m_connection->execute(Viper::select(
        Viper::max<ComplianceRuleId>("entry_id"), "compliance_rule_entries",
        &entryId));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException(e.what()));
    }
    if(!entryId) {
      return 1;
    }
    return *entryId + 1;
  }

  template<typename C>
  boost::optional<ComplianceRuleEntry> SqlComplianceRuleDataStore<C>::
      LoadComplianceRuleEntry(ComplianceRuleId id) {
    auto row = std::optional<ComplianceRuleEntriesRow>();
    {
      auto lock = std::lock_guard(m_mutex);
      m_connection->execute(Viper::select(GetComplianceRuleEntriesRow(),
        "compliance_rule_entries", Viper::sym("entry_id") == id, &row));
    }
    if(!row) {
      return boost::none;
    }
    return ConvertRow(*row);
  }

  template<typename C>
  std::vector<ComplianceRuleEntry> SqlComplianceRuleDataStore<C>::
      LoadComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    auto rows = std::vector<ComplianceRuleEntriesRow>();
    {
      auto lock = std::lock_guard(m_mutex);
      try {
        m_connection->execute(Viper::select(GetComplianceRuleEntriesRow(),
          "compliance_rule_entries",
          Viper::sym("directory_entry") == directoryEntry.m_id,
          std::back_inserter(rows)));
      } catch(const Viper::ExecuteException& e) {
        BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException(e.what()));
      }
    }
    auto entries = std::vector<ComplianceRuleEntry>();
    std::transform(rows.begin(), rows.end(), std::back_inserter(entries),
      &ConvertRow);
    return entries;
  }

  template<typename C>
  void SqlComplianceRuleDataStore<C>::Store(const ComplianceRuleEntry& entry) {
    auto parameterBuffer = Beam::IO::SharedBuffer();
    auto sender = Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>();
    sender.SetSink(Beam::Ref(parameterBuffer));
    try {
      sender.Shuttle(entry.GetSchema().GetParameters());
    } catch(const Beam::Serialization::SerializationException&) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException(
        "Unable to store schema parameters."));
    }
    auto row = ComplianceRuleEntriesRow{entry.GetId(),
      entry.GetDirectoryEntry(), entry.GetState(), entry.GetSchema().GetName(),
      std::move(parameterBuffer)};
    try {
      auto lock = std::lock_guard(m_mutex);
      m_connection->execute(Viper::upsert(GetComplianceRuleEntriesRow(),
        "compliance_rule_entries", &row));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlComplianceRuleDataStore<C>::Delete(ComplianceRuleId id) {
    auto lock = std::lock_guard(m_mutex);
    try {
      m_connection->execute(Viper::erase("compliance_rule_entries",
        Viper::sym("entry_id") == id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlComplianceRuleDataStore<C>::Store(
      const ComplianceRuleViolationRecord& violationRecord) {
    auto lock = std::lock_guard(m_mutex);
    try {
      m_connection->execute(Viper::insert(
        GetComplianceRuleViolationRecordsRow(),
        "compliance_rule_violation_records", &violationRecord));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlComplianceRuleDataStore<C>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_connection->open();
      m_connection->execute(Viper::create_if_not_exists(
        GetComplianceRuleEntriesRow(), "compliance_rule_entries"));
      m_connection->execute(Viper::create_if_not_exists(
        GetComplianceRuleViolationRecordsRow(),
        "compliance_rule_violation_records"));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C>
  void SqlComplianceRuleDataStore<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C>
  void SqlComplianceRuleDataStore<C>::Shutdown() {
    m_connection->close();
    m_openState.SetClosed();
  }
}

#endif
