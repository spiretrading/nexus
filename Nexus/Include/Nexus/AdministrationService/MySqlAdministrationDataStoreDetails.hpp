#ifndef NEXUS_MYSQL_ADMINISTRATION_DATA_STORE_DETAILS_HPP
#define NEXUS_MYSQL_ADMINISTRATION_DATA_STORE_DETAILS_HPP
#include <Beam/MySql/Utilities.hpp>
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include "Nexus/AdministrationService/AdministrationService.hpp"

namespace Nexus {
namespace AdministrationService {
namespace Details {
  sql_create_12(account_identities, 12, 0,
    mysqlpp::sql_int_unsigned, account,
    mysqlpp::sql_varchar, first_name,
    mysqlpp::sql_varchar, last_name,
    mysqlpp::sql_varchar, e_mail,
    mysqlpp::sql_varchar, address_line_one,
    mysqlpp::sql_varchar, address_line_two,
    mysqlpp::sql_varchar, address_line_three,
    mysqlpp::sql_varchar, city,
    mysqlpp::sql_varchar, province,
    mysqlpp::sql_int, country,
    mysqlpp::sql_text, user_notes,
    mysqlpp::sql_varchar, photo_id);

  sql_create_7(risk_parameters, 7, 0,
    mysqlpp::sql_int_unsigned, account,
    mysqlpp::sql_int, currency,
    mysqlpp::sql_bigint, buying_power,
    mysqlpp::sql_bigint, net_loss,
    mysqlpp::sql_int, allowed_state,
    mysqlpp::sql_int, loss_from_top,
    mysqlpp::sql_int, transition_time);

  sql_create_3(risk_states, 3, 0,
    mysqlpp::sql_int_unsigned, account,
    mysqlpp::sql_int, state,
    mysqlpp::sql_datetime, expiry);

  sql_create_5(account_modification_requests, 5, 0,
    mysqlpp::sql_int, id,
    mysqlpp::sql_int, type,
    mysqlpp::sql_int_unsigned, account,
    mysqlpp::sql_int_unsigned, submission_account,
    mysqlpp::sql_bigint_unsigned, timestamp);

  sql_create_2(entitlement_modifications, 2, 0,
    mysqlpp::sql_int, id,
    mysqlpp::sql_int_unsigned, entitlement);

  sql_create_3(administration_messages, 3, 0,
    mysqlpp::sql_int, id,
    mysqlpp::sql_int_unsigned, account,
    mysqlpp::sql_bigint_unsigned, timestamp);

  sql_create_3(administration_message_bodies, 3, 0,
    mysqlpp::sql_int, id,
    mysqlpp::sql_varchar, content_type,
    mysqlpp::sql_varchar, message);

  sql_create_2(account_modification_request_messages, 2, 0,
    mysqlpp::sql_int, request_id,
    mysqlpp::sql_int, message_id);

  inline bool LoadAccountIdentitiesTable(
      mysqlpp::Connection& databaseConnection, const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "account_identities",
        databaseConnection)) {
      return true;
    }
    auto query = databaseConnection.query();
    query << "CREATE TABLE account_identities ("
      "account INTEGER UNSIGNED PRIMARY KEY NOT NULL,"
      "first_name VARCHAR(64) BINARY NOT NULL,"
      "last_name VARCHAR(64) BINARY NOT NULL,"
      "e_mail VARCHAR(64) BINARY NOT NULL,"
      "address_line_one VARCHAR(256) BINARY NOT NULL,"
      "address_line_two VARCHAR(256) BINARY NOT NULL,"
      "address_line_three VARCHAR(256) BINARY NOT NULL,"
      "city VARCHAR(64) BINARY NOT NULL,"
      "province VARCHAR(64) BINARY NOT NULL,"
      "country INTEGER NOT NULL,"
      "user_notes TEXT NOT NULL,"
      "photo_id VARCHAR(256) BINARY NOT NULL)";
    return query.execute();
  }

