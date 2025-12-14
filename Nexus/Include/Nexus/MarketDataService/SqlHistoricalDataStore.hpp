#ifndef NEXUS_SQL_HISTORICAL_DATA_STORE_HPP
#define NEXUS_SQL_HISTORICAL_DATA_STORE_HPP
#include <functional>
#include <thread>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/SqlDataStore.hpp>
#include <Beam/Sql/DatabaseConnectionPool.hpp>
#include <Beam/Threading/Sync.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/HistoricalDataStoreException.hpp"
#include "Nexus/MarketDataService/SqlDefinitions.hpp"
#include "Nexus/Queries/SqlTranslator.hpp"

namespace Nexus {

  /**
   * Stores historical market data in an SQL database.
   * @param <C> The type of SQL connection.
   */
  template<typename C>
  class SqlHistoricalDataStore {
    public:

      /** The type of SQL connection. */
      using Connection = C;

      /** The callable used to build SQL connections. */
      using ConnectionBuilder = std::function<Connection ()>;

      /**
       * Constructs an SqlHistoricalDataStore.
       * @param venues The available venues.
       * @param connection_builder The callable used to build SQL connections.
       */
      explicit SqlHistoricalDataStore(
        VenueDatabase venues, ConnectionBuilder connection_builder);

      ~SqlHistoricalDataStore();

      std::vector<SecurityInfo> load_security_info(
        const SecurityInfoQuery& query);
      void store(const SecurityInfo& info);
      std::vector<SequencedOrderImbalance> load_order_imbalances(
        const VenueMarketDataQuery& query);
      void store(const SequencedVenueOrderImbalance& imbalance);
      void store(const std::vector<SequencedVenueOrderImbalance>& imbalances);
      std::vector<SequencedBboQuote> load_bbo_quotes(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityBboQuote& quote);
      void store(const std::vector<SequencedSecurityBboQuote>& quotes);
      std::vector<SequencedBookQuote> load_book_quotes(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityBookQuote& quote);
      void store(const std::vector<SequencedSecurityBookQuote>& quotes);
      std::vector<SequencedTimeAndSale> load_time_and_sales(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityTimeAndSale& time_and_sale);
      void store(
        const std::vector<SequencedSecurityTimeAndSale>& time_and_sales);
      void close();

    private:
      template<typename V, typename I>
      using DataStore = Beam::SqlDataStore<Connection, V, I, SqlTranslator>;
      VenueDatabase m_venues;
      Beam::DatabaseConnectionPool<Connection> m_reader_pool;
      Beam::DatabaseConnectionPool<Connection> m_writer_pool;
      DataStore<Viper::Row<OrderImbalance>, Viper::Row<Venue>>
        m_order_imbalance_data_store;
      DataStore<Viper::Row<BboQuote>, Viper::Row<Security>>
        m_bbo_quote_data_store;
      DataStore<Viper::Row<BookQuote>, Viper::Row<Security>>
        m_book_quote_data_store;
      DataStore<Viper::Row<TimeAndSale>, Viper::Row<Security>>
        m_time_and_sale_data_store;
      Beam::OpenState m_open_state;

      SqlHistoricalDataStore(const SqlHistoricalDataStore&) = delete;
      SqlHistoricalDataStore& operator =(
        const SqlHistoricalDataStore&) = delete;
  };

