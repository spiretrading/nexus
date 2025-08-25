#ifndef NEXUS_MARKET_DATA_ASYNC_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_ASYNC_HISTORICAL_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/AsyncDataStore.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/MarketDataService/HistoricalDataStoreQueryWrapper.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus {

  /**
   * Implements a HistoricalDataStore using an AsyncDataStore as its backing.
   * @param <D> The underlying data store to commit the data to.
   */
  template<IsHistoricalDataStore D>
  class AsyncHistoricalDataStore {
    public:

      /** The underlying data store to commit the data to. */
      using HistoricalDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs an AsyncHistoricalDataStore.
       * @param data_store Initializes the data store to commit data to.
       */
      template<Beam::Initializes<D> DF>
      explicit AsyncHistoricalDataStore(DF&& data_store);

      ~AsyncHistoricalDataStore();
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
      template<typename T>
      using DataStore = Beam::Queries::AsyncDataStore<
        HistoricalDataStoreQueryWrapper<T, HistoricalDataStore*>,
        EvaluatorTranslator>;
      Beam::GetOptionalLocalPtr<D> m_data_store;
      LocalHistoricalDataStore m_security_info;
      DataStore<OrderImbalance> m_order_imbalance_data_store;
      DataStore<BboQuote> m_bbo_quote_data_store;
      DataStore<BookQuote> m_book_quote_data_store;
      DataStore<TimeAndSale> m_time_and_sale_data_store;
      Beam::IO::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      AsyncHistoricalDataStore(const AsyncHistoricalDataStore&) = delete;
      AsyncHistoricalDataStore& operator =(
        const AsyncHistoricalDataStore&) = delete;
  };

  template<IsHistoricalDataStore D>
  template<Beam::Initializes<D> DF>
  AsyncHistoricalDataStore<D>::AsyncHistoricalDataStore(DF&& data_store)
    : m_data_store(std::forward<DF>(data_store)),
      m_order_imbalance_data_store(&*m_data_store),
      m_bbo_quote_data_store(&*m_data_store),
      m_book_quote_data_store(&*m_data_store),
      m_time_and_sale_data_store(&*m_data_store) {}

  template<IsHistoricalDataStore D>
  AsyncHistoricalDataStore<D>::~AsyncHistoricalDataStore() {
    close();
  }

  template<IsHistoricalDataStore D>
  std::vector<SecurityInfo> AsyncHistoricalDataStore<D>::load_security_info(
      const SecurityInfoQuery& query) {
    auto local_info = m_security_info.load_security_info(query);
    auto persistent_info = m_data_store->load_security_info(query);
    auto info = std::vector<SecurityInfo>();
    Beam::MergeWithoutDuplicates(local_info.begin(), local_info.end(),
      persistent_info.begin(), persistent_info.end(), std::back_inserter(info),
      [] (const auto& left, const auto& right) {
        return left.m_security < right.m_security;
      });
    if(static_cast<int>(info.size()) > query.GetSnapshotLimit().GetSize()) {
      if(query.GetSnapshotLimit().GetType() ==
          Beam::Queries::SnapshotLimit::Type::HEAD) {
        info.erase(
          info.begin() + query.GetSnapshotLimit().GetSize(), info.end());
      } else {
        info.erase(info.begin(),
          info.begin() + (info.size() - query.GetSnapshotLimit().GetSize()));
      }
    }
    return info;
  }

  template<IsHistoricalDataStore D>
  void AsyncHistoricalDataStore<D>::store(const SecurityInfo& info) {
    m_security_info.store(info);
    m_tasks.Push([=, this] {
      m_data_store->store(info);
    });
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedOrderImbalance> AsyncHistoricalDataStore<D>::
      load_order_imbalances(const VenueMarketDataQuery& query) {
    return m_order_imbalance_data_store.Load(query);
  }

  template<IsHistoricalDataStore D>
  void AsyncHistoricalDataStore<D>::store(
      const SequencedVenueOrderImbalance& imbalance) {
    m_order_imbalance_data_store.Store(imbalance);
  }

  template<IsHistoricalDataStore D>
  void AsyncHistoricalDataStore<D>::store(
      const std::vector<SequencedVenueOrderImbalance>& imbalances) {
    m_order_imbalance_data_store.Store(imbalances);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedBboQuote> AsyncHistoricalDataStore<D>::load_bbo_quotes(
      const SecurityMarketDataQuery& query) {
    return m_bbo_quote_data_store.Load(query);
  }

  template<IsHistoricalDataStore D>
  void AsyncHistoricalDataStore<D>::store(
      const SequencedSecurityBboQuote& quote) {
    m_bbo_quote_data_store.Store(quote);
  }

  template<IsHistoricalDataStore D>
  void AsyncHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBboQuote>& quotes) {
    m_bbo_quote_data_store.Store(quotes);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedBookQuote> AsyncHistoricalDataStore<D>::load_book_quotes(
      const SecurityMarketDataQuery& query) {
    return m_book_quote_data_store.Load(query);
  }

  template<IsHistoricalDataStore D>
  void AsyncHistoricalDataStore<D>::store(
      const SequencedSecurityBookQuote& quote) {
    m_book_quote_data_store.Store(quote);
  }

  template<IsHistoricalDataStore D>
  void AsyncHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBookQuote>& quotes) {
    m_book_quote_data_store.Store(quotes);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedTimeAndSale>
      AsyncHistoricalDataStore<D>::load_time_and_sales(
        const SecurityMarketDataQuery& query) {
    return m_time_and_sale_data_store.Load(query);
  }

  template<IsHistoricalDataStore D>
  void AsyncHistoricalDataStore<D>::store(
      const SequencedSecurityTimeAndSale& time_and_sale) {
    m_time_and_sale_data_store.Store(time_and_sale);
  }

  template<IsHistoricalDataStore D>
  void AsyncHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) {
    m_time_and_sale_data_store.Store(time_and_sales);
  }

  template<IsHistoricalDataStore D>
  void AsyncHistoricalDataStore<D>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_time_and_sale_data_store.Close();
    m_book_quote_data_store.Close();
    m_bbo_quote_data_store.Close();
    m_order_imbalance_data_store.Close();
    m_tasks.Break();
    m_tasks.Wait();
    m_data_store->close();
    m_open_state.Close();
  }
}

#endif
