#ifndef NEXUS_MARKET_DATA_ASYNC_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_ASYNC_HISTORICAL_DATA_STORE_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/AsyncDataStore.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include "Nexus/MarketDataService/HistoricalDataStoreQueryWrapper.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus::MarketDataService {

  /**
   * Implements a HistoricalDataStore using an AsyncDataStore as its backing.
   * @param <D> The underlying data store to commit the data to.
   */
  template<typename D>
  class AsyncHistoricalDataStore {
    public:

      /** The underlying data store to commit the data to. */
      using HistoricalDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs an AsyncHistoricalDataStore.
       * @param dataStore Initializes the data store to commit data to.
       */
      template<typename DF>
      AsyncHistoricalDataStore(DF&& dataStore);

      ~AsyncHistoricalDataStore();

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
      using DataStore = Beam::Queries::AsyncDataStore<
        HistoricalDataStoreQueryWrapper<T, HistoricalDataStore*>,
        Queries::EvaluatorTranslator>;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      LocalHistoricalDataStore m_securityInfo;
      DataStore<OrderImbalance> m_orderImbalanceDataStore;
      DataStore<BboQuote> m_bboQuoteDataStore;
      DataStore<BookQuote> m_bookQuoteDataStore;
      DataStore<MarketQuote> m_marketQuoteDataStore;
      DataStore<TimeAndSale> m_timeAndSaleDataStore;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      AsyncHistoricalDataStore(const AsyncHistoricalDataStore&) = delete;
      AsyncHistoricalDataStore& operator =(
        const AsyncHistoricalDataStore&) = delete;
  };

  template<typename D>
  template<typename DF>
  AsyncHistoricalDataStore<D>::AsyncHistoricalDataStore(DF&& dataStore)
    : m_dataStore(std::forward<DF>(dataStore)),
      m_orderImbalanceDataStore(&*m_dataStore),
      m_bboQuoteDataStore(&*m_dataStore),
      m_bookQuoteDataStore(&*m_dataStore),
      m_marketQuoteDataStore(&*m_dataStore),
      m_timeAndSaleDataStore(&*m_dataStore) {}

  template<typename D>
  AsyncHistoricalDataStore<D>::~AsyncHistoricalDataStore() {
    Close();
  }

  template<typename D>
  std::vector<SecurityInfo> AsyncHistoricalDataStore<D>::LoadSecurityInfo(
      const SecurityInfoQuery& query) {
    auto localInfo = m_securityInfo.LoadSecurityInfo(query);
    auto persistentInfo = m_dataStore->LoadSecurityInfo(query);
    auto info = std::vector<SecurityInfo>();
    Beam::MergeWithoutDuplicates(localInfo.begin(), localInfo.end(),
      persistentInfo.begin(), persistentInfo.end(), std::back_inserter(info),
      [] (const auto& left, const auto& right) {
        return left.m_security < right.m_security;
      });
    if(static_cast<int>(info.size()) > query.GetSnapshotLimit().GetSize()) {
      if(query.GetSnapshotLimit().GetType() ==
          Beam::Queries::SnapshotLimit::Type::HEAD) {
        info.erase(info.begin() + query.GetSnapshotLimit().GetSize(),
          info.end());
      } else {
        info.erase(info.begin(),
          info.begin() + (info.size() - query.GetSnapshotLimit().GetSize()));
      }
    }
    return info;
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
    m_securityInfo.Store(info);
    m_tasks.Push([=, this] {
      m_dataStore->Store(info);
    });
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
    m_timeAndSaleDataStore.Close();
    m_marketQuoteDataStore.Close();
    m_bookQuoteDataStore.Close();
    m_bboQuoteDataStore.Close();
    m_orderImbalanceDataStore.Close();
    m_tasks.Break();
    m_tasks.Wait();
    m_dataStore->Close();
    m_openState.Close();
  }
}

#endif
