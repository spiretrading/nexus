#ifndef NEXUS_MARKETDATAHISTORICALDATASTOREWRAPPER_HPP
#define NEXUS_MARKETDATAHISTORICALDATASTOREWRAPPER_HPP
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class HistoricalDataStoreQueryWrapper
      \brief Wraps a HistoricalDataStore decomposing it into a query DataStore.
      \tparam ValueType The type of value to query.
      \tparam DataStoreType The HistoricalDataStore to wrap.
   */
  template<typename T, typename DataStoreType>
  class HistoricalDataStoreQueryWrapper {};

  template<typename QueryType, typename ValueType, typename IndexType,
    typename DataStoreType>
  class BaseHistoricalDataStoreQueryWrapper {
    public:
      typedef QueryType Query;
      typedef ValueType Value;
      typedef IndexType Index;
      typedef typename Beam::TryDereferenceType<DataStoreType>::type DataStore;
      typedef Beam::Queries::SequencedValue<Value> SequencedValue;
      typedef Beam::Queries::SequencedValue<
        Beam::Queries::IndexedValue<Value, Index>> IndexedValue;

      template<typename DataStoreForward>
      BaseHistoricalDataStoreQueryWrapper(DataStoreForward&& dataStore);

      ~BaseHistoricalDataStoreQueryWrapper();

      void Store(const IndexedValue& value);

      void Store(const std::vector<IndexedValue>& values);

      void Open();

      void Close();

    protected:
      typename Beam::OptionalLocalPtr<DataStoreType>::type m_dataStore;
  };

  template<typename DataStoreType>
  class HistoricalDataStoreQueryWrapper<OrderImbalance, DataStoreType> :
      public BaseHistoricalDataStoreQueryWrapper<MarketWideDataQuery,
      OrderImbalance, MarketCode, DataStoreType> {
    public:
      template<typename DataStoreForward>
      HistoricalDataStoreQueryWrapper(DataStoreForward&& dataStore)
          : BaseHistoricalDataStoreQueryWrapper<MarketWideDataQuery,
              OrderImbalance, MarketCode, DataStoreType>(
              std::forward<DataStoreForward>(dataStore)) {}

      Beam::Queries::Sequence LoadInitialSequence(const MarketCode& market) {
        return this->m_dataStore->LoadInitialSequences(
          market).m_nextOrderImbalanceSequence;
      }
      std::vector<SequencedOrderImbalance> Load(
          const MarketWideDataQuery& query) {
        return this->m_dataStore->LoadOrderImbalances(query);
      }
  };

  template<typename DataStoreType>
  class HistoricalDataStoreQueryWrapper<BboQuote, DataStoreType> :
      public BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
      BboQuote, Security, DataStoreType> {
    public:
      template<typename DataStoreForward>
      HistoricalDataStoreQueryWrapper(DataStoreForward&& dataStore)
          : BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
              BboQuote, Security, DataStoreType>(
              std::forward<DataStoreForward>(dataStore)) {}

      Beam::Queries::Sequence LoadInitialSequence(const Security& security) {
        return this->m_dataStore->LoadInitialSequences(
          security).m_nextBboQuoteSequence;
      }

      std::vector<SequencedBboQuote> Load(
          const SecurityMarketDataQuery& query) {
        return this->m_dataStore->LoadBboQuotes(query);
      }
  };

  template<typename DataStoreType>
  class HistoricalDataStoreQueryWrapper<BookQuote, DataStoreType> :
      public BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
      BookQuote, Security, DataStoreType> {
    public:
      template<typename DataStoreForward>
      HistoricalDataStoreQueryWrapper(DataStoreForward&& dataStore)
          : BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
              BookQuote, Security, DataStoreType>(
              std::forward<DataStoreForward>(dataStore)) {}

      Beam::Queries::Sequence LoadInitialSequence(const Security& security) {
        return this->m_dataStore->LoadInitialSequences(
          security).m_nextBookQuoteSequence;
      }

      std::vector<SequencedBookQuote> Load(
          const SecurityMarketDataQuery& query) {
        return this->m_dataStore->LoadBookQuotes(query);
      }
  };

  template<typename DataStoreType>
  class HistoricalDataStoreQueryWrapper<MarketQuote, DataStoreType> :
      public BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
      MarketQuote, Security, DataStoreType> {
    public:
      template<typename DataStoreForward>
      HistoricalDataStoreQueryWrapper(DataStoreForward&& dataStore)
          : BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
              MarketQuote, Security, DataStoreType>(
              std::forward<DataStoreForward>(dataStore)) {}

      Beam::Queries::Sequence LoadInitialSequence(const Security& security) {
        return this->m_dataStore->LoadInitialSequences(
          security).m_nextMarketQuoteSequence;
      }

      std::vector<SequencedMarketQuote> Load(
          const SecurityMarketDataQuery& query) {
        return this->m_dataStore->LoadMarketQuotes(query);
      }
  };

  template<typename DataStoreType>
  class HistoricalDataStoreQueryWrapper<TimeAndSale, DataStoreType> :
      public BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
      TimeAndSale, Security, DataStoreType> {
    public:
      template<typename DataStoreForward>
      HistoricalDataStoreQueryWrapper(DataStoreForward&& dataStore)
          : BaseHistoricalDataStoreQueryWrapper<SecurityMarketDataQuery,
              TimeAndSale, Security, DataStoreType>(
              std::forward<DataStoreForward>(dataStore)) {}

      Beam::Queries::Sequence LoadInitialSequence(const Security& security) {
        return this->m_dataStore->LoadInitialSequences(
          security).m_nextTimeAndSaleSequence;
      }

      std::vector<SequencedTimeAndSale> Load(
          const SecurityMarketDataQuery& query) {
        return this->m_dataStore->LoadTimeAndSales(query);
      }
  };

  template<typename QueryType, typename ValueType, typename IndexType,
    typename DataStoreType>
  template<typename DataStoreForward>
  BaseHistoricalDataStoreQueryWrapper<QueryType, ValueType, IndexType,
      DataStoreType>::BaseHistoricalDataStoreQueryWrapper(
      DataStoreForward&& dataStore)
      : m_dataStore(std::forward<DataStoreForward>(dataStore)) {}

  template<typename QueryType, typename ValueType, typename IndexType,
    typename DataStoreType>
  BaseHistoricalDataStoreQueryWrapper<QueryType, ValueType, IndexType,
      DataStoreType>::~BaseHistoricalDataStoreQueryWrapper() {
    Close();
  }

  template<typename QueryType, typename ValueType, typename IndexType,
    typename DataStoreType>
  void BaseHistoricalDataStoreQueryWrapper<QueryType, ValueType, IndexType,
      DataStoreType>::Store(const IndexedValue& value) {
    m_dataStore->Store(value);
  }

  template<typename QueryType, typename ValueType, typename IndexType,
    typename DataStoreType>
  void BaseHistoricalDataStoreQueryWrapper<QueryType, ValueType, IndexType,
      DataStoreType>::Store(const std::vector<IndexedValue>& values) {
    return m_dataStore->Store(values);
  }

  template<typename QueryType, typename ValueType, typename IndexType,
    typename DataStoreType>
  void BaseHistoricalDataStoreQueryWrapper<QueryType, ValueType, IndexType,
      DataStoreType>::Open() {
    m_dataStore->Open();
  }

  template<typename QueryType, typename ValueType, typename IndexType,
    typename DataStoreType>
  void BaseHistoricalDataStoreQueryWrapper<QueryType, ValueType, IndexType,
      DataStoreType>::Close() {
    m_dataStore->Close();
  }
}
}

#endif
