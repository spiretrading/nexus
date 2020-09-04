#ifndef NEXUS_MARKET_DATA_CACHED_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_CACHED_HISTORICAL_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/CachedDataStore.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/HistoricalDataStoreQueryWrapper.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus::MarketDataService {

  /**
   * Caches historical market data.
   * @param <D> The underlying data store to cache.
   */
  template<typename D>
  class CachedHistoricalDataStore : private boost::noncopyable {
    public:

      /** The type of HistoricalDataStore to cache. */
      using HistoricalDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a CachedHistoricalDataStore.
       * @param dataStore Initializes the data store to commit data to.
       * @param blockSize The size of a single cache block.
       */
      template<typename HistoricalDataStoreForward>
      CachedHistoricalDataStore(HistoricalDataStoreForward&& dataStore,
        int blockSize);

      ~CachedHistoricalDataStore();

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
      template<typename T>
      using DataStore = Beam::Queries::CachedDataStore<
        HistoricalDataStoreQueryWrapper<T, HistoricalDataStore*>,
        Queries::EvaluatorTranslator>;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      DataStore<OrderImbalance> m_orderImbalanceDataStore;
      DataStore<BboQuote> m_bboQuoteDataStore;
      DataStore<BookQuote> m_bookQuoteDataStore;
      DataStore<MarketQuote> m_marketQuoteDataStore;
      DataStore<TimeAndSale> m_timeAndSaleDataStore;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename D>
  template<typename HistoricalDataStoreForward>
  CachedHistoricalDataStore<D>::CachedHistoricalDataStore(
      HistoricalDataStoreForward&& dataStore, int blockSize)
      : m_dataStore(std::forward<HistoricalDataStoreForward>(dataStore)),
        m_orderImbalanceDataStore(&*m_dataStore, blockSize),
        m_bboQuoteDataStore(&*m_dataStore, blockSize),
        m_bookQuoteDataStore(&*m_dataStore, blockSize),
        m_marketQuoteDataStore(&*m_dataStore, blockSize),
        m_timeAndSaleDataStore(&*m_dataStore, blockSize) {
    m_openState.SetOpen();
  }

  template<typename D>
  CachedHistoricalDataStore<D>::~CachedHistoricalDataStore() {
    Close();
  }

  template<typename D>
  boost::optional<SecurityInfo> CachedHistoricalDataStore<D>::LoadSecurityInfo(
      const Security& security) {
    return m_dataStore->LoadSecurityInfo(security);
  }

  template<typename D>
  std::vector<SecurityInfo> CachedHistoricalDataStore<D>::
      LoadAllSecurityInfo() {
    return m_dataStore->LoadAllSecurityInfo();
  }

  template<typename D>
  std::vector<SequencedOrderImbalance> CachedHistoricalDataStore<D>::
      LoadOrderImbalances(const MarketWideDataQuery& query) {
    return m_orderImbalanceDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedBboQuote> CachedHistoricalDataStore<D>::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bboQuoteDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedBookQuote> CachedHistoricalDataStore<D>::LoadBookQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bookQuoteDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedMarketQuote> CachedHistoricalDataStore<D>::
      LoadMarketQuotes(const SecurityMarketDataQuery& query) {
    return m_marketQuoteDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedTimeAndSale> CachedHistoricalDataStore<D>::
      LoadTimeAndSales(const SecurityMarketDataQuery& query) {
    return m_timeAndSaleDataStore.Load(query);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Store(const SecurityInfo& info) {
    m_dataStore->Store(info);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    m_orderImbalanceDataStore.Store(orderImbalance);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    m_orderImbalanceDataStore.Store(orderImbalances);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    m_bboQuoteDataStore.Store(bboQuote);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    m_bboQuoteDataStore.Store(bboQuotes);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    m_marketQuoteDataStore.Store(marketQuote);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    m_marketQuoteDataStore.Store(marketQuotes);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    m_bookQuoteDataStore.Store(bookQuote);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    m_bookQuoteDataStore.Store(bookQuotes);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    m_timeAndSaleDataStore.Store(timeAndSale);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    m_timeAndSaleDataStore.Store(timeAndSales);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::Shutdown() {
    m_timeAndSaleDataStore.Close();
    m_marketQuoteDataStore.Close();
    m_bookQuoteDataStore.Close();
    m_bboQuoteDataStore.Close();
    m_orderImbalanceDataStore.Close();
    m_dataStore->Close();
    m_openState.SetClosed();
  }
}

#endif
