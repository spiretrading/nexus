#ifndef NEXUS_MARKET_DATA_SESSION_CACHED_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_SESSION_CACHED_HISTORICAL_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/SessionCachedDataStore.hpp>
#include "Nexus/MarketDataService/HistoricalDataStoreQueryWrapper.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus::MarketDataService {

  /**
   * Caches historical market data for a specified session.
   * @param <D> The underlying data store to cache.
   */
  template<typename D>
  class SessionCachedHistoricalDataStore {
    public:

      /** The type of HistoricalDataStore to cache. */
      using HistoricalDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a SessionCachedHistoricalDataStore.
       * @param dataStore Initializes the data store to commit data to.
       * @param blockSize The size of a single cache block.
       */
      template<typename DF>
      SessionCachedHistoricalDataStore(DF&& dataStore, int blockSize);

      ~SessionCachedHistoricalDataStore();

      std::vector<SecurityInfo> LoadSecurityInfo(
        const SecurityInfoQuery& query);

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
      template<typename T>
      using DataStore = Beam::Queries::SessionCachedDataStore<
        HistoricalDataStoreQueryWrapper<T, HistoricalDataStore*>,
        Queries::EvaluatorTranslator>;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      DataStore<BboQuote> m_bboQuoteDataStore;
      DataStore<BookQuote> m_bookQuoteDataStore;
      DataStore<MarketQuote> m_marketQuoteDataStore;
      DataStore<TimeAndSale> m_timeAndSaleDataStore;
      Beam::IO::OpenState m_openState;

      SessionCachedHistoricalDataStore(
        const SessionCachedHistoricalDataStore&) = delete;
      SessionCachedHistoricalDataStore& operator =(
        const SessionCachedHistoricalDataStore&) = delete;
  };

  template<typename D>
  template<typename DF>
  SessionCachedHistoricalDataStore<D>::SessionCachedHistoricalDataStore(
    DF&& dataStore, int blockSize)
    : m_dataStore(std::forward<DF>(dataStore)),
      m_bboQuoteDataStore(&*m_dataStore, blockSize / 10),
      m_bookQuoteDataStore(&*m_dataStore, blockSize / 10),
      m_marketQuoteDataStore(&*m_dataStore, blockSize / 10),
      m_timeAndSaleDataStore(&*m_dataStore, blockSize) {}

  template<typename D>
  SessionCachedHistoricalDataStore<D>::~SessionCachedHistoricalDataStore() {
    Close();
  }

  template<typename D>
  std::vector<SecurityInfo> SessionCachedHistoricalDataStore<D>::
      LoadSecurityInfo(const SecurityInfoQuery& query) {
    return m_dataStore->LoadSecurityInfo(query);
  }

  template<typename D>
  std::vector<SequencedOrderImbalance> SessionCachedHistoricalDataStore<D>::
      LoadOrderImbalances(const MarketWideDataQuery& query) {
    return m_dataStore->LoadOrderImbalances(query);
  }

  template<typename D>
  std::vector<SequencedBboQuote> SessionCachedHistoricalDataStore<D>::
      LoadBboQuotes(const SecurityMarketDataQuery& query) {
    return m_bboQuoteDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedBookQuote> SessionCachedHistoricalDataStore<D>::
      LoadBookQuotes(const SecurityMarketDataQuery& query) {
    return m_bookQuoteDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedMarketQuote> SessionCachedHistoricalDataStore<D>::
      LoadMarketQuotes(const SecurityMarketDataQuery& query) {
    return m_marketQuoteDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedTimeAndSale> SessionCachedHistoricalDataStore<D>::
      LoadTimeAndSales(const SecurityMarketDataQuery& query) {
    return m_timeAndSaleDataStore.Load(query);
  }

  template<typename D>
  void SessionCachedHistoricalDataStore<D>::Store(const SecurityInfo& info) {
    m_dataStore->Store(info);
  }

  template<typename D>
  void SessionCachedHistoricalDataStore<D>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    m_dataStore->Store(orderImbalance);
  }

  template<typename D>
  void SessionCachedHistoricalDataStore<D>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    m_dataStore->Store(orderImbalances);
  }

  template<typename D>
  void SessionCachedHistoricalDataStore<D>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    m_bboQuoteDataStore.Store(bboQuote);
  }

  template<typename D>
  void SessionCachedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    m_bboQuoteDataStore.Store(bboQuotes);
  }

  template<typename D>
  void SessionCachedHistoricalDataStore<D>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    m_marketQuoteDataStore.Store(marketQuote);
  }

  template<typename D>
  void SessionCachedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    m_marketQuoteDataStore.Store(marketQuotes);
  }

  template<typename D>
  void SessionCachedHistoricalDataStore<D>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    m_bookQuoteDataStore.Store(bookQuote);
  }

  template<typename D>
  void SessionCachedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    m_bookQuoteDataStore.Store(bookQuotes);
  }

  template<typename D>
  void SessionCachedHistoricalDataStore<D>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    m_timeAndSaleDataStore.Store(timeAndSale);
  }

  template<typename D>
  void SessionCachedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    m_timeAndSaleDataStore.Store(timeAndSales);
  }

  template<typename D>
  void SessionCachedHistoricalDataStore<D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_timeAndSaleDataStore.Close();
    m_marketQuoteDataStore.Close();
    m_bookQuoteDataStore.Close();
    m_bboQuoteDataStore.Close();
    m_dataStore->Close();
    m_openState.Close();
  }
}

#endif
