#ifndef NEXUS_DATA_STORE_MARKET_DATA_CLIENT_HPP
#define NEXUS_DATA_STORE_MARKET_DATA_CLIENT_HPP
#include <memory>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus::MarketDataService {

  /**
   * Implements a MarketDataClient that directly queries a data store.
   * @param <D> The type of HistoricalDataStore to query.
   */
  template<typename D>
  class DataStoreMarketDataClient {
    public:

      /** The type of HistoricalDataStore to query. */
      using DataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a DataStoreMarketDataClient.
       * @param dataStore Initializes the HistoricalDataStore.
       */
      template<typename S>
      explicit DataStoreMarketDataClient(S&& dataStore);

      ~DataStoreMarketDataClient();

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedMarketQuote> queue);

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<MarketQuote> queue);

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);

      SecuritySnapshot LoadSecuritySnapshot(const Security& security);

      SecurityTechnicals LoadSecurityTechnicals(const Security& security);

      std::vector<SecurityInfo> QuerySecurityInfo(
        const SecurityInfoQuery& query);

      std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      Beam::IO::OpenState m_openState;

      DataStoreMarketDataClient(const DataStoreMarketDataClient&) = delete;
      DataStoreMarketDataClient& operator =(
        const DataStoreMarketDataClient&) = delete;
  };

  template<typename D>
  template<typename S>
  DataStoreMarketDataClient<D>::DataStoreMarketDataClient(S&& dataStore)
    : m_dataStore(std::forward<S>(dataStore)) {}

  template<typename D>
  DataStoreMarketDataClient<D>::~DataStoreMarketDataClient() {
    Close();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    auto values = m_dataStore->LoadOrderImbalances(query);
    for(auto& value : values) {
      queue.Push(std::move(value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    auto values = m_dataStore->LoadOrderImbalances(query);
    for(auto& value : values) {
      queue.Push(std::move(*value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    auto values = m_dataStore->LoadBboQuotes(query);
    for(auto& value : values) {
      queue.Push(std::move(value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    auto values = m_dataStore->LoadBboQuotes(query);
    for(auto& value : values) {
      queue.Push(std::move(*value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    auto values = m_dataStore->LoadBookQuotes(query);
    for(auto& value : values) {
      queue.Push(std::move(value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    auto values = m_dataStore->LoadBookQuotes(query);
    for(auto& value : values) {
      queue.Push(std::move(*value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedMarketQuote> queue) {
    auto values = m_dataStore->LoadMarketQuotes(query);
    for(auto& value : values) {
      queue.Push(std::move(value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<MarketQuote> queue) {
    auto values = m_dataStore->LoadMarketQuotes(query);
    for(auto& value : values) {
      queue.Push(std::move(*value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    auto values = m_dataStore->LoadTimeAndSales(query);
    for(auto& value : values) {
      queue.Push(std::move(value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    auto values = m_dataStore->LoadTimeAndSales(query);
    for(auto& value : values) {
      queue.Push(std::move(*value));
    }
  }

  template<typename D>
  SecuritySnapshot DataStoreMarketDataClient<D>::LoadSecuritySnapshot(
      const Security& security) {
    return {};
  }

  template<typename D>
  SecurityTechnicals DataStoreMarketDataClient<D>::LoadSecurityTechnicals(
      const Security& security) {
    return {};
  }

  template<typename D>
  std::vector<SecurityInfo> DataStoreMarketDataClient<D>::QuerySecurityInfo(
      const SecurityInfoQuery& query) {
    return m_dataStore->LoadSecurityInfo(query);
  }

  template<typename D>
  std::vector<SecurityInfo> DataStoreMarketDataClient<D>::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    return {};
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_dataStore->Close();
    m_openState.Close();
  }
}

#endif
