#ifndef NEXUS_MYSQLCOMPLIANCERULEDATASTOREDETAILS_HPP
#define NEXUS_MYSQLCOMPLIANCERULEDATASTOREDETAILS_HPP
#include <Beam/Sql/Utilities.hpp>
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include "Nexus/Compliance/Compliance.hpp"

namespace Nexus {
namespace Compliance {
namespace Details {
  sql_create_6(compliance_rule_entries, 6, 0,
    mysqlpp::sql_bigint_unsigned, entry_id,
    mysqlpp::sql_int_unsigned, directory_entry,
    mysqlpp::sql_int_unsigned, directory_entry_type,
    mysqlpp::sql_int, state,
    mysqlpp::sql_varchar, schema_name,
    mysqlpp::sql_blob, schema_parameters);

  sql_create_6(compliance_rule_violation_records, 6, 0,
    mysqlpp::sql_int_unsigned, account,
    mysqlpp::sql_bigint_unsigned, order_id,
    mysqlpp::sql_bigint_unsigned, rule_id,
    mysqlpp::sql_varchar, schema_name,
    mysqlpp::sql_varchar, reason,
    mysqlpp::sql_bigint_unsigned, timestamp);

  inline bool LoadComplianceRuleEntriesTable(
      mysqlpp::Connection& databaseConnection, const std::string& schema) {
    if(Beam::TestTable(schema, "compliance_rule_entries", databaseConnection)) {
      return true;
    }
    auto query = databaseConnection.query();
    query << "CREATE TABLE compliance_rule_entries ("
      "entry_id BIGINT UNSIGNED PRIMARY KEY NOT NULL,"
      "directory_entry INTEGER UNSIGNED NOT NULL,"
      "directory_entry_type INTEGER UNSIGNED NOT NULL,"
      "state INTEGER NOT NULL,"
      "schema_name VARCHAR(64) BINARY NOT NULL,"
      "schema_parameters BLOB NOT NULL)";
    return query.execute();
  }

  inline bool LoadComplianceRuleViolationsTable(
      mysqlpp::Connection& databaseConnection, const std::string& schema) {
    if(Beam::TestTable(schema, "compliance_rule_violation_records",
        databaseConnection)) {
      return true;
    }
    auto query = databaseConnection.query();
    query << "CREATE TABLE compliance_rule_violation_records ("
      "account INTEGER UNSIGNED NOT NULL,"
      "order_id BIGINT UNSIGNED NOT NULL,"
      "rule_id BIGINT UNSIGNED NOT NULL,"
      "schema_name VARCHAR(64) BINARY NOT NULL,"
      "reason VARCHAR(256) BINARY NOT NULL,"
      "timestamp BIGINT UNSIGNED NOT NULL)";
    return query.execute();
  }

  inline bool LoadTables(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(!LoadComplianceRuleEntriesTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadComplianceRuleViolationsTable(databaseConnection, schema)) {
      return false;
    }
    return true;
  }
}
}
}

#endif
