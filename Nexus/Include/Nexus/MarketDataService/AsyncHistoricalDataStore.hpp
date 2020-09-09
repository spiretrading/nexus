#ifndef NEXUS_MARKET_DATA_ASYNC_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_ASYNC_HISTORICAL_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/AsyncDataStore.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/HistoricalDataStoreQueryWrapper.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus::MarketDataService {

  /**
   * Implements a HistoricalDataStore using an AsyncDataStore as its backing.
   * @param <D> The underlying data store to commit the data to.
   */
  template<typename D>
  class AsyncHistoricalDataStore : private boost::noncopyable {
    public:

      /** The underlying data store to commit the data to. */
      using HistoricalDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs an AsyncHistoricalDataStore.
       * @param dataStore Initializes the data store to commit data to.
       */
      template<typename HistoricalDataStoreForward>
      AsyncHistoricalDataStore(HistoricalDataStoreForward&& dataStore);

      ~AsyncHistoricalDataStore();

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
      using DataStore = Beam::Queries::AsyncDataStore<
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
  AsyncHistoricalDataStore<D>::AsyncHistoricalDataStore(
    HistoricalDataStoreForward&& dataStore)
    : m_dataStore(std::forward<HistoricalDataStoreForward>(dataStore)),
      m_orderImbalanceDataStore(&*m_dataStore),
      m_bboQuoteDataStore(&*m_dataStore),
      m_bookQuoteDataStore(&*m_dataStore),
      m_marketQuoteDataStore(&*m_dataStore),
      m_timeAndSaleDataStore(&*m_dataStore) {
    m_openState.SetOpen();
  }

  template<typename D>
  AsyncHistoricalDataStore<D>::~AsyncHistoricalDataStore() {
    Close();
  }

  template<typename D>
  boost::optional<SecurityInfo> AsyncHistoricalDataStore<D>::LoadSecurityInfo(
      const Security& security) {
    return m_dataStore->LoadSecurityInfo(security);
  }

  template<typename D>
  std::vector<SecurityInfo> AsyncHistoricalDataStore<D>::LoadAllSecurityInfo() {
    return m_dataStore->LoadAllSecurityInfo();
  }

  template<typename D>
  std::vector<SequencedOrderImbalance> AsyncHistoricalDataStore<D>::
      LoadOrderImbalances(const MarketWideDataQuery& query) {
    return m_orderImbalanceDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedBboQuote> AsyncHistoricalDataStore<D>::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bboQuoteDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedBookQuote> AsyncHistoricalDataStore<D>::
      LoadBookQuotes(const SecurityMarketDataQuery& query) {
    return m_bookQuoteDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedMarketQuote> AsyncHistoricalDataStore<D>::
      LoadMarketQuotes(const SecurityMarketDataQuery& query) {
    return m_marketQuoteDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedTimeAndSale> AsyncHistoricalDataStore<D>::
      LoadTimeAndSales(const SecurityMarketDataQuery& query) {
    return m_timeAndSaleDataStore.Load(query);
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Store(const SecurityInfo& info) {
    m_dataStore->Store(info);
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    m_orderImbalanceDataStore.Store(orderImbalance);
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    m_orderImbalanceDataStore.Store(orderImbalances);
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    m_bboQuoteDataStore.Store(bboQuote);
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    m_bboQuoteDataStore.Store(bboQuotes);
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    m_marketQuoteDataStore.Store(marketQuote);
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    m_marketQuoteDataStore.Store(marketQuotes);
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    m_bookQuoteDataStore.Store(bookQuote);
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    m_bookQuoteDataStore.Store(bookQuotes);
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    m_timeAndSaleDataStore.Store(timeAndSale);
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    m_timeAndSaleDataStore.Store(timeAndSales);
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename D>
  void AsyncHistoricalDataStore<D>::Shutdown() {
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
