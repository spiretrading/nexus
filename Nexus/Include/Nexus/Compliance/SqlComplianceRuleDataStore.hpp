#ifndef NEXUS_SQL_COMPLIANCE_RULE_DATA_STORE_HPP
#define NEXUS_SQL_COMPLIANCE_RULE_DATA_STORE_HPP
#include <memory>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStoreException.hpp"
#include "Nexus/Compliance/SqlDefinitions.hpp"

namespace Nexus {

  /**
   * Implements a ComplianceRuleDataStore backed by an SQL database.
   * @param <C> The SQL connection to use.
   */
  template<typename C>
  class SqlComplianceRuleDataStore {
    public:

      /** The SQL connection to use. */
      using Connection = C;

      /**
       * Constructs an SqlComplianceRuleDataStore.
       * @param connection The SQL connection to use.
       */
      explicit SqlComplianceRuleDataStore(
        std::unique_ptr<Connection> connection);

      ~SqlComplianceRuleDataStore();

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
      mutable Beam::Mutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      Beam::OpenState m_open_state;
  };

  template<typename C>
  SqlComplianceRuleDataStore<C>::SqlComplianceRuleDataStore(
      std::unique_ptr<Connection> connection)
      : m_connection(std::move(connection)) {
    try {
      m_connection->open();
      m_connection->execute(Viper::create_if_not_exists(
        get_compliance_rule_entries_row(), "compliance_rule_entries"));
      m_connection->execute(
        Viper::create_if_not_exists(get_compliance_rule_violation_records_row(),
        "compliance_rule_violation_records"));
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C>
  SqlComplianceRuleDataStore<C>::~SqlComplianceRuleDataStore() {
    close();
  }

  template<typename C>
  std::vector<ComplianceRuleEntry>
      SqlComplianceRuleDataStore<C>::load_all_compliance_rule_entries() {
    auto rows = std::vector<ComplianceRuleEntriesRow>();
    {
      auto lock = std::lock_guard(m_mutex);
      try {
        m_connection->execute(Viper::select(get_compliance_rule_entries_row(),
          "compliance_rule_entries", std::back_inserter(rows)));
      } catch(const Viper::ExecuteException& e) {
        boost::throw_with_location(ComplianceRuleDataStoreException(e.what()));
      }
    }
    auto entries = std::vector<ComplianceRuleEntry>();
    std::transform(
      rows.begin(), rows.end(), std::back_inserter(entries), &convert);
    return entries;
  }

  template<typename C>
  ComplianceRuleEntry::Id
      SqlComplianceRuleDataStore<C>::load_next_compliance_rule_entry_id() {
    auto entry_id = std::optional<ComplianceRuleEntry::Id>();
    auto lock = std::lock_guard(m_mutex);
    try {
      m_connection->execute(
        Viper::select(Viper::max<ComplianceRuleEntry::Id>("entry_id"),
          "compliance_rule_entries", &entry_id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ComplianceRuleDataStoreException(e.what()));
    }
    if(!entry_id) {
      return 1;
    }
    return *entry_id + 1;
  }

  template<typename C>
  boost::optional<ComplianceRuleEntry>
      SqlComplianceRuleDataStore<C>::load_compliance_rule_entry(
        ComplianceRuleEntry::Id id) {
    auto row = std::optional<ComplianceRuleEntriesRow>();
    {
      auto lock = std::lock_guard(m_mutex);
      try {
        m_connection->execute(Viper::select(get_compliance_rule_entries_row(),
          "compliance_rule_entries", Viper::sym("entry_id") == id, &row));
      } catch(const Viper::ExecuteException& e) {
        boost::throw_with_location(ComplianceRuleDataStoreException(e.what()));
      }
    }
    if(!row) {
      return boost::none;
    }
    return convert(*row);
  }

  template<typename C>
  std::vector<ComplianceRuleEntry>
      SqlComplianceRuleDataStore<C>::load_compliance_rule_entries(
        const Beam::DirectoryEntry& directory_entry) {
    auto rows = std::vector<ComplianceRuleEntriesRow>();
    {
      auto lock = std::lock_guard(m_mutex);
      try {
        m_connection->execute(Viper::select(
          get_compliance_rule_entries_row(), "compliance_rule_entries",
          Viper::sym("directory_entry") == directory_entry.m_id,
          std::back_inserter(rows)));
      } catch(const Viper::ExecuteException& e) {
        boost::throw_with_location(ComplianceRuleDataStoreException(e.what()));
      }
    }
    auto entries = std::vector<ComplianceRuleEntry>();
    std::transform(
      rows.begin(), rows.end(), std::back_inserter(entries), &convert);
    return entries;
  }

  template<typename C>
  void SqlComplianceRuleDataStore<C>::store(const ComplianceRuleEntry& entry) {
    auto parameter_buffer = Beam::SharedBuffer();
    auto sender = Beam::BinarySender<Beam::SharedBuffer>();
    sender.set(Beam::Ref(parameter_buffer));
    try {
      sender.shuttle(entry.get_schema().get_parameters());
    } catch(const Beam::SerializationException&) {
      boost::throw_with_location(
        ComplianceRuleDataStoreException("Unable to store schema parameters."));
    }
    auto row = ComplianceRuleEntriesRow(entry.get_id(),
      entry.get_directory_entry(), entry.get_state(),
      entry.get_schema().get_name(), std::move(parameter_buffer));
    try {
      auto lock = std::lock_guard(m_mutex);
      m_connection->execute(Viper::upsert(
        get_compliance_rule_entries_row(), "compliance_rule_entries", &row));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ComplianceRuleDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlComplianceRuleDataStore<C>::remove(ComplianceRuleEntry::Id id) {
    auto lock = std::lock_guard(m_mutex);
    try {
      m_connection->execute(
        Viper::erase("compliance_rule_entries", Viper::sym("entry_id") == id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ComplianceRuleDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlComplianceRuleDataStore<C>::store(
      const ComplianceRuleViolationRecord& record) {
    auto lock = std::lock_guard(m_mutex);
    try {
      m_connection->execute(
        Viper::insert(get_compliance_rule_violation_records_row(),
          "compliance_rule_violation_records", &record));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(ComplianceRuleDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlComplianceRuleDataStore<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_connection->close();
    m_open_state.close();
  }
}

#endif
