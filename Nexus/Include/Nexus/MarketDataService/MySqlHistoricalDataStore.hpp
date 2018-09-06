#ifndef NEXUS_SQL_HISTORICAL_DATA_STORE_HPP
#define NEXUS_SQL_HISTORICAL_DATA_STORE_HPP
#include <thread>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/SqlDataStore.hpp>
#include <Beam/Sql/DatabaseConnectionPool.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Threading/ThreadPool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/HistoricalDataStoreException.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/SqlDefinitions.hpp"
#include "Nexus/Queries/SqlTranslator.hpp"

namespace Nexus::MarketDataService {

  /** Stores historical market data in an SQL database. */
  template<typename C>
  class SqlHistoricalDataStore : private boost::noncopyable {
    public:

      //! The type of SQL connection.
      using Connection = C;

      //! Constructs a MySqlHistoricalDataStore.
      /*!
        \param config The MySQL configuration.
      */
      SqlHistoricalDataStore(Beam::MySqlConfig config);

      ~SqlHistoricalDataStore();

      std::vector<SequencedOrderImbalance> LoadOrderImbalances(
        const MarketWideDataQuery& query);

      std::vector<SequencedBboQuote> LoadBboQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedBookQuote> LoadBookQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedMarketQuote> LoadMarketQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedTimeAndSale> LoadTimeAndSales(
        const SecurityMarketDataQuery& query);

      void Store(const SequencedMarketOrderImbalance& orderImbalance);

      void Store(const std::vector<SequencedMarketOrderImbalance>&
        orderImbalances);

      void Store(const SequencedSecurityBboQuote& bboQuote);

      void Store(const std::vector<SequencedSecurityBboQuote>& bboQuotes);

      void Store(const SequencedSecurityMarketQuote& marketQuote);

      void Store(const std::vector<SequencedSecurityMarketQuote>& marketQuotes);

      void Store(const SequencedSecurityBookQuote& bookQuote);

      void Store(const std::vector<SequencedSecurityBookQuote>& bookQuotes);

      void Store(const SequencedSecurityTimeAndSale& timeAndSale);

      void Store(const std::vector<SequencedSecurityTimeAndSale>& timeAndSales);

      void Open();

      void Close();

    private:
      template<typename V, typename I>
      using DataStore = Beam::Queries::SqlDataStore<Connection, V, I,
        Queries::SqlTranslator>;
      std::function<std::unique_ptr<Connection>> m_connectionBuilder;
      Beam::DatabaseConnectionPool<Connection> m_connectionPool;
      Beam::Threading::Sync<Connection, Beam::Threading::Mutex>
        m_writeConnection;
      Beam::Threading::ThreadPool m_threadPool;
      DataStore<OrderImbalanceRow, MarketCodeRow> m_orderImbalanceDataStore;
      DataStore<BboQuoteRow, SecurityRow> m_bboQuoteDataStore;
      DataStore<MarketQuoteRow, SecurityRow> m_marketQuoteDataStore;
      DataStore<BookQuoteRow SecurityRow> m_bookQuoteDataStore;
      DataStore<TimeAndSaleRow, SecurityRow> m_timeAndSaleDataStore;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline MySqlHistoricalDataStore::MySqlHistoricalDataStore(
      Beam::MySqlConfig config)
      : m_config(std::move(config)) {}

  inline MySqlHistoricalDataStore::~MySqlHistoricalDataStore() {
    Close();
  }

  inline std::vector<SequencedOrderImbalance> MySqlHistoricalDataStore::
      LoadOrderImbalances(const MarketWideDataQuery& query) {
    return m_orderImbalanceDataStore.Load(query);
  }

  inline std::vector<SequencedBboQuote> MySqlHistoricalDataStore::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bboQuoteDataStore.Load(query);
  }

  inline std::vector<SequencedBookQuote> MySqlHistoricalDataStore::
      LoadBookQuotes(const SecurityMarketDataQuery& query) {
    return m_bookQuoteDataStore.Load(query);
  }

  inline std::vector<SequencedMarketQuote> MySqlHistoricalDataStore::
      LoadMarketQuotes(const SecurityMarketDataQuery& query) {
    return m_marketQuoteDataStore.Load(query);
  }

  inline std::vector<SequencedTimeAndSale> MySqlHistoricalDataStore::
      LoadTimeAndSales(const SecurityMarketDataQuery& query) {
    return m_timeAndSaleDataStore.Load(query);
  }

  inline void MySqlHistoricalDataStore::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    return m_orderImbalanceDataStore.Store(orderImbalance);
  }

  inline void MySqlHistoricalDataStore::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    return m_orderImbalanceDataStore.Store(orderImbalances);
  }

  inline void MySqlHistoricalDataStore::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    return m_bboQuoteDataStore.Store(bboQuote);
  }

  inline void MySqlHistoricalDataStore::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    return m_bboQuoteDataStore.Store(bboQuotes);
  }

  inline void MySqlHistoricalDataStore::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    return m_marketQuoteDataStore.Store(marketQuote);
  }

  inline void MySqlHistoricalDataStore::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    return m_marketQuoteDataStore.Store(marketQuotes);
  }

  inline void MySqlHistoricalDataStore::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    return m_bookQuoteDataStore.Store(bookQuote);
  }

  inline void MySqlHistoricalDataStore::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    return m_bookQuoteDataStore.Store(bookQuotes);
  }

  inline void MySqlHistoricalDataStore::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    return m_timeAndSaleDataStore.Store(timeAndSale);
  }

  inline void MySqlHistoricalDataStore::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    return m_timeAndSaleDataStore.Store(timeAndSales);
  }

  inline void MySqlHistoricalDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      Beam::Threading::With(m_writeConnection,
        [&] (auto& connection) {
          connection.open();
        });
      for(auto i = std::size_t(0);
          i <= std::thread::hardware_concurrency(); ++i) {
        auto connection = std::make_unique<Viper::MySql::Connection>(
          m_config.m_address.GetHost(), m_config.m_address.GetPort(),
          m_config.m_username, m_config.m_password, m_config.m_schema);
        connection->open();
        m_connectionPool.Add(std::move(connection));
      }
      m_orderImbalanceDataStore.Open();
      m_bboQuoteDataStore.Open();
      m_marketQuoteDataStore.Open();
      m_bookQuoteDataStore.Open();
      m_timeAndSaleDataStore.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void MySqlHistoricalDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void MySqlHistoricalDataStore::Shutdown() {
    m_timeAndSaleDataStore.Close();
    m_bookQuoteDataStore.Close();
    m_marketQuoteDataStore.Close();
    m_bboQuoteDataStore.Close();
    m_orderImbalanceDataStore.Close();
    m_connectionPool.Close();
    Beam::Threading::With(m_writeConnection,
      [] (auto& connection) {
        connection.close();
      });
    m_openState.SetClosed();
  }
}
}

#endif
