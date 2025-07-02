#ifndef NEXUS_MARKET_DATA_HISTORICAL_DATA_STORE_WRAPPER_HPP
#define NEXUS_MARKET_DATA_HISTORICAL_DATA_STORE_WRAPPER_HPP
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /**
   * Wraps a HistoricalDataStore decomposing it into a query DataStore.
   * @param <V> The type of value to query.
   * @param <D> The HistoricalDataStore to wrap.
   */
  template<typename V, typename D>
  class HistoricalDataStoreQueryWrapper {};

  template<typename Q, typename V, typename I, typename D>
  class BaseHistoricalDataStoreQueryWrapper {
    public:
      using Query = Q;
      using Value = V;
      using Index = I;
      using DataStore = Beam::GetTryDereferenceType<D>;
      using SequencedValue = Beam::Queries::SequencedValue<Value>;
      using IndexedValue = Beam::Queries::SequencedValue<
        Beam::Queries::IndexedValue<Value, Index>>;

      template<typename DF>
      explicit BaseHistoricalDataStoreQueryWrapper(DF&& dataStore);

      virtual ~BaseHistoricalDataStoreQueryWrapper();

      void Store(const IndexedValue& value);

      void Store(const std::vector<IndexedValue>& values);

      void Close();

    protected:
      Beam::GetOptionalLocalPtr<D> m_dataStore;
  };

  template<typename D>
  class HistoricalDataStoreQueryWrapper<OrderImbalance, D> :
      public BaseHistoricalDataStoreQueryWrapper<VenueQuery,
        OrderImbalance, Venue, D> {
    public:
      template<typename DF>
      explicit HistoricalDataStoreQueryWrapper(DF&& dataStore)
        : BaseHistoricalDataStoreQueryWrapper<VenueQuery,
            OrderImbalance, Venue, D>(std::forward<DF>(dataStore)) {}

      std::vector<SequencedOrderImbalance> Load(const VenueQuery& query) {
        return this->m_dataStore->LoadOrderImbalances(query);
      }
  };

  template<typename D>
  class HistoricalDataStoreQueryWrapper<BboQuote, D> :
      public BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
        BboQuote, Security, D> {
    public:
      template<typename DF>
      explicit HistoricalDataStoreQueryWrapper(DF&& dataStore)
        : BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery, BboQuote,
            Security, D>(std::forward<DF>(dataStore)) {}

      std::vector<SequencedBboQuote> Load(
          const SecurityMarketDataQuery& query) {
        return this->m_dataStore->LoadBboQuotes(query);
      }
  };

  template<typename D>
  class HistoricalDataStoreQueryWrapper<BookQuote, D> :
      public BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
        BookQuote, Security, D> {
    public:
      template<typename DF>
      explicit HistoricalDataStoreQueryWrapper(DF&& dataStore)
        : BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
            BookQuote, Security, D>(std::forward<DF>(dataStore)) {}

      std::vector<SequencedBookQuote> Load(
          const SecurityMarketDataQuery& query) {
        return this->m_dataStore->LoadBookQuotes(query);
      }
  };

  template<typename D>
  class HistoricalDataStoreQueryWrapper<TimeAndSale, D> :
      public BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
        TimeAndSale, Security, D> {
    public:
      template<typename DF>
      explicit HistoricalDataStoreQueryWrapper(DF&& dataStore)
        : BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
            TimeAndSale, Security, D>(std::forward<DF>(dataStore)) {}

      std::vector<SequencedTimeAndSale> Load(
          const SecurityMarketDataQuery& query) {
        return this->m_dataStore->LoadTimeAndSales(query);
      }
  };

  template<typename Q, typename V, typename I, typename D>
  template<typename DF>
  BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::
    BaseHistoricalDataStoreQueryWrapper(DF&& dataStore)
    : m_dataStore(std::forward<DF>(dataStore)) {}

  template<typename Q, typename V, typename I, typename D>
  BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::
      ~BaseHistoricalDataStoreQueryWrapper() {
    Close();
  }

  template<typename Q, typename V, typename I, typename D>
  void BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::Store(
      const IndexedValue& value) {
    m_dataStore->Store(value);
  }

  template<typename Q, typename V, typename I, typename D>
  void BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::Store(
      const std::vector<IndexedValue>& values) {
    return m_dataStore->Store(values);
  }

  template<typename Q, typename V, typename I, typename D>
  void BaseHistoricalDataStoreQueryWrapper<Q, V, I, D>::Close() {
    m_dataStore->Close();
  }
}

#endif
