#ifndef NEXUS_MARKETDATACACHEDHISTORICALDATASTORE_HPP
#define NEXUS_MARKETDATACACHEDHISTORICALDATASTORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/CachedDataStore.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/HistoricalDataStoreQueryWrapper.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class CachedHistoricalDataStore
      \brief Caches historical market data.
      \tparam HistoricalDataStoreType The underlying data store to cache.
   */
  template<typename HistoricalDataStoreType>
  class CachedHistoricalDataStore : private boost::noncopyable {
    public:

      //! The type of HistoricalDataStore to cache.
      typedef typename Beam::TryDereferenceType<HistoricalDataStoreType>::type
        HistoricalDataStore;

      //! Constructs a CachedHistoricalDataStore.
      /*!
        \param dataStore Initializes the data store to commit data to.
        \param blockSize The size of a single cache block.
      */
      template<typename HistoricalDataStoreForward>
      CachedHistoricalDataStore(HistoricalDataStoreForward&& dataStore,
        int blockSize);

      ~CachedHistoricalDataStore();

      MarketEntry::InitialSequences LoadInitialSequences(MarketCode market);

      SecurityEntry::InitialSequences LoadInitialSequences(
        const Security& security);

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
      template<typename T>
      using DataStore = Beam::Queries::CachedDataStore<
        HistoricalDataStoreQueryWrapper<T, HistoricalDataStore*>,
        Queries::EvaluatorTranslator>;
      typename Beam::OptionalLocalPtr<HistoricalDataStoreType>::type
        m_dataStore;
      DataStore<OrderImbalance> m_orderImbalanceDataStore;
      DataStore<BboQuote> m_bboQuoteDataStore;
      DataStore<BookQuote> m_bookQuoteDataStore;
      DataStore<MarketQuote> m_marketQuoteDataStore;
      DataStore<TimeAndSale> m_timeAndSaleDataStore;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename HistoricalDataStoreType>
  template<typename HistoricalDataStoreForward>
  CachedHistoricalDataStore<HistoricalDataStoreType>::
      CachedHistoricalDataStore(HistoricalDataStoreForward&& dataStore,
      int blockSize)
      : m_dataStore(std::forward<HistoricalDataStoreForward>(dataStore)),
        m_orderImbalanceDataStore(&*m_dataStore, blockSize),
        m_bboQuoteDataStore(&*m_dataStore, blockSize),
        m_bookQuoteDataStore(&*m_dataStore, blockSize),
        m_marketQuoteDataStore(&*m_dataStore, blockSize),
        m_timeAndSaleDataStore(&*m_dataStore, blockSize) {}

  template<typename HistoricalDataStoreType>
  CachedHistoricalDataStore<HistoricalDataStoreType>::
      ~CachedHistoricalDataStore() {
    Close();
  }

  template<typename HistoricalDataStoreType>
  MarketEntry::InitialSequences CachedHistoricalDataStore<
      HistoricalDataStoreType>::LoadInitialSequences(MarketCode market) {
    MarketEntry::InitialSequences initialSequences;
    initialSequences.m_nextOrderImbalanceSequence =
      m_orderImbalanceDataStore.LoadInitialSequence(market);
    return initialSequences;
  }

  template<typename HistoricalDataStoreType>
  SecurityEntry::InitialSequences CachedHistoricalDataStore<
      HistoricalDataStoreType>::LoadInitialSequences(const Security& security) {
    SecurityEntry::InitialSequences initialSequences;
    initialSequences.m_nextBboQuoteSequence =
      m_bboQuoteDataStore.LoadInitialSequence(security);
    initialSequences.m_nextBookQuoteSequence =
      m_bookQuoteDataStore.LoadInitialSequence(security);
    initialSequences.m_nextMarketQuoteSequence =
      m_marketQuoteDataStore.LoadInitialSequence(security);
    initialSequences.m_nextTimeAndSaleSequence =
      m_timeAndSaleDataStore.LoadInitialSequence(security);
    return initialSequences;
  }

  template<typename HistoricalDataStoreType>
  std::vector<SequencedOrderImbalance> CachedHistoricalDataStore<
      HistoricalDataStoreType>::LoadOrderImbalances(
      const MarketWideDataQuery& query) {
    return m_orderImbalanceDataStore.Load(query);
  }

  template<typename HistoricalDataStoreType>
  std::vector<SequencedBboQuote> CachedHistoricalDataStore<
      HistoricalDataStoreType>::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bboQuoteDataStore.Load(query);
  }

  template<typename HistoricalDataStoreType>
  std::vector<SequencedBookQuote> CachedHistoricalDataStore<
      HistoricalDataStoreType>::LoadBookQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bookQuoteDataStore.Load(query);
  }

  template<typename HistoricalDataStoreType>
  std::vector<SequencedMarketQuote> CachedHistoricalDataStore<
      HistoricalDataStoreType>::LoadMarketQuotes(
      const SecurityMarketDataQuery& query) {
    return m_marketQuoteDataStore.Load(query);
  }

  template<typename HistoricalDataStoreType>
  std::vector<SequencedTimeAndSale> CachedHistoricalDataStore<
      HistoricalDataStoreType>::LoadTimeAndSales(
      const SecurityMarketDataQuery& query) {
    return m_timeAndSaleDataStore.Load(query);
  }

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    m_orderImbalanceDataStore.Store(orderImbalance);
  }

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    m_orderImbalanceDataStore.Store(orderImbalances);
  }

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    m_bboQuoteDataStore.Store(bboQuote);
  }

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    m_bboQuoteDataStore.Store(bboQuotes);
  }

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    m_marketQuoteDataStore.Store(marketQuote);
  }

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    m_marketQuoteDataStore.Store(marketQuotes);
  }

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    m_bookQuoteDataStore.Store(bookQuote);
  }

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    m_bookQuoteDataStore.Store(bookQuotes);
  }

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    m_timeAndSaleDataStore.Store(timeAndSale);
  }

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    m_timeAndSaleDataStore.Store(timeAndSales);
  }

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Open() {
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

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename HistoricalDataStoreType>
  void CachedHistoricalDataStore<HistoricalDataStoreType>::Shutdown() {
    m_timeAndSaleDataStore.Close();
    m_marketQuoteDataStore.Close();
    m_bookQuoteDataStore.Close();
    m_bboQuoteDataStore.Close();
    m_orderImbalanceDataStore.Close();
    m_dataStore->Close();
    m_openState.SetClosed();
  }
}
}

#endif
