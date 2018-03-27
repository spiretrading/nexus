#ifndef NEXUS_MYSQLORDEREXECUTIONDATASTOREDETAILS_HPP
#define NEXUS_MYSQLORDEREXECUTIONDATASTOREDETAILS_HPP
#include <string>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/MySql/PosixTimeToMySqlDateTime.hpp>
#include <Beam/MySql/Utilities.hpp>
#include <Beam/Queries/SqlUtilities.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <boost/lexical_cast.hpp>
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDataStoreException.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"
#include "Nexus/Queries/TraversalExpressionVisitor.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Details {
  sql_create_18(submissions, 18, 0,
    mysqlpp::sql_bigint_unsigned, order_id,
    mysqlpp::sql_int_unsigned, submission_account,
    mysqlpp::sql_int_unsigned, account,
    mysqlpp::sql_bigint_unsigned, timestamp,
    mysqlpp::sql_varchar, symbol,
    mysqlpp::sql_varchar, market,
    mysqlpp::sql_int, country,
    mysqlpp::sql_int, currency,
    mysqlpp::sql_int, type,
    mysqlpp::sql_int, side,
    mysqlpp::sql_varchar, destination,
    mysqlpp::sql_double, quantity,
    mysqlpp::sql_double, price,
    mysqlpp::sql_int, time_in_force,
    mysqlpp::sql_bigint_unsigned, time_in_force_expiry,
    mysqlpp::sql_blob, additional_fields,
    mysqlpp::sql_bool, shorting_flag,
    mysqlpp::sql_bigint_unsigned, query_sequence);

  sql_create_1(live_orders, 1, 0,
    mysqlpp::sql_bigint_unsigned, order_id);

  sql_create_18(status_submissions, 18, 0,
    mysqlpp::sql_bigint_unsigned, order_id,
    mysqlpp::sql_int_unsigned, submission_account,
    mysqlpp::sql_int_unsigned, account,
    mysqlpp::sql_bigint_unsigned, timestamp,
    mysqlpp::sql_varchar, symbol,
    mysqlpp::sql_varchar, market,
    mysqlpp::sql_int, country,
    mysqlpp::sql_int, currency,
    mysqlpp::sql_int, type,
    mysqlpp::sql_int, side,
    mysqlpp::sql_varchar, destination,
    mysqlpp::sql_double, quantity,
    mysqlpp::sql_double, price,
    mysqlpp::sql_int, time_in_force,
    mysqlpp::sql_bigint_unsigned, time_in_force_expiry,
    mysqlpp::sql_blob, additional_fields,
    mysqlpp::sql_bool, shorting_flag,
    mysqlpp::sql_bigint_unsigned, query_sequence);

  sql_create_14(execution_reports, 14, 0,
    mysqlpp::sql_int_unsigned, account,
    mysqlpp::sql_bigint_unsigned, order_id,
    mysqlpp::sql_int, sequence,
    mysqlpp::sql_bigint_unsigned, timestamp,
    mysqlpp::sql_int, status,
    mysqlpp::sql_double, last_quantity,
    mysqlpp::sql_double, last_price,
    mysqlpp::sql_varchar, liquidity_flag,
    mysqlpp::sql_varchar, last_market,
    mysqlpp::sql_double, execution_fee,
    mysqlpp::sql_double, processing_fee,
    mysqlpp::sql_double, commission,
    mysqlpp::sql_varchar, text,
    mysqlpp::sql_bigint_unsigned, query_sequence);

  inline bool LoadSubmissionsTable(mysqlpp::Connection& connection,
      const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "submissions", connection)) {
      return true;
    }
    auto query = connection.query();
    query << "CREATE TABLE submissions ("
      "order_id BIGINT UNSIGNED PRIMARY KEY NOT NULL,"
      "submission_account INTEGER UNSIGNED NOT NULL,"
      "account INTEGER UNSIGNED NOT NULL,"
      "timestamp BIGINT UNSIGNED NOT NULL,"
      "symbol VARCHAR(16) BINARY NOT NULL,"
      "market VARCHAR(16) BINARY NOT NULL,"
      "country INTEGER UNSIGNED NOT NULL,"
      "currency INTEGER UNSIGNED NOT NULL,"
      "type INTEGER UNSIGNED NOT NULL,"
      "side INTEGER UNSIGNED NOT NULL,"
      "destination VARCHAR(16) BINARY NOT NULL,"
      "quantity DOUBLE NOT NULL,"
      "price DOUBLE NOT NULL,"
      "time_in_force INTEGER UNSIGNED NOT NULL,"
      "time_in_force_expiry BIGINT UNSIGNED NOT NULL,"
      "additional_fields BLOB NOT NULL,"
      "shorting_flag BOOLEAN NOT NULL,"
      "query_sequence BIGINT UNSIGNED NOT NULL,"
      "INDEX(order_id),"
      "INDEX sequence_index(account, query_sequence),"
      "INDEX timestamp_index(account, timestamp, query_sequence))";
    return query.execute();
  }

  inline bool LoadLiveOrdersTable(mysqlpp::Connection& connection,
      const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "live_orders", connection)) {
      return true;
    }
    auto query = connection.query();
    query << "CREATE TABLE live_orders ("
      "order_id BIGINT UNSIGNED PRIMARY KEY NOT NULL)";
    return query.execute();
  }

  inline bool LoadStatusSubmissionsView(mysqlpp::Connection& connection,
      const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "status_submissions", connection)) {
      return true;
    }
    auto query = connection.query();
    query << "CREATE VIEW status_submissions AS "
      "SELECT submissions.*, IFNULL(live_orders.order_id, 0) != 0 AS "
      "is_live FROM submissions LEFT JOIN live_orders ON "
      "submissions.order_id = live_orders.order_id";
    return query.execute();
  }

  inline bool LoadExecutionReportsTable(mysqlpp::Connection& connection,
      const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "execution_reports", connection)) {
      return true;
    }
    auto query = connection.query();
    query << "CREATE TABLE execution_reports ("
      "account INTEGER UNSIGNED NOT NULL,"
      "order_id BIGINT UNSIGNED NOT NULL,"
      "sequence INTEGER NOT NULL,"
      "timestamp BIGINT UNSIGNED NOT NULL,"
      "status INTEGER UNSIGNED NOT NULL,"
      "last_quantity DOUBLE NOT NULL,"
      "last_price DOUBLE NOT NULL,"
      "liquidity_flag VARCHAR(8) BINARY NOT NULL,"
      "last_market VARCHAR(16) BINARY NOT NULL,"
      "execution_fee DOUBLE NOT NULL,"
      "processing_fee DOUBLE NOT NULL,"
      "commission DOUBLE NOT NULL,"
      "text VARCHAR(256) BINARY NOT NULL,"
      "query_sequence BIGINT UNSIGNED NOT NULL,"
      "INDEX(order_id),"
      "INDEX sequence_index(account, query_sequence),"
      "INDEX timestamp_index(account, timestamp, query_sequence))";
    return query.execute();
  }

  inline bool LoadTables(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(!LoadSubmissionsTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadLiveOrdersTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadStatusSubmissionsView(databaseConnection, schema)) {
      return false;
    }
    if(!LoadExecutionReportsTable(databaseConnection, schema)) {
      return false;
    }
    return true;
  }

  struct SqlFunctor {
    std::string operator ()(
        const Beam::ServiceLocator::DirectoryEntry& account) const {
      std::string index = "account = " + boost::lexical_cast<std::string>(
        account.m_id);
      return index;
    };

    std::string InsertKey(
        const Beam::ServiceLocator::DirectoryEntry& account) const {
      return boost::lexical_cast<std::string>(account.m_id);
    }

    SequencedOrderInfo operator ()(const submissions& row) const {
      SequencedOrderInfo orderInfo;
      orderInfo->m_fields.m_account.m_type =
        Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT;
      orderInfo->m_fields.m_account.m_id = row.account;
      orderInfo->m_fields.m_security = Security{row.symbol, row.market,
        static_cast<CountryCode>(row.country)};
      orderInfo->m_fields.m_currency = CurrencyId{row.currency};
      orderInfo->m_fields.m_type = static_cast<OrderType>(row.type);
      orderInfo->m_fields.m_side = static_cast<Side>(row.side);
      orderInfo->m_fields.m_destination = row.destination;
      orderInfo->m_fields.m_quantity =
        Quantity::FromRepresentation(row.quantity);
      orderInfo->m_fields.m_price =
        Money{Quantity::FromRepresentation(row.price)};
      orderInfo->m_fields.m_timeInForce = TimeInForce{
        static_cast<TimeInForce::Type>(row.time_in_force),
        Beam::MySql::FromMySqlTimestamp(row.time_in_force_expiry)};
      if(!row.additional_fields.empty()) {
        Beam::Serialization::BinaryReceiver<Beam::IO::SharedBuffer> receiver;
        Beam::IO::SharedBuffer additionalFieldsBuffer{
          row.additional_fields.data(), row.additional_fields.size()};
        receiver.SetSource(Beam::Ref(additionalFieldsBuffer));
        try {
          receiver.Shuttle(orderInfo->m_fields.m_additionalFields);
        } catch(const Beam::Serialization::SerializationException&) {
          BOOST_THROW_EXCEPTION(OrderExecutionDataStoreException{
            "Unable to load additional fields."});
        }
      }
      orderInfo->m_submissionAccount.m_type =
        Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT;
      orderInfo->m_submissionAccount.m_id = row.submission_account;
      orderInfo->m_orderId = row.order_id;
      orderInfo->m_shortingFlag = row.shorting_flag;
      orderInfo->m_timestamp = Beam::MySql::FromMySqlTimestamp(row.timestamp);
      orderInfo.GetSequence() = Beam::Queries::Sequence{row.query_sequence};
      return orderInfo;
    }

    SequencedOrderInfo operator ()(const status_submissions& row) const {
      SequencedOrderInfo orderInfo;
      orderInfo->m_fields.m_account.m_type =
        Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT;
      orderInfo->m_fields.m_account.m_id = row.account;
      orderInfo->m_fields.m_security = Security{row.symbol, row.market,
        static_cast<CountryCode>(row.country)};
      orderInfo->m_fields.m_currency = CurrencyId{row.currency};
      orderInfo->m_fields.m_type = static_cast<OrderType>(row.type);
      orderInfo->m_fields.m_side = static_cast<Side>(row.side);
      orderInfo->m_fields.m_destination = row.destination;
      orderInfo->m_fields.m_quantity =
        Quantity::FromRepresentation(row.quantity);
      orderInfo->m_fields.m_price =
        Money{Quantity::FromRepresentation(row.price)};
      orderInfo->m_fields.m_timeInForce = TimeInForce{
        static_cast<TimeInForce::Type>(row.time_in_force),
        Beam::MySql::FromMySqlTimestamp(row.time_in_force_expiry)};
      if(!row.additional_fields.empty()) {
        Beam::Serialization::BinaryReceiver<Beam::IO::SharedBuffer> receiver;
        Beam::IO::SharedBuffer additionalFieldsBuffer{
          row.additional_fields.data(), row.additional_fields.size()};
        receiver.SetSource(Beam::Ref(additionalFieldsBuffer));
        try {
          receiver.Shuttle(orderInfo->m_fields.m_additionalFields);
        } catch(const Beam::Serialization::SerializationException&) {
          BOOST_THROW_EXCEPTION(OrderExecutionDataStoreException(
            "Unable to load additional fields."));
        }
      }
      orderInfo->m_submissionAccount.m_type =
        Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT;
      orderInfo->m_submissionAccount.m_id = row.submission_account;
      orderInfo->m_orderId = row.order_id;
      orderInfo->m_shortingFlag = row.shorting_flag;
      orderInfo->m_timestamp = Beam::MySql::FromMySqlTimestamp(row.timestamp);
      orderInfo.GetSequence() = Beam::Queries::Sequence{row.query_sequence};
      return orderInfo;
    }

    SequencedExecutionReport operator ()(const execution_reports& row) const {
      SequencedExecutionReport executionReport;
      executionReport->m_id = row.order_id;
      executionReport->m_timestamp = Beam::MySql::FromMySqlTimestamp(
        row.timestamp);
      executionReport->m_sequence = row.sequence;
      executionReport->m_status = static_cast<OrderStatus>(row.status);
      executionReport->m_lastQuantity =
        Quantity::FromRepresentation(row.last_quantity);
      executionReport->m_lastPrice =
        Money{Quantity::FromRepresentation(row.last_price)};
      executionReport->m_liquidityFlag = row.liquidity_flag;
      executionReport->m_lastMarket = row.last_market;
      executionReport->m_executionFee = Money{Quantity::FromRepresentation(
        row.execution_fee)};
      executionReport->m_processingFee = Money{Quantity::FromRepresentation(
        row.processing_fee)};
      executionReport->m_commission = Money{Quantity::FromRepresentation(
        row.commission)};
      executionReport->m_text = row.text;
      executionReport.GetSequence() = Beam::Queries::Sequence{
        row.query_sequence};
      return executionReport;
    }

    submissions operator ()(
        const SequencedAccountOrderInfo& orderInfo) const {
      Beam::IO::SharedBuffer additionalFieldsBuffer;
      if(!(*orderInfo)->m_fields.m_additionalFields.empty()) {
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer> sender;
        sender.SetSink(Beam::Ref(additionalFieldsBuffer));
        try {
          sender.Shuttle((*orderInfo)->m_fields.m_additionalFields);
        } catch(const Beam::Serialization::SerializationException&) {
          BOOST_THROW_EXCEPTION(OrderExecutionDataStoreException{
            "Unable to store additional fields."});
        }
      }
      submissions row{(*orderInfo)->m_orderId,
        (*orderInfo)->m_submissionAccount.m_id,
        (*orderInfo)->m_fields.m_account.m_id,
        Beam::MySql::ToMySqlTimestamp((*orderInfo)->m_timestamp),
        (*orderInfo)->m_fields.m_security.GetSymbol(),
        (*orderInfo)->m_fields.m_security.GetMarket().GetData(),
        (*orderInfo)->m_fields.m_security.GetCountry(),
        (*orderInfo)->m_fields.m_currency.m_value,
        static_cast<int>((*orderInfo)->m_fields.m_type),
        static_cast<int>((*orderInfo)->m_fields.m_side),
        (*orderInfo)->m_fields.m_destination,
        (*orderInfo)->m_fields.m_quantity.GetRepresentation(),
        static_cast<Quantity>(
          (*orderInfo)->m_fields.m_price).GetRepresentation(),
        static_cast<int>((*orderInfo)->m_fields.m_timeInForce.GetType()),
        Beam::MySql::ToMySqlTimestamp(
          (*orderInfo)->m_fields.m_timeInForce.GetExpiry()),
        mysqlpp::sql_blob(additionalFieldsBuffer.GetData(),
          additionalFieldsBuffer.GetSize()),
        (*orderInfo)->m_shortingFlag, orderInfo.GetSequence().GetOrdinal()};
      return row;
    }

    execution_reports operator ()(
        const SequencedAccountExecutionReport& executionReport) const {
      execution_reports row{executionReport->GetIndex().m_id,
        (*executionReport)->m_id, (*executionReport)->m_sequence,
        Beam::MySql::ToMySqlTimestamp((*executionReport)->m_timestamp),
        static_cast<int>((*executionReport)->m_status),
        (*executionReport)->m_lastQuantity.GetRepresentation(),
        static_cast<Quantity>(
          (*executionReport)->m_lastPrice).GetRepresentation(),
        (*executionReport)->m_liquidityFlag, (*executionReport)->m_lastMarket,
        static_cast<Quantity>(
          (*executionReport)->m_executionFee).GetRepresentation(),
        static_cast<Quantity>(
          (*executionReport)->m_processingFee).GetRepresentation(),
        static_cast<Quantity>(
          (*executionReport)->m_commission).GetRepresentation(),
        (*executionReport)->m_text, executionReport.GetSequence().GetOrdinal()};
      return row;
    }
  };

  inline bool HasLiveCheck(const Beam::Queries::Expression& expression) {
    struct IsLiveVisitor : Queries::TraversalExpressionVisitor {
      bool m_hasCheck = false;

      virtual void Visit(
          const Beam::Queries::MemberAccessExpression& expression)
          override final {
        if(expression.GetName() == "is_live" &&
            expression.GetExpression()->GetType() == Queries::OrderInfoType()) {
          m_hasCheck = true;
        } else {
          Queries::TraversalExpressionVisitor::Visit(expression);
        }
      }
    };
    IsLiveVisitor visitor;
    expression->Apply(visitor);
    return visitor.m_hasCheck;
  }
}
}
}

#endif
