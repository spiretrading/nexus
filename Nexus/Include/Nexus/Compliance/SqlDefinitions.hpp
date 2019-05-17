#ifndef NEXUS_COMPLIANCE_SQL_DEFINITIONS_HPP
#define NEXUS_COMPLIANCE_SQL_DEFINITIONS_HPP
#include <Beam/Sql/Conversions.hpp>
#include <Beam/Sql/PosixTimeToSqlDateTime.hpp>
#include <Viper/Row.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus::Compliance {

  //! Row used to store ComplianceRuleEntries.
  struct ComplianceRuleEntriesRow {

    //! The entry's id.
    ComplianceRuleId m_entryId;

    //! The DirectoryEntry the rule applies to.
    Beam::ServiceLocator::DirectoryEntry m_directoryEntry;

    //! The entry's state.
    ComplianceRuleEntry::State m_state;

    //! The name of the entry's schema.
    std::string m_schemaName;

    //! The raw representation of the entry's parameters.
    Beam::IO::SharedBuffer m_schemaParameters;
  };

  //! Returns a row representing a ComplianceRuleEntry.
  inline const auto& GetComplianceRuleEntriesRow() {
    static auto ROW = Viper::Row<ComplianceRuleEntriesRow>().
      add_column("entry_id", &ComplianceRuleEntriesRow::m_entryId).
      extend(Viper::Row<Beam::ServiceLocator::DirectoryEntry>().
        add_column("directory_entry",
          &Beam::ServiceLocator::DirectoryEntry::m_id).
        add_column("directory_entry_type",
          &Beam::ServiceLocator::DirectoryEntry::m_type),
        &ComplianceRuleEntriesRow::m_directoryEntry).
      add_column("state", &ComplianceRuleEntriesRow::m_state).
      add_column("schema_name", &ComplianceRuleEntriesRow::m_schemaName).
      add_column("schema_parameters",
        &ComplianceRuleEntriesRow::m_schemaParameters).
      set_primary_key("entry_id");
    return ROW;
  }

  //! Returns a row representing a ComplianceRuleViolationRecord.
  inline const auto& GetComplianceRuleViolationRecordsRow() {
    static auto ROW = Viper::Row<ComplianceRuleViolationRecord>().
      add_column("account",
        [] (const auto& row) {
          return row.m_account.m_id;
        },
        [] (auto& row, unsigned int id) {
          row.m_account.m_id = id;
          row.m_account.m_type =
            Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT;
        }).
      add_column("order_id", &ComplianceRuleViolationRecord::m_orderId).
      add_column("rule_id", &ComplianceRuleViolationRecord::m_ruleId).
      add_column("schema_name", Viper::varchar(64),
        &ComplianceRuleViolationRecord::m_schemaName).
      add_column("reason", Viper::varchar(256),
        &ComplianceRuleViolationRecord::m_reason).
      add_column("timestamp",
        [] (const auto& row) {
          return Beam::ToSqlTimestamp(row.m_timestamp);
        },
        [] (auto& row, auto column) {
          row.m_timestamp = Beam::FromSqlTimestamp(column);
        });
    return ROW;
  }
}

#endif
