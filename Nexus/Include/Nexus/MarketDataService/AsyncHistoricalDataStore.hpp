#ifndef NEXUS_MARKET_DATA_ASYNC_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_ASYNC_HISTORICAL_DATA_STORE_HPP
#include <algorithm>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queries/AsyncDataStore.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/MarketDataService/HistoricalDataStoreQueryWrapper.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus {

  /**
   * Implements a HistoricalDataStore using an AsyncDataStore as its backing.
   * @param <D> The underlying data store to commit the data to.
   */
  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  class AsyncHistoricalDataStore {
    public:

      /** The underlying data store to commit the data to. */
      using HistoricalDataStore = Beam::dereference_t<D>;

      /**
       * Constructs an AsyncHistoricalDataStore.
       * @param data_store Initializes the data store to commit data to.
       */
      template<Beam::Initializes<D> DF>
      explicit AsyncHistoricalDataStore(DF&& data_store);

      ~AsyncHistoricalDataStore();

      std::vector<TickerInfo> load_ticker_info(const TickerInfoQuery& query);
      void store(const TickerInfo& info);
      std::vector<SequencedOrderImbalance> load_order_imbalances(
        const VenueMarketDataQuery& query);
      void store(const SequencedVenueOrderImbalance& imbalance);
      void store(const std::vector<SequencedVenueOrderImbalance>& imbalances);
      std::vector<SequencedBboQuote> load_bbo_quotes(
        const TickerMarketDataQuery& query);
      void store(const SequencedTickerBboQuote& quote);
      void store(const std::vector<SequencedTickerBboQuote>& quotes);
      std::vector<SequencedBookQuote> load_book_quotes(
        const TickerMarketDataQuery& query);
      void store(const SequencedTickerBookQuote& quote);
      void store(const std::vector<SequencedTickerBookQuote>& quotes);
      std::vector<SequencedTimeAndSale> load_time_and_sales(
        const TickerMarketDataQuery& query);
      void store(const SequencedTickerTimeAndSale& time_and_sale);
      void store(const std::vector<SequencedTickerTimeAndSale>& time_and_sales);
      void close();

    private:
      template<typename T>
      using DataStore = Beam::AsyncDataStore<
        HistoricalDataStoreQueryWrapper<T, HistoricalDataStore*>,
        EvaluatorTranslator>;
      Beam::local_ptr_t<D> m_data_store;
      LocalHistoricalDataStore m_ticker_info;
      DataStore<OrderImbalance> m_order_imbalance_data_store;
      DataStore<BboQuote> m_bbo_quote_data_store;
      DataStore<BookQuote> m_book_quote_data_store;
      DataStore<TimeAndSale> m_time_and_sale_data_store;
      Beam::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      AsyncHistoricalDataStore(const AsyncHistoricalDataStore&) = delete;
      AsyncHistoricalDataStore& operator =(
        const AsyncHistoricalDataStore&) = delete;
  };

  template<typename D>
  AsyncHistoricalDataStore(D&&) ->
    AsyncHistoricalDataStore<std::remove_cvref_t<D>>;

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  template<Beam::Initializes<D> DF>
  AsyncHistoricalDataStore<D>::AsyncHistoricalDataStore(DF&& data_store)
    : m_data_store(std::forward<DF>(data_store)),
      m_order_imbalance_data_store(&*m_data_store),
      m_bbo_quote_data_store(&*m_data_store),
      m_book_quote_data_store(&*m_data_store),
      m_time_and_sale_data_store(&*m_data_store) {}

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  AsyncHistoricalDataStore<D>::~AsyncHistoricalDataStore() {
    close();
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<TickerInfo> AsyncHistoricalDataStore<D>::load_ticker_info(
      const TickerInfoQuery& query) {
    auto local_info = m_ticker_info.load_ticker_info(query);
    auto persistent_info = m_data_store->load_ticker_info(query);
    auto info = std::vector<TickerInfo>();
    std::ranges::set_union(local_info, persistent_info,
      std::back_inserter(info), [] (const auto& left, const auto& right) {
        return left.m_ticker < right.m_ticker;
      });
    if(static_cast<int>(info.size()) > query.get_snapshot_limit().get_size()) {
      if(query.get_snapshot_limit().get_type() ==
          Beam::SnapshotLimit::Type::HEAD) {
        info.erase(
          info.begin() + query.get_snapshot_limit().get_size(), info.end());
      } else {
        info.erase(info.begin(),
          info.begin() + (info.size() - query.get_snapshot_limit().get_size()));
      }
    }
    return info;
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void AsyncHistoricalDataStore<D>::store(const TickerInfo& info) {
    m_ticker_info.store(info);
    m_tasks.push([=, this] {
      m_data_store->store(info);
    });
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedOrderImbalance> AsyncHistoricalDataStore<D>::
      load_order_imbalances(const VenueMarketDataQuery& query) {
    return m_order_imbalance_data_store.load(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void AsyncHistoricalDataStore<D>::store(
      const SequencedVenueOrderImbalance& imbalance) {
    m_order_imbalance_data_store.store(imbalance);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void AsyncHistoricalDataStore<D>::store(
      const std::vector<SequencedVenueOrderImbalance>& imbalances) {
    m_order_imbalance_data_store.store(imbalances);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedBboQuote> AsyncHistoricalDataStore<D>::load_bbo_quotes(
      const TickerMarketDataQuery& query) {
    return m_bbo_quote_data_store.load(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void AsyncHistoricalDataStore<D>::store(
      const SequencedTickerBboQuote& quote) {
    m_bbo_quote_data_store.store(quote);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void AsyncHistoricalDataStore<D>::store(
      const std::vector<SequencedTickerBboQuote>& quotes) {
    m_bbo_quote_data_store.store(quotes);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedBookQuote> AsyncHistoricalDataStore<D>::load_book_quotes(
      const TickerMarketDataQuery& query) {
    return m_book_quote_data_store.load(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void AsyncHistoricalDataStore<D>::store(
      const SequencedTickerBookQuote& quote) {
    m_book_quote_data_store.store(quote);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void AsyncHistoricalDataStore<D>::store(
      const std::vector<SequencedTickerBookQuote>& quotes) {
    m_book_quote_data_store.store(quotes);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedTimeAndSale>
      AsyncHistoricalDataStore<D>::load_time_and_sales(
        const TickerMarketDataQuery& query) {
    return m_time_and_sale_data_store.load(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void AsyncHistoricalDataStore<D>::store(
      const SequencedTickerTimeAndSale& time_and_sale) {
    m_time_and_sale_data_store.store(time_and_sale);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void AsyncHistoricalDataStore<D>::store(
      const std::vector<SequencedTickerTimeAndSale>& time_and_sales) {
    m_time_and_sale_data_store.store(time_and_sales);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void AsyncHistoricalDataStore<D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_time_and_sale_data_store.close();
    m_book_quote_data_store.close();
    m_bbo_quote_data_store.close();
    m_order_imbalance_data_store.close();
    m_tasks.close();
    m_tasks.wait();
    m_data_store->close();
    m_open_state.close();
  }
}

#endif
