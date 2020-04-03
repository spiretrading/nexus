#ifndef NEXUS_DATA_STORE_MARKET_DATA_CLIENT_HPP
#define NEXUS_DATA_STORE_MARKET_DATA_CLIENT_HPP
#include <memory>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/QueueWriter.hpp>
#include <boost/noncopyable.hpp>
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
  class DataStoreMarketDataClient : private boost::noncopyable {
    public:

      /** The type of HistoricalDataStore to query. */
      using DataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a DataStoreMarketDataClient.
       * @param dataStore Initializes the HistoricalDataStore.
       */
      template<typename S>
      DataStoreMarketDataClient(S&& dataStore);

      ~DataStoreMarketDataClient();

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedOrderImbalance>>& queue);

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue);

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue);

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue);

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue);

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue);

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue);

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue);

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue);

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue);

      SecuritySnapshot LoadSecuritySnapshot(const Security& security);

      SecurityTechnicals LoadSecurityTechnicals(const Security& security);

      boost::optional<SecurityInfo> LoadSecurityInfo(const Security& security);

      std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix);

      void Open();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      Beam::IO::OpenState m_openState;

      void Shutdown();
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
      const MarketWideDataQuery& query, const std::shared_ptr<
      Beam::QueueWriter<SequencedOrderImbalance>>& queue) {
    auto values = m_dataStore->LoadOrderImbalances(query);
    for(auto& value : values) {
      queue->Push(std::move(value));
    }
    queue->Break();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) {
    auto values = m_dataStore->LoadOrderImbalances(query);
    for(auto& value : values) {
      queue->Push(std::move(*value));
    }
    queue->Break();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue) {
    auto values = m_dataStore->LoadBboQuotes(query);
    for(auto& value : values) {
      queue->Push(std::move(value));
    }
    queue->Break();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) {
    auto values = m_dataStore->LoadBboQuotes(query);
    for(auto& value : values) {
      queue->Push(std::move(*value));
    }
    queue->Break();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue) {
    auto values = m_dataStore->LoadBookQuotes(query);
    for(auto& value : values) {
      queue->Push(std::move(value));
    }
    queue->Break();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) {
    auto values = m_dataStore->LoadBookQuotes(query);
    for(auto& value : values) {
      queue->Push(std::move(*value));
    }
    queue->Break();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedMarketQuote>>& queue) {
    auto values = m_dataStore->LoadMarketQuotes(query);
    for(auto& value : values) {
      queue->Push(std::move(value));
    }
    queue->Break();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) {
    auto values = m_dataStore->LoadMarketQuotes(query);
    for(auto& value : values) {
      queue->Push(std::move(*value));
    }
    queue->Break();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue) {
    auto values = m_dataStore->LoadTimeAndSales(query);
    for(auto& value : values) {
      queue->Push(std::move(value));
    }
    queue->Break();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) {
    auto values = m_dataStore->LoadTimeAndSales(query);
    for(auto& value : values) {
      queue->Push(std::move(*value));
    }
    queue->Break();
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
  boost::optional<SecurityInfo> DataStoreMarketDataClient<D>::LoadSecurityInfo(
      const Security& security) {
    return m_dataStore->LoadSecurityInfo(security);
  }

  template<typename D>
  std::vector<SecurityInfo> DataStoreMarketDataClient<D>::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    return {};
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStore->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::Shutdown() {
    m_dataStore->Close();
    m_openState.SetClosed();
  }
}

#endif
