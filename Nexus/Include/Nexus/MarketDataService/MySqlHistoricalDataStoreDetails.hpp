#ifndef NEXUS_MARKETDATAMYSQLHISTORICALDATASTOREDETAILS_HPP
#define NEXUS_MARKETDATAMYSQLHISTORICALDATASTOREDETAILS_HPP
#include <string>
#include <Beam/MySql/PosixTimeToMySqlDateTime.hpp>
#include <Beam/MySql/Utilities.hpp>
#include <Beam/Queries/SqlUtilities.hpp>
#include <boost/lexical_cast.hpp>
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Details {
  sql_create_9(order_imbalances, 9, 0,
    mysqlpp::sql_varchar, market,
    mysqlpp::sql_varchar, symbol,
    mysqlpp::sql_varchar, symbol_market,
    mysqlpp::sql_int, country,
    mysqlpp::sql_int, side,
    mysqlpp::sql_bigint, size,
    mysqlpp::sql_bigint, price,
    mysqlpp::sql_bigint, timestamp,
    mysqlpp::sql_bigint_unsigned, query_sequence);

  sql_create_8(bbo_quotes, 8, 0,
    mysqlpp::sql_varchar, symbol,
    mysqlpp::sql_int, country,
    mysqlpp::sql_bigint, bid_price,
    mysqlpp::sql_bigint, bid_size,
    mysqlpp::sql_bigint, ask_price,
    mysqlpp::sql_bigint, ask_size,
    mysqlpp::sql_bigint, timestamp,
    mysqlpp::sql_bigint_unsigned, query_sequence);

  sql_create_9(market_quotes, 9, 0,
    mysqlpp::sql_varchar, symbol,
    mysqlpp::sql_int, country,
    mysqlpp::sql_varchar, market,
    mysqlpp::sql_bigint, bid_price,
    mysqlpp::sql_bigint, bid_size,
    mysqlpp::sql_bigint, ask_price,
    mysqlpp::sql_bigint, ask_size,
    mysqlpp::sql_bigint, timestamp,
    mysqlpp::sql_bigint_unsigned, query_sequence);

  sql_create_10(book_quotes, 10, 0,
    mysqlpp::sql_varchar, symbol,
    mysqlpp::sql_int, country,
    mysqlpp::sql_varchar, mpid,
    mysqlpp::sql_bool, is_primary,
    mysqlpp::sql_varchar, market,
    mysqlpp::sql_bigint, price,
    mysqlpp::sql_bigint, size,
    mysqlpp::sql_int, side,
    mysqlpp::sql_bigint, timestamp,
    mysqlpp::sql_bigint_unsigned, query_sequence);

  sql_create_9(time_and_sales, 9, 0,
    mysqlpp::sql_varchar, symbol,
    mysqlpp::sql_int, country,
    mysqlpp::sql_bigint, price,
    mysqlpp::sql_bigint, size,
    mysqlpp::sql_varchar, condition_code,
    mysqlpp::sql_int, condition_type,
    mysqlpp::sql_varchar, market,
    mysqlpp::sql_bigint, timestamp,
    mysqlpp::sql_bigint_unsigned, query_sequence);

  inline bool LoadOrderImbalancesTable(mysqlpp::Connection& connection,
      const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "order_imbalances", connection)) {
      return true;
    }
    auto query = connection.query();
    query << "CREATE TABLE order_imbalances ("
      "market VARCHAR(16) BINARY NOT NULL,"
      "symbol VARCHAR(16) BINARY NOT NULL,"
      "symbol_market VARCHAR(16) BINARY NOT NULL,"
      "country INTEGER UNSIGNED NOT NULL,"
      "side INTEGER UNSIGNED NOT NULL,"
      "size BIGINT NOT NULL,"
      "price BIGINT NOT NULL,"
      "timestamp BIGINT NOT NULL,"
      "query_sequence BIGINT UNSIGNED NOT NULL,"
      "INDEX sequence_index(market, query_sequence),"
      "INDEX timestamp_index(market, timestamp, query_sequence))";
    return query.execute();
  }

  inline bool LoadBboQuotesTable(mysqlpp::Connection& connection,
      const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "bbo_quotes", connection)) {
      return true;
    }
    auto query = connection.query();
    query << "CREATE TABLE bbo_quotes ("
      "symbol VARCHAR(16) BINARY NOT NULL,"
      "country INTEGER UNSIGNED NOT NULL,"
      "bid_price BIGINT NOT NULL,"
      "bid_size BIGINT NOT NULL,"
      "ask_price BIGINT NOT NULL,"
      "ask_size BIGINT NOT NULL,"
      "timestamp BIGINT NOT NULL,"
      "query_sequence BIGINT UNSIGNED NOT NULL,"
      "INDEX sequence_index(symbol, country, query_sequence),"
      "INDEX timestamp_index(symbol, country, timestamp, query_sequence))";
    return query.execute();
  }

  inline bool LoadMarketQuotesTable(mysqlpp::Connection& connection,
      const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "market_quotes", connection)) {
      return true;
    }
    auto query = connection.query();
    query << "CREATE TABLE market_quotes ("
      "symbol VARCHAR(16) BINARY NOT NULL,"
      "country INTEGER UNSIGNED NOT NULL,"
      "market VARCHAR(16) BINARY NOT NULL,"
      "bid_price BIGINT NOT NULL,"
      "bid_size BIGINT NOT NULL,"
      "ask_price BIGINT NOT NULL,"
      "ask_size BIGINT NOT NULL,"
      "timestamp BIGINT NOT NULL,"
      "query_sequence BIGINT UNSIGNED NOT NULL,"
      "INDEX sequence_index(symbol, country, query_sequence),"
      "INDEX timestamp_index(symbol, country, timestamp, query_sequence))";
    return query.execute();
  }

  inline bool LoadBookQuotesTable(mysqlpp::Connection& connection,
      const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "book_quotes", connection)) {
      return true;
    }
    auto query = connection.query();
    query << "CREATE TABLE book_quotes ("
      "symbol VARCHAR(16) BINARY NOT NULL,"
      "country INTEGER UNSIGNED NOT NULL,"
      "mpid VARCHAR(16) BINARY NOT NULL,"
      "is_primary BOOL NOT NULL,"
      "market VARCHAR(16) BINARY NOT NULL,"
      "price BIGINT NOT NULL,"
      "size BIGINT NOT NULL,"
      "side INTEGER UNSIGNED NOT NULL,"
      "timestamp BIGINT NOT NULL,"
      "query_sequence BIGINT UNSIGNED NOT NULL,"
      "INDEX sequence_index(symbol, country, query_sequence),"
      "INDEX timestamp_index(symbol, country, timestamp, query_sequence))";
    return query.execute();
  }

  inline bool LoadTimeAndSalesTable(mysqlpp::Connection& connection,
      const std::string& schema) {
    if(Beam::MySql::TestTable(schema, "time_and_sales", connection)) {
      return true;
    }
    auto query = connection.query();
    query << "CREATE TABLE time_and_sales ("
      "symbol VARCHAR(16) BINARY NOT NULL,"
      "country INTEGER UNSIGNED NOT NULL,"
      "price BIGINT NOT NULL,"
      "size BIGINT NOT NULL,"
      "condition_code VARCHAR(4) BINARY NOT NULL,"
      "condition_type INTEGER NOT NULL,"
      "market VARCHAR(16) BINARY NOT NULL,"
      "timestamp BIGINT NOT NULL,"
      "query_sequence BIGINT UNSIGNED NOT NULL,"
      "INDEX sequence_index(symbol, country, query_sequence),"
      "INDEX timestamp_index(symbol, country, timestamp, query_sequence))";
    return query.execute();
  }

  inline bool LoadTables(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(!LoadOrderImbalancesTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadBboQuotesTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadMarketQuotesTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadBookQuotesTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadTimeAndSalesTable(databaseConnection, schema)) {
      return false;
    }
    return true;
  }

  struct SqlFunctor {
    std::string operator ()(MarketCode market) const {
      std::string index = "market = \"";
      index += Beam::Queries::EscapeSql(market.GetData());
      index += "\"";
      return index;
    };

    std::string InsertKey(MarketCode market) const {
      return "\"" + Beam::Queries::EscapeSql(market.GetData()) + "\"";
    }

    std::string operator ()(const Security& security) const {
      std::string index = "symbol = \"" +
        Beam::Queries::EscapeSql(security.GetSymbol()) +
        "\" AND country = " + boost::lexical_cast<std::string>(
        static_cast<int>(security.GetCountry()));
      return index;
    };

    std::string InsertKey(const Security& security) const {
      return "\"" + Beam::Queries::EscapeSql(security.GetSymbol()) + "\", " +
        boost::lexical_cast<std::string>(
        static_cast<int>(security.GetCountry()));
    }

    SequencedOrderImbalance operator ()(const order_imbalances& row) const {
      auto orderImbalance = Beam::Queries::MakeSequencedValue(OrderImbalance(
        Security(row.symbol, std::string(row.symbol_market),
        static_cast<CountryCode>(row.country)),
        static_cast<Side>(row.side), row.size,
        Money::FromRepresentation(row.price),
        Beam::MySql::FromMySqlTimestamp(row.timestamp)),
        Beam::Queries::Sequence(row.query_sequence));
      return orderImbalance;
    }

    SequencedBboQuote operator ()(const bbo_quotes& row) const {
      auto bboQuote = Beam::Queries::MakeSequencedValue(
        BboQuote(Quote(Money::FromRepresentation(row.bid_price), row.bid_size,
        Side::BID), Quote(Money::FromRepresentation(row.ask_price),
        row.ask_size, Side::ASK), Beam::MySql::FromMySqlTimestamp(
        row.timestamp)), Beam::Queries::Sequence(row.query_sequence));
      return bboQuote;
    }

    SequencedBookQuote operator ()(const book_quotes& row) const {
      auto bookQuote = Beam::Queries::MakeSequencedValue(BookQuote(
        std::string(row.mpid), row.is_primary, std::string(row.market),
        Quote(Money::FromRepresentation(row.price), row.size,
        static_cast<Side>(row.side)),
        Beam::MySql::FromMySqlTimestamp(row.timestamp)),
        Beam::Queries::Sequence(row.query_sequence));
      return bookQuote;
    }

    SequencedMarketQuote operator ()(const market_quotes& row) const {
      auto marketQuote = Beam::Queries::MakeSequencedValue(MarketQuote(
        std::string(row.market),
        Quote(Money::FromRepresentation(row.bid_price), row.bid_size,
        Side::BID), Quote(Money::FromRepresentation(row.ask_price),
        row.ask_size, Side::ASK),
        Beam::MySql::FromMySqlTimestamp(row.timestamp)),
        Beam::Queries::Sequence(row.query_sequence));
      return marketQuote;
    }

    SequencedTimeAndSale operator ()(const time_and_sales& row) const {
      TimeAndSale::Condition condition(
        static_cast<TimeAndSale::Condition::Type>(row.condition_type),
        std::string(row.condition_code));
      auto timeAndSale = Beam::Queries::MakeSequencedValue(TimeAndSale(
        Beam::MySql::FromMySqlTimestamp(row.timestamp),
        Money::FromRepresentation(row.price), row.size, condition,
        std::string(row.market)), Beam::Queries::Sequence(row.query_sequence));
      return timeAndSale;
    }

    order_imbalances operator ()(
        const SequencedMarketOrderImbalance& orderImbalance) const {
      order_imbalances row(orderImbalance->GetIndex().GetData(),
        (*orderImbalance)->m_security.GetSymbol(),
        (*orderImbalance)->m_security.GetMarket().GetData(),
        (*orderImbalance)->m_security.GetCountry(),
        static_cast<int>((*orderImbalance)->m_side), (*orderImbalance)->m_size,
        (*orderImbalance)->m_referencePrice.GetRepresentation(),
        Beam::MySql::ToMySqlTimestamp((*orderImbalance)->m_timestamp),
        orderImbalance.GetSequence().GetOrdinal());
      return row;
    }

    bbo_quotes operator ()(const SequencedSecurityBboQuote& bboQuote) const {
      bbo_quotes row(bboQuote->GetIndex().GetSymbol(),
        bboQuote->GetIndex().GetCountry(),
        (*bboQuote)->m_bid.m_price.GetRepresentation(),
        (*bboQuote)->m_bid.m_size,
        (*bboQuote)->m_ask.m_price.GetRepresentation(),
        (*bboQuote)->m_ask.m_size,
        Beam::MySql::ToMySqlTimestamp((*bboQuote)->m_timestamp),
        bboQuote.GetSequence().GetOrdinal());
      return row;
    }

    market_quotes operator ()(
        const SequencedSecurityMarketQuote& marketQuote) const {
      market_quotes row(marketQuote->GetIndex().GetSymbol(),
        marketQuote->GetIndex().GetCountry(),
        (*marketQuote)->m_market.GetData(),
        (*marketQuote)->m_bid.m_price.GetRepresentation(),
        (*marketQuote)->m_bid.m_size,
        (*marketQuote)->m_ask.m_price.GetRepresentation(),
        (*marketQuote)->m_ask.m_size,
        Beam::MySql::ToMySqlTimestamp((*marketQuote)->m_timestamp),
        marketQuote.GetSequence().GetOrdinal());
      return row;
    }

    book_quotes operator ()(const SequencedSecurityBookQuote& bookQuote) const {
      book_quotes row(bookQuote->GetIndex().GetSymbol(),
        bookQuote->GetIndex().GetCountry(), (*bookQuote)->m_mpid,
        (*bookQuote)->m_isPrimaryMpid, (*bookQuote)->m_market.GetData(),
        (*bookQuote)->m_quote.m_price.GetRepresentation(),
        (*bookQuote)->m_quote.m_size,
        static_cast<int>((*bookQuote)->m_quote.m_side),
        Beam::MySql::ToMySqlTimestamp((*bookQuote)->m_timestamp),
        bookQuote.GetSequence().GetOrdinal());
      return row;
    }

    time_and_sales operator ()(
        const SequencedSecurityTimeAndSale& timeAndSale) const {
      time_and_sales row(timeAndSale->GetIndex().GetSymbol(),
        timeAndSale->GetIndex().GetCountry(),
        (*timeAndSale)->m_price.GetRepresentation(),
        (*timeAndSale)->m_size, (*timeAndSale)->m_condition.m_code,
        static_cast<int>((*timeAndSale)->m_condition.m_type),
        (*timeAndSale)->m_marketCenter, Beam::MySql::ToMySqlTimestamp(
        (*timeAndSale)->m_timestamp), timeAndSale.GetSequence().GetOrdinal());
      return row;
    }
  };
}
}
}

#endif
