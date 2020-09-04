#ifndef NEXUS_SQL_HISTORICAL_DATA_STORE_HPP
#define NEXUS_SQL_HISTORICAL_DATA_STORE_HPP
#include <functional>
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

  /**
   * Stores historical market data in an SQL database.
   * @param <C> The type of SQL connection.
   */
  template<typename C>
  class SqlHistoricalDataStore : private boost::noncopyable {
    public:

      /** The type of SQL connection. */
      using Connection = C;

      /** The callable used to build SQL connections. */
      using ConnectionBuilder = std::function<Connection ()>;

      /**
       * Constructs an SqlHistoricalDataStore.
       * @param connectionBuilder The callable used to build SQL connections.
       */
      SqlHistoricalDataStore(ConnectionBuilder connectionBuilder);

      ~SqlHistoricalDataStore();

      boost::optional<SecurityInfo> LoadSecurityInfo(const Security& security);

      std::vector<SecurityInfo> LoadAllSecurityInfo();

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

      void Store(const SecurityInfo& info);

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

      void Close();

    private:
      template<typename V, typename I>
      using DataStore = Beam::Queries::SqlDataStore<Connection, V, I,
        Queries::SqlTranslator>;
      ConnectionBuilder m_connectionBuilder;
      Beam::DatabaseConnectionPool<Connection> m_readerPool;
      Beam::DatabaseConnectionPool<Connection> m_writerPool;
      Beam::Threading::ThreadPool m_threadPool;
      DataStore<Viper::Row<OrderImbalance>, Viper::Row<MarketCode>>
        m_orderImbalanceDataStore;
      DataStore<Viper::Row<BboQuote>, Viper::Row<Security>> m_bboQuoteDataStore;
      DataStore<Viper::Row<MarketQuote>, Viper::Row<Security>>
        m_marketQuoteDataStore;
      DataStore<Viper::Row<BookQuote>, Viper::Row<Security>>
        m_bookQuoteDataStore;
      DataStore<Viper::Row<TimeAndSale>, Viper::Row<Security>>
        m_timeAndSaleDataStore;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename C>
  SqlHistoricalDataStore<C>::SqlHistoricalDataStore(
      ConnectionBuilder connectionBuilder)
      : m_connectionBuilder(std::move(connectionBuilder)),
        m_orderImbalanceDataStore("order_imbalances", GetOrderImbalanceRow(),
          GetMarketCodeRow(), Beam::Ref(m_readerPool), Beam::Ref(m_writerPool),
          Beam::Ref(m_threadPool)),
        m_bboQuoteDataStore("bbo_quotes", GetBboQuoteRow(), GetSecurityRow(),
          Beam::Ref(m_readerPool), Beam::Ref(m_writerPool),
          Beam::Ref(m_threadPool)),
        m_marketQuoteDataStore("market_quotes", GetMarketQuoteRow(),
          GetSecurityRow(), Beam::Ref(m_readerPool), Beam::Ref(m_writerPool),
          Beam::Ref(m_threadPool)),
        m_bookQuoteDataStore("book_quotes", GetBookQuoteRow(), GetSecurityRow(),
          Beam::Ref(m_readerPool), Beam::Ref(m_writerPool),
          Beam::Ref(m_threadPool)),
        m_timeAndSaleDataStore("time_and_sales", GetTimeAndSaleRow(),
          GetSecurityRow(), Beam::Ref(m_readerPool), Beam::Ref(m_writerPool),
          Beam::Ref(m_threadPool)) {
    m_openState.SetOpening();
    try {
      for(auto i = std::size_t(0);
          i <= std::thread::hardware_concurrency(); ++i) {
        auto readerConnection =
          std::make_unique<Connection>(m_connectionBuilder());
        readerConnection->open();
        m_readerPool.Add(std::move(readerConnection));
      }
      auto writerConnection =
        std::make_unique<Connection>(m_connectionBuilder());
      writerConnection->open();
      writerConnection->execute(Viper::create_if_not_exists(
        GetSecurityInfoRow(), "security_info"));
      m_writerPool.Add(std::move(writerConnection));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C>
  SqlHistoricalDataStore<C>::~SqlHistoricalDataStore() {
    Close();
  }

  template<typename C>
  boost::optional<SecurityInfo> SqlHistoricalDataStore<C>::LoadSecurityInfo(
      const Security& security) {
    auto info = std::optional<SecurityInfo>();
    {
      auto reader = m_readerPool.Acquire();
      reader->execute(Viper::select(GetSecurityInfoRow(), "security_info",
        Viper::sym("symbol") == security.GetSymbol() &&
        Viper::sym("country") == security.GetCountry(), &info));
    }
    if(info.has_value()) {
      return std::move(*info);
    }
    return boost::none;
  }

  template<typename C>
  std::vector<SecurityInfo> SqlHistoricalDataStore<C>::LoadAllSecurityInfo() {
    auto result = std::vector<SecurityInfo>();
    {
      auto reader = m_readerPool.Acquire();
      reader->execute(Viper::select(GetSecurityInfoRow(), "security_info",
        std::back_inserter(result)));
    }
    return result;
  }

  template<typename C>
  std::vector<SequencedOrderImbalance> SqlHistoricalDataStore<C>::
      LoadOrderImbalances(const MarketWideDataQuery& query) {
    return m_orderImbalanceDataStore.Load(query);
  }

  template<typename C>
  std::vector<SequencedBboQuote> SqlHistoricalDataStore<C>::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bboQuoteDataStore.Load(query);
  }

  template<typename C>
  std::vector<SequencedBookQuote> SqlHistoricalDataStore<C>::LoadBookQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bookQuoteDataStore.Load(query);
  }

  template<typename C>
  std::vector<SequencedMarketQuote> SqlHistoricalDataStore<C>::LoadMarketQuotes(
      const SecurityMarketDataQuery& query) {
    return m_marketQuoteDataStore.Load(query);
  }

  template<typename C>
  std::vector<SequencedTimeAndSale> SqlHistoricalDataStore<C>::LoadTimeAndSales(
      const SecurityMarketDataQuery& query) {
    return m_timeAndSaleDataStore.Load(query);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(const SecurityInfo& info) {
    auto writer = m_writerPool.Acquire();
    writer->execute(Viper::upsert(GetSecurityInfoRow(), "security_info",
      &info));
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    m_orderImbalanceDataStore.Store(orderImbalance);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    m_orderImbalanceDataStore.Store(orderImbalances);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    m_bboQuoteDataStore.Store(bboQuote);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    m_bboQuoteDataStore.Store(bboQuotes);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    m_marketQuoteDataStore.Store(marketQuote);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    m_marketQuoteDataStore.Store(marketQuotes);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    m_bookQuoteDataStore.Store(bookQuote);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    m_bookQuoteDataStore.Store(bookQuotes);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    m_timeAndSaleDataStore.Store(timeAndSale);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    m_timeAndSaleDataStore.Store(timeAndSales);
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C>
  void SqlHistoricalDataStore<C>::Shutdown() {
    m_timeAndSaleDataStore.Close();
    m_bookQuoteDataStore.Close();
    m_marketQuoteDataStore.Close();
    m_bboQuoteDataStore.Close();
    m_orderImbalanceDataStore.Close();
    m_writerPool.Close();
    m_readerPool.Close();
    m_openState.SetClosed();
  }
}

#endif
