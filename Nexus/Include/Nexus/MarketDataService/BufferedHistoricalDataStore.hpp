#ifndef NEXUS_MARKET_DATA_BUFFERED_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_BUFFERED_HISTORICAL_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/BufferedDataStore.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/HistoricalDataStoreQueryWrapper.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus::MarketDataService {

  /**
   * Buffers historical market data before committing it to an underlying data
   * store.
   * @param <D> The underlying data store to commit the data to.
   */
  template<typename D>
  class BufferedHistoricalDataStore : private boost::noncopyable {
    public:

      /** The type of HistoricalDataStore to buffer. */
      using HistoricalDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a BufferedHistoricalDataStore.
       * @param dataStore Initializes the data store to commit data to.
       * @param bufferSize The number of messages to buffer before committing to
       *        to the <i>dataStore</i>.
       * @param threadPool The ThreadPool to queue the writes to.
       */
      template<typename HistoricalDataStoreForward>
      BufferedHistoricalDataStore(HistoricalDataStoreForward&& dataStore,
        std::size_t bufferSize,
        Beam::Ref<Beam::Threading::ThreadPool> threadPool);

      ~BufferedHistoricalDataStore();

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

      void Open();

      void Close();

    private:
      template<typename T>
      using DataStore = Beam::Queries::BufferedDataStore<
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
  BufferedHistoricalDataStore<D>::BufferedHistoricalDataStore(
    HistoricalDataStoreForward&& dataStore, std::size_t bufferSize,
    Beam::Ref<Beam::Threading::ThreadPool> threadPool)
    : m_dataStore(std::forward<HistoricalDataStoreForward>(dataStore)),
      m_orderImbalanceDataStore(&*m_dataStore, bufferSize,
        Beam::Ref(threadPool)),
      m_bboQuoteDataStore(&*m_dataStore, bufferSize, Beam::Ref(threadPool)),
      m_bookQuoteDataStore(&*m_dataStore, bufferSize, Beam::Ref(threadPool)),
      m_marketQuoteDataStore(&*m_dataStore, bufferSize, Beam::Ref(threadPool)),
      m_timeAndSaleDataStore(&*m_dataStore, bufferSize,
        Beam::Ref(threadPool)) {}

  template<typename D>
  BufferedHistoricalDataStore<D>::~BufferedHistoricalDataStore() {
    Close();
  }

  template<typename D>
  boost::optional<SecurityInfo> BufferedHistoricalDataStore<D>::
      LoadSecurityInfo(const Security& security) {
    return m_dataStore->LoadSecurityInfo(security);
  }

  template<typename D>
  std::vector<SecurityInfo> BufferedHistoricalDataStore<D>::
      LoadAllSecurityInfo() {
    return m_dataStore->LoadAllSecurityInfo();
  }

  template<typename D>
  std::vector<SequencedOrderImbalance> BufferedHistoricalDataStore<D>::
      LoadOrderImbalances(const MarketWideDataQuery& query) {
    return m_orderImbalanceDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedBboQuote> BufferedHistoricalDataStore<D>::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bboQuoteDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedBookQuote> BufferedHistoricalDataStore<D>::
      LoadBookQuotes(const SecurityMarketDataQuery& query) {
    return m_bookQuoteDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedMarketQuote> BufferedHistoricalDataStore<D>::
      LoadMarketQuotes(const SecurityMarketDataQuery& query) {
    return m_marketQuoteDataStore.Load(query);
  }

  template<typename D>
  std::vector<SequencedTimeAndSale> BufferedHistoricalDataStore<D>::
      LoadTimeAndSales(const SecurityMarketDataQuery& query) {
    return m_timeAndSaleDataStore.Load(query);
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Store(const SecurityInfo& info) {
    m_dataStore->Store(info);
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    m_orderImbalanceDataStore.Store(orderImbalance);
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    m_orderImbalanceDataStore.Store(orderImbalances);
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    m_bboQuoteDataStore.Store(bboQuote);
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    m_bboQuoteDataStore.Store(bboQuotes);
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    m_marketQuoteDataStore.Store(marketQuote);
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    m_marketQuoteDataStore.Store(marketQuotes);
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    m_bookQuoteDataStore.Store(bookQuote);
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    m_bookQuoteDataStore.Store(bookQuotes);
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    m_timeAndSaleDataStore.Store(timeAndSale);
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    m_timeAndSaleDataStore.Store(timeAndSales);
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStore->Open();
      m_orderImbalanceDataStore.Open();
      m_bboQuoteDataStore.Open();
      m_bookQuoteDataStore.Open();
      m_marketQuoteDataStore.Open();
      m_timeAndSaleDataStore.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename D>
  void BufferedHistoricalDataStore<D>::Shutdown() {
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
