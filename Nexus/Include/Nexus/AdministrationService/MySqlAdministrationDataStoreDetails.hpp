#ifndef NEXUS_MYSQLADMINISTRATIONDATASTOREDETAILS_HPP
#define NEXUS_MYSQLADMINISTRATIONDATASTOREDETAILS_HPP
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include "Nexus/AdministrationService/AdministrationService.hpp"

namespace Nexus {
namespace AdministrationService {
namespace Details {
namespace SqlInsert {
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
    mysqlpp::sql_double, buying_power,
    mysqlpp::sql_double, net_loss,
    mysqlpp::sql_int, allowed_state,
    mysqlpp::sql_int, loss_from_top,
    mysqlpp::sql_int, transition_time);

  sql_create_3(risk_states, 3, 0,
    mysqlpp::sql_int_unsigned, account,
    mysqlpp::sql_int, state,
    mysqlpp::sql_datetime, expiry);
}

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
    mysqlpp::sql_double, buying_power,
    mysqlpp::sql_double, net_loss,
    mysqlpp::sql_int, allowed_state,
    mysqlpp::sql_int, loss_from_top,
    mysqlpp::sql_int, transition_time);

  sql_create_3(risk_states, 3, 0,
    mysqlpp::sql_int_unsigned, account,
    mysqlpp::sql_int, state,
    mysqlpp::sql_datetime, expiry);

  inline bool TableExists(mysqlpp::Connection& databaseConnection,
      const std::string& schema, const char* table) {
    mysqlpp::Query query = databaseConnection.query();
    query << "SHOW TABLES IN " << schema << " LIKE " << mysqlpp::quote << table;
    mysqlpp::StoreQueryResult result = query.store();
    return !result.empty();
  }

  inline bool LoadAccountIdentitiesTable(
      mysqlpp::Connection& databaseConnection, const std::string& schema) {
    if(TableExists(databaseConnection, schema, "account_identities")) {
      return true;
    }
    mysqlpp::Query query = databaseConnection.query();
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

  inline bool LoadRiskParametersTable(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(TableExists(databaseConnection, schema, "risk_parameters")) {
      return true;
    }
    mysqlpp::Query query = databaseConnection.query();
    query << "CREATE TABLE risk_parameters ("
      "account INTEGER UNSIGNED PRIMARY KEY NOT NULL,"
      "currency INTEGER NOT NULL,"
      "buying_power DOUBLE NOT NULL,"
      "net_loss DOUBLE NOT NULL,"
      "allowed_state INTEGER NOT NULL,"
      "loss_from_top INTEGER NOT NULL,"
      "transition_time INTEGER NOT NULL)";
    return query.execute();
  }

  inline bool LoadRiskStatesTable(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(TableExists(databaseConnection, schema, "risk_states")) {
      return true;
    }
    mysqlpp::Query query = databaseConnection.query();
    query << "CREATE TABLE risk_states ("
      "account INTEGER UNSIGNED PRIMARY KEY NOT NULL,"
      "state INTEGER NOT NULL,"
      "expiry DATETIME NOT NULL)";
    return query.execute();
  }

  inline bool LoadTables(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(!LoadAccountIdentitiesTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadRiskParametersTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadRiskStatesTable(databaseConnection, schema)) {
      return false;
    }
    return true;
  }
}
}
}

#endif