  template<typename C>
  SqlHistoricalDataStore<C>::SqlHistoricalDataStore(
      VenueDatabase venues, ConnectionBuilder connection_builder)
      : m_venues(std::move(venues)),
        m_reader_pool(std::thread::hardware_concurrency(), [&] {
          auto connection = std::make_unique<Connection>(connection_builder());
          connection->open();
          return connection;
        }),
        m_writer_pool(1, [&] {
          auto connection = std::make_unique<Connection>(connection_builder());
          connection->open();
          return connection;
        }),
        m_order_imbalance_data_store(
          "order_imbalances", get_order_imbalance_row(), get_venue_row(),
          Beam::Ref(m_reader_pool), Beam::Ref(m_writer_pool)),
        m_bbo_quote_data_store("bbo_quotes", get_bbo_quote_row(),
          get_security_row(), Beam::Ref(m_reader_pool),
          Beam::Ref(m_writer_pool)),
        m_book_quote_data_store("book_quotes", get_book_quote_row(),
          get_security_row(), Beam::Ref(m_reader_pool),
          Beam::Ref(m_writer_pool)),
        m_time_and_sale_data_store("time_and_sales", get_time_and_sale_row(),
          get_security_row(), Beam::Ref(m_reader_pool),
          Beam::Ref(m_writer_pool)) {
    try {
      auto connection = m_writer_pool.load();
      connection->execute(
        Viper::create_if_not_exists(get_security_info_row(), "security_info"));
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C>
  SqlHistoricalDataStore<C>::~SqlHistoricalDataStore() {
    close();
  }

  template<typename C>
  std::vector<SecurityInfo> SqlHistoricalDataStore<C>::load_security_info(
      const SecurityInfoQuery& query) {
    auto matches = std::vector<SecurityInfo>();
    auto filter =
      Beam::make_sql_query<SqlTranslator>("security_info", query.get_filter());
    auto anchor = [&] {
      if(auto anchor = query.get_anchor()) {
        auto left = Viper::Expression(
          std::make_shared<Viper::LiteralExpression>("(symbol,venue)"));
        auto symbol_literal = std::string();
        Viper::literal(anchor->get_symbol()).append_query(symbol_literal);
        auto venue_literal = std::string();
        Viper::literal(anchor->get_venue()).append_query(venue_literal);
        auto right = Viper::Expression(
          std::make_shared<Viper::LiteralExpression>(
            "(" + symbol_literal + "," + venue_literal + ")"));
        if(query.get_snapshot_limit().get_type() ==
            Beam::SnapshotLimit::Type::HEAD) {
          return left > right;
        }
        return left < right;
      }
      return Viper::literal(true);
    }();
    auto order = [&] {
      if(query.get_snapshot_limit().get_type() ==
          Beam::SnapshotLimit::Type::HEAD) {
        return Viper::Order::ASC;
      }
      return Viper::Order::DESC;
    }();
    auto region_filter = Viper::literal(query.get_index().is_global());
    for(auto country : query.get_index().get_countries()) {
      for(auto entry : m_venues.get_entries()) {
        if(entry.m_country_code == country) {
          region_filter = region_filter || Viper::sym("venue") ==
            Viper::literal(std::string(entry.m_venue.get_code().get_data()));
        }
      }
    }
    for(auto venue : query.get_index().get_venues()) {
      region_filter = region_filter || Viper::sym("venue") ==
        Viper::literal(std::string(venue.get_code().get_data()));
    }
    for(auto& security : query.get_index().get_securities()) {
      region_filter = region_filter ||
        Viper::sym("symbol") == security.get_symbol() &&
        Viper::sym("venue") == security.get_venue();
    }
    {
      auto reader = m_reader_pool.load();
      reader->execute(Viper::select(get_security_info_row(), "security_info",
        filter && anchor && region_filter,
        Viper::order_by({{"symbol", order}, {"venue", order}}),
        Viper::limit(query.get_snapshot_limit().get_size()),
        std::back_inserter(matches)));
    }
    if(query.get_snapshot_limit().get_type() ==
        Beam::SnapshotLimit::Type::TAIL) {
      std::reverse(matches.begin(), matches.end());
    }
    return matches;
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::store(const SecurityInfo& info) {
    auto writer = m_writer_pool.load();
    writer->execute(
      Viper::upsert(get_security_info_row(), "security_info", &info));
  }

  template<typename C>
  std::vector<SequencedOrderImbalance> SqlHistoricalDataStore<C>::
      load_order_imbalances(const VenueMarketDataQuery& query) {
    return m_order_imbalance_data_store.load(query);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::store(
      const SequencedVenueOrderImbalance& imbalance) {
    m_order_imbalance_data_store.store(imbalance);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::store(
      const std::vector<SequencedVenueOrderImbalance>& imbalances) {
    m_order_imbalance_data_store.store(imbalances);
  }

  template<typename C>
  std::vector<SequencedBboQuote> SqlHistoricalDataStore<C>::load_bbo_quotes(
      const SecurityMarketDataQuery& query) {
    return m_bbo_quote_data_store.load(query);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::store(
      const SequencedSecurityBboQuote& quote) {
    m_bbo_quote_data_store.store(quote);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::store(
      const std::vector<SequencedSecurityBboQuote>& quotes) {
    m_bbo_quote_data_store.store(quotes);
  }


  template<typename C>
  std::vector<SequencedBookQuote> SqlHistoricalDataStore<C>::load_book_quotes(
      const SecurityMarketDataQuery& query) {
    return m_book_quote_data_store.load(query);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::store(
      const SequencedSecurityBookQuote& quote) {
    m_book_quote_data_store.store(quote);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::store(
      const std::vector<SequencedSecurityBookQuote>& quotes) {
    m_book_quote_data_store.store(quotes);
  }

  template<typename C>
  std::vector<SequencedTimeAndSale> SqlHistoricalDataStore<C>::
      load_time_and_sales(const SecurityMarketDataQuery& query) {
    return m_time_and_sale_data_store.load(query);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::store(
      const SequencedSecurityTimeAndSale& time_and_sale) {
    m_time_and_sale_data_store.store(time_and_sale);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::store(
      const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) {
    m_time_and_sale_data_store.store(time_and_sales);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_time_and_sale_data_store.close();
    m_book_quote_data_store.close();
    m_bbo_quote_data_store.close();
    m_order_imbalance_data_store.close();
    m_writer_pool.close();
    m_reader_pool.close();
    m_open_state.close();
  }
}

#endif