  inline bool LoadAccountModificationRequestsTable(
      mysqlpp::Connection& databaseConnection, const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "account_modification_requests",
        databaseConnection)) {
      return true;
    }
    auto query = databaseConnection.query();
    query << "CREATE TABLE account_modification_requests ("
      "id INTEGER PRIMARY KEY NOT NULL,"
      "type INTEGER NOT NULL,"
      "account INTEGER UNSIGNED NOT NULL,"
      "submission_account INTEGER UNSIGNED NOT NULL,"
      "timestamp BIGINT UNSIGNED NOT NULL,"
      "INDEX(id, account))";
    return query.execute();
  }

  inline bool LoadAccountModificationRequestMessagesTable(
      mysqlpp::Connection& databaseConnection, const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "account_modification_request_messages",
        databaseConnection)) {
      return true;
    }
    auto query = databaseConnection.query();
    query << "CREATE TABLE account_modification_request_messages ("
      "request_id INTEGER PRIMARY KEY NOT NULL,"
      "message_id INTEGER NOT NULL)";
    return query.execute();
  }

  inline bool LoadAdministrationMessagesTable(
      mysqlpp::Connection& databaseConnection, const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "administration_messages",
        databaseConnection)) {
      return true;
    }
    auto query = databaseConnection.query();
    query << "CREATE TABLE administration_messages ("
      "id INTEGER PRIMARY KEY NOT NULL,"
      "account INTEGER UNSIGNED NOT NULL,"
      "timestamp BIGINT UNSIGNED NOT NULL)";
    return query.execute();
  }

  inline bool LoadAdministrationMessageBodiesTable(
      mysqlpp::Connection& databaseConnection, const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "administration_message_bodies",
        databaseConnection)) {
      return true;
    }
    auto query = databaseConnection.query();
    query << "CREATE TABLE administration_message_bodies ("
      "id INTEGER PRIMARY KEY NOT NULL,"
      "content_type VARCHAR(100),"
      "message VARCHAR(40000))";
    return query.execute();
  }

  inline bool LoadEntitlementModificationsTable(
      mysqlpp::Connection& databaseConnection, const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "entitlement_modifications",
        databaseConnection)) {
      return true;
    }
    auto query = databaseConnection.query();
    query << "CREATE TABLE entitlement_modifications ("
      "id INTEGER PRIMARY KEY NOT NULL,"
      "entitlement INTEGER UNSIGNED NOT NULL)";
    return query.execute();
  }

  inline bool LoadRiskParametersTable(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "risk_parameters", databaseConnection)) {
      return true;
    }
    auto query = databaseConnection.query();
    query << "CREATE TABLE risk_parameters ("
      "account INTEGER UNSIGNED PRIMARY KEY NOT NULL,"
      "currency INTEGER NOT NULL,"
      "buying_power BIGINT NOT NULL,"
      "net_loss BIGINT NOT NULL,"
      "allowed_state INTEGER NOT NULL,"
      "loss_from_top INTEGER NOT NULL,"
      "transition_time INTEGER NOT NULL)";
    return query.execute();
  }

  inline bool LoadRiskStatesTable(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "risk_states", databaseConnection)) {
      return true;
    }
    auto query = databaseConnection.query();
    query << "CREATE TABLE risk_states ("
      "account INTEGER UNSIGNED PRIMARY KEY NOT NULL,"
      "state INTEGER NOT NULL,"
      "expiry DATETIME NOT NULL)";
    return query.execute();
  }

  inline bool LoadTables(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    return LoadAccountIdentitiesTable(databaseConnection, schema) &&
      LoadAccountModificationRequestsTable(databaseConnection, schema) &&
      LoadAccountModificationRequestMessagesTable(databaseConnection, schema) &&
      LoadAdministrationMessagesTable(databaseConnection, schema) &&
      LoadAdministrationMessageBodiesTable(databaseConnection, schema) &&
      LoadEntitlementModificationsTable(databaseConnection, schema) &&
      LoadRiskParametersTable(databaseConnection, schema) &&
      LoadRiskStatesTable(databaseConnection, schema);
  }
}
}
}

#endif
