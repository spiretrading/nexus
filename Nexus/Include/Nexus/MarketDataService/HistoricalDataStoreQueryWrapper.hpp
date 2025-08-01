#ifndef NEXUS_MARKET_DATA_HISTORICAL_DATA_STORE_WRAPPER_HPP
#define NEXUS_MARKET_DATA_HISTORICAL_DATA_STORE_WRAPPER_HPP
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"

namespace Nexus::MarketDataService {

  /**
   * Wraps a HistoricalDataStore decomposing it into a query DataStore.
   * @param <V> The type of value to query.
   * @param <D> The HistoricalDataStore to wrap.
   */
  template<typename V, IsHistoricalDataStore D>
  class HistoricalDataStoreQueryWrapper {};

  template<typename Q, typename V, typename I, IsHistoricalDataStore D>
  class BaseHistoricalDataStoreQueryWrapper {
    public:
      using Query = Q;
      using Value = V;
      using Index = I;
      using DataStore = Beam::GetTryDereferenceType<D>;
      using SequencedValue = Beam::Queries::SequencedValue<Value>;
      using IndexedValue = Beam::Queries::SequencedValue<
        Beam::Queries::IndexedValue<Value, Index>>;

      template<Beam::Initializes<D> DF>
      explicit BaseHistoricalDataStoreQueryWrapper(DF&& data_store);

      virtual ~BaseHistoricalDataStoreQueryWrapper();

      void Store(const IndexedValue& value);

      void Store(const std::vector<IndexedValue>& values);

      void Close();

    protected:
      Beam::GetOptionalLocalPtr<D> m_data_store;
  };

  template<IsHistoricalDataStore D>
  class HistoricalDataStoreQueryWrapper<OrderImbalance, D> final :
      public BaseHistoricalDataStoreQueryWrapper<
        VenueMarketDataQuery, OrderImbalance, Venue, D> {
    public:
      template<Beam::Initializes<D> DF>
      explicit HistoricalDataStoreQueryWrapper(DF&& data_store);
      std::vector<SequencedOrderImbalance> Load(
        const VenueMarketDataQuery& query);
  };

  template<IsHistoricalDataStore D>
  class HistoricalDataStoreQueryWrapper<BboQuote, D> final :
      public BaseHistoricalDataStoreQueryWrapper<
        SecurityMarketDataQuery, BboQuote, Security, D> {
    public:
      template<Beam::Initializes<D> DF>
      explicit HistoricalDataStoreQueryWrapper(DF&& data_store);
      std::vector<SequencedBboQuote> Load(const SecurityMarketDataQuery& query);
  };

  template<IsHistoricalDataStore D>
  class HistoricalDataStoreQueryWrapper<BookQuote, D> final :
      public BaseHistoricalDataStoreQueryWrapper<
        SecurityMarketDataQuery, BookQuote, Security, D> {
    public:
      template<Beam::Initializes<D> DF>
      explicit HistoricalDataStoreQueryWrapper(DF&& data_store);
      std::vector<SequencedBookQuote> Load(
        const SecurityMarketDataQuery& query);
  };

  template<IsHistoricalDataStore D>
  class HistoricalDataStoreQueryWrapper<TimeAndSale, D> final :
      public BaseHistoricalDataStoreQueryWrapper<
        SecurityMarketDataQuery, TimeAndSale, Security, D> {
    public:
      template<Beam::Initializes<D> DF>
      explicit HistoricalDataStoreQueryWrapper(DF&& data_store);
      std::vector<SequencedTimeAndSale> Load(
        const SecurityMarketDataQuery& query);
  };

  template<typename Q, typename V, typename I, IsHistoricalDataStore D>
  template<Beam::Initializes<D> DF>
  BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::
    BaseHistoricalDataStoreQueryWrapper(DF&& data_store)
    : m_data_store(std::forward<DF>(data_store)) {}

  template<typename Q, typename V, typename I, IsHistoricalDataStore D>
  BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::
      ~BaseHistoricalDataStoreQueryWrapper() {
    Close();
  }

  template<typename Q, typename V, typename I, IsHistoricalDataStore D>
  void BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::Store(
      const IndexedValue& value) {
    m_data_store->store(value);
  }

  template<typename Q, typename V, typename I, IsHistoricalDataStore D>
  void BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::Store(
      const std::vector<IndexedValue>& values) {
    m_data_store->store(values);
  }

  template<typename Q, typename V, typename I, IsHistoricalDataStore D>
  void BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::Close() {
    m_data_store->close();
  }

  template<IsHistoricalDataStore D>
  template<Beam::Initializes<D> DF>
  HistoricalDataStoreQueryWrapper<OrderImbalance, D>::
    HistoricalDataStoreQueryWrapper(DF&& data_store)
    : BaseHistoricalDataStoreQueryWrapper<VenueMarketDataQuery, OrderImbalance,
        Venue, D>(std::forward<DF>(data_store)) {}

  template<IsHistoricalDataStore D>
  std::vector<SequencedOrderImbalance>
      HistoricalDataStoreQueryWrapper<OrderImbalance, D>::Load(
        const VenueMarketDataQuery& query) {
    return this->m_data_store->load_order_imbalances(query);
  }

  template<IsHistoricalDataStore D>
  template<Beam::Initializes<D> DF>
  HistoricalDataStoreQueryWrapper<BboQuote, D>::HistoricalDataStoreQueryWrapper(
    DF&& data_store)
    : BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery, BboQuote,
        Security, D>(std::forward<DF>(data_store)) {}

  template<IsHistoricalDataStore D>
  std::vector<SequencedBboQuote>
      HistoricalDataStoreQueryWrapper<BboQuote, D>::Load(
        const SecurityMarketDataQuery& query) {
    return this->m_data_store->load_bbo_quotes(query);
  }

  template<IsHistoricalDataStore D>
  template<Beam::Initializes<D> DF>
  HistoricalDataStoreQueryWrapper<BookQuote, D>::
    HistoricalDataStoreQueryWrapper(DF&& data_store)
    : BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery, BookQuote,
        Security, D>(std::forward<DF>(data_store)) {}

  template<IsHistoricalDataStore D>
  std::vector<SequencedBookQuote>
      HistoricalDataStoreQueryWrapper<BookQuote, D>::Load(
        const SecurityMarketDataQuery& query) {
    return this->m_data_store->load_book_quotes(query);
  }

  template<IsHistoricalDataStore D>
  template<Beam::Initializes<D> DF>
  HistoricalDataStoreQueryWrapper<TimeAndSale, D>::
    HistoricalDataStoreQueryWrapper(DF&& data_store)
    : BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery, TimeAndSale,
        Security, D>(std::forward<DF>(data_store)) {}

  template<IsHistoricalDataStore D>
  std::vector<SequencedTimeAndSale>
      HistoricalDataStoreQueryWrapper<TimeAndSale, D>::Load(
        const SecurityMarketDataQuery& query) {
    return this->m_data_store->load_time_and_sales(query);
  }
}

#endif
