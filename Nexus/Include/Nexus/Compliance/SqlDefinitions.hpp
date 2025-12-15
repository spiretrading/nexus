#ifndef NEXUS_COMPLIANCE_SQL_DEFINITIONS_HPP
#define NEXUS_COMPLIANCE_SQL_DEFINITIONS_HPP
#include <vector>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Sql/Conversions.hpp>
#include <Beam/Sql/PosixTimeToSqlDateTime.hpp>
#include <boost/throw_exception.hpp>
#include <Viper/Row.hpp>
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus {

  /** Row used to store ComplianceRuleEntries. */
  struct ComplianceRuleEntriesRow {

    /** The entry's id. */
    ComplianceRuleEntry::Id m_entry_id;

    /** The DirectoryEntry the rule applies to. */
    Beam::DirectoryEntry m_directory_entry;

    /** The entry's state. */
    ComplianceRuleEntry::State m_state;

    /** The name of the entry's schema. */
    std::string m_schema_name;

    /** The raw representation of the entry's parameters. */
    Beam::SharedBuffer m_schema_parameters;
  };

  /** Returns a row representing a ComplianceRuleEntry. */
  inline const auto& get_compliance_rule_entries_row() {
    static auto ROW = Viper::Row<ComplianceRuleEntriesRow>().
      add_column("entry_id", &ComplianceRuleEntriesRow::m_entry_id).
      extend(Viper::Row<Beam::DirectoryEntry>().
        add_column("directory_entry", &Beam::DirectoryEntry::m_id).
        add_column("directory_entry_type", &Beam::DirectoryEntry::m_type),
        &ComplianceRuleEntriesRow::m_directory_entry).
      add_column("state", &ComplianceRuleEntriesRow::m_state).
      add_column("schema_name", Viper::varchar(64),
        &ComplianceRuleEntriesRow::m_schema_name).
      add_column("schema_parameters",
        &ComplianceRuleEntriesRow::m_schema_parameters).
      set_primary_key("entry_id");
    return ROW;
  }

  /** Returns a row representing a ComplianceRuleViolationRecord. */
  inline const auto& get_compliance_rule_violation_records_row() {
    static auto ROW = Viper::Row<ComplianceRuleViolationRecord>().
      add_column("account",
        [] (const auto& row) {
          return row.m_account.m_id;
        },
        [] (auto& row, auto column) {
          row.m_account =
            Beam::DirectoryEntry::make_account(column);
        }).
      add_column("order_id", &ComplianceRuleViolationRecord::m_order_id).
      add_column("rule_id", &ComplianceRuleViolationRecord::m_rule_id).
      add_column("schema_name", Viper::varchar(64),
        &ComplianceRuleViolationRecord::m_schema_name).
      add_column("reason", Viper::varchar(256),
        &ComplianceRuleViolationRecord::m_reason).
      add_column("timestamp", &ComplianceRuleViolationRecord::m_timestamp);
    return ROW;
  }

  inline ComplianceRuleEntry convert(const ComplianceRuleEntriesRow& row) {
    auto receiver = Beam::BinaryReceiver<Beam::SharedBuffer>();
    receiver.set(Beam::Ref(row.m_schema_parameters));
    auto schema_parameters = std::vector<ComplianceParameter>();
    try {
      receiver.shuttle(schema_parameters);
    } catch(const Beam::SerializationException&) {
      boost::throw_with_location(
        ComplianceRuleDataStoreException("Unable to load schema parameters."));
    }
    auto schema =
      ComplianceRuleSchema(row.m_schema_name, std::move(schema_parameters));
    auto result = ComplianceRuleEntry(row.m_entry_id,
      std::move(row.m_directory_entry), row.m_state, std::move(schema));
    return result;
  }
}

#endif
