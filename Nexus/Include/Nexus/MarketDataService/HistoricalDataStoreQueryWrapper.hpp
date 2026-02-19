#ifndef NEXUS_MARKET_DATA_HISTORICAL_DATA_STORE_WRAPPER_HPP
#define NEXUS_MARKET_DATA_HISTORICAL_DATA_STORE_WRAPPER_HPP
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"

namespace Nexus {

  /**
   * Wraps a HistoricalDataStore decomposing it into a query DataStore.
   * @param <V> The type of value to query.
   * @param <D> The HistoricalDataStore to wrap.
   */
  template<typename V, typename D> requires
    IsHistoricalDataStore<Beam::dereference_t<D>>
  class HistoricalDataStoreQueryWrapper {};

  template<typename Q, typename V, typename I, typename D> requires
    IsHistoricalDataStore<Beam::dereference_t<D>>
  class BaseHistoricalDataStoreQueryWrapper {
    public:
      using Query = Q;
      using Value = V;
      using Index = I;
      using DataStore = Beam::dereference_t<D>;
      using SequencedValue = Beam::SequencedValue<Value>;
      using IndexedValue =
        Beam::SequencedValue<Beam::IndexedValue<Value, Index>>;

      /**
       * Constructs a BaseHistoricalDataStoreQueryWrapper.
       * @param data_store Initializes the underlying data store.
       */
      template<Beam::Initializes<D> DF>
      explicit BaseHistoricalDataStoreQueryWrapper(DF&& data_store);

      virtual ~BaseHistoricalDataStoreQueryWrapper();

      void store(const IndexedValue& value);
      void store(const std::vector<IndexedValue>& values);
      void close();

    protected:
      Beam::local_ptr_t<D> m_data_store;
  };

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  class HistoricalDataStoreQueryWrapper<OrderImbalance, D> final :
      public BaseHistoricalDataStoreQueryWrapper<
        VenueMarketDataQuery, OrderImbalance, Venue, D> {
    public:
      template<Beam::Initializes<D> DF>
      explicit HistoricalDataStoreQueryWrapper(DF&& data_store);

      std::vector<SequencedOrderImbalance> load(
        const VenueMarketDataQuery& query);
  };

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  class HistoricalDataStoreQueryWrapper<BboQuote, D> final :
      public BaseHistoricalDataStoreQueryWrapper<
        TickerQuery, BboQuote, Ticker, D> {
    public:
      template<Beam::Initializes<D> DF>
      explicit HistoricalDataStoreQueryWrapper(DF&& data_store);

      std::vector<SequencedBboQuote> load(const TickerQuery& query);
  };

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  class HistoricalDataStoreQueryWrapper<BookQuote, D> final :
      public BaseHistoricalDataStoreQueryWrapper<
        TickerQuery, BookQuote, Ticker, D> {
    public:
      template<Beam::Initializes<D> DF>
      explicit HistoricalDataStoreQueryWrapper(DF&& data_store);

      std::vector<SequencedBookQuote> load(const TickerQuery& query);
  };

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  class HistoricalDataStoreQueryWrapper<TimeAndSale, D> final :
      public BaseHistoricalDataStoreQueryWrapper<
        TickerQuery, TimeAndSale, Ticker, D> {
    public:
      template<Beam::Initializes<D> DF>
      explicit HistoricalDataStoreQueryWrapper(DF&& data_store);

      std::vector<SequencedTimeAndSale> load(const TickerQuery& query);
  };

  template<typename Q, typename V, typename I, typename D> requires
    IsHistoricalDataStore<Beam::dereference_t<D>>
  template<Beam::Initializes<D> DF>
  BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::
    BaseHistoricalDataStoreQueryWrapper(DF&& data_store)
    : m_data_store(std::forward<DF>(data_store)) {}

  template<typename Q, typename V, typename I, typename D> requires
    IsHistoricalDataStore<Beam::dereference_t<D>>
  BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::
      ~BaseHistoricalDataStoreQueryWrapper() {
    close();
  }

  template<typename Q, typename V, typename I, typename D> requires
    IsHistoricalDataStore<Beam::dereference_t<D>>
  void BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::store(
      const IndexedValue& value) {
    m_data_store->store(value);
  }

  template<typename Q, typename V, typename I, typename D> requires
    IsHistoricalDataStore<Beam::dereference_t<D>>
  void BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::store(
      const std::vector<IndexedValue>& values) {
    m_data_store->store(values);
  }

  template<typename Q, typename V, typename I, typename D> requires
    IsHistoricalDataStore<Beam::dereference_t<D>>
  void BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::close() {
    m_data_store->close();
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  template<Beam::Initializes<D> DF>
  HistoricalDataStoreQueryWrapper<OrderImbalance, D>::
    HistoricalDataStoreQueryWrapper(DF&& data_store)
    : BaseHistoricalDataStoreQueryWrapper<VenueMarketDataQuery, OrderImbalance,
        Venue, D>(std::forward<DF>(data_store)) {}

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedOrderImbalance>
      HistoricalDataStoreQueryWrapper<OrderImbalance, D>::load(
        const VenueMarketDataQuery& query) {
    return this->m_data_store->load_order_imbalances(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  template<Beam::Initializes<D> DF>
  HistoricalDataStoreQueryWrapper<BboQuote, D>::HistoricalDataStoreQueryWrapper(
    DF&& data_store)
    : BaseHistoricalDataStoreQueryWrapper<
        TickerQuery, BboQuote, Ticker, D>(std::forward<DF>(data_store)) {}

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedBboQuote>
      HistoricalDataStoreQueryWrapper<BboQuote, D>::load(
        const TickerQuery& query) {
    return this->m_data_store->load_bbo_quotes(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  template<Beam::Initializes<D> DF>
  HistoricalDataStoreQueryWrapper<BookQuote, D>::
    HistoricalDataStoreQueryWrapper(DF&& data_store)
    : BaseHistoricalDataStoreQueryWrapper<
        TickerQuery, BookQuote, Ticker, D>(std::forward<DF>(data_store)) {}

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedBookQuote>
      HistoricalDataStoreQueryWrapper<BookQuote, D>::load(
        const TickerQuery& query) {
    return this->m_data_store->load_book_quotes(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  template<Beam::Initializes<D> DF>
  HistoricalDataStoreQueryWrapper<TimeAndSale, D>::
    HistoricalDataStoreQueryWrapper(DF&& data_store)
    : BaseHistoricalDataStoreQueryWrapper<
        TickerQuery, TimeAndSale, Ticker, D>(std::forward<DF>(data_store)) {}

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedTimeAndSale>
      HistoricalDataStoreQueryWrapper<TimeAndSale, D>::load(
        const TickerQuery& query) {
    return this->m_data_store->load_time_and_sales(query);
  }
}

#endif
