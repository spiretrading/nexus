#ifndef NEXUS_DATA_STORE_MARKET_DATA_CLIENT_HPP
#define NEXUS_DATA_STORE_MARKET_DATA_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

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
      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);
      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);
      std::vector<SecurityInfo> query(const SecurityInfoQuery& query);
      SecuritySnapshot load_snapshot(const Security& security);
      SecurityTechnicals load_technicals(const Security& security);
      std::vector<SecurityInfo> load_security_info_from_prefix(
        const std::string& prefix);
      void close();

    private:
      Beam::GetOptionalLocalPtr<D> m_data_store;
      Beam::IO::OpenState m_open_state;

      DataStoreMarketDataClient(const DataStoreMarketDataClient&) = delete;
      DataStoreMarketDataClient& operator =(
        const DataStoreMarketDataClient&) = delete;
  };

  template<typename D>
  template<typename S>
  DataStoreMarketDataClient<D>::DataStoreMarketDataClient(S&& data_store)
    : m_data_store(std::forward<S>(data_store)) {}

  template<typename D>
  DataStoreMarketDataClient<D>::~DataStoreMarketDataClient() {
    close();
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    auto values = m_data_store->load_order_imbalances(query);
    for(auto& value : values) {
      queue.Push(std::move(value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    auto values = m_data_store->load_order_imbalances(query);
    for(auto& value : values) {
      queue.Push(std::move(*value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    auto values = m_data_store->load_bbo_quotes(query);
    for(auto& value : values) {
      queue.Push(std::move(value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    auto values = m_data_store->load_bbo_quotes(query);
    for(auto& value : values) {
      queue.Push(std::move(*value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    auto values = m_data_store->load_book_quotes(query);
    for(auto& value : values) {
      queue.Push(std::move(value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    auto values = m_data_store->load_book_quotes(query);
    for(auto& value : values) {
      queue.Push(std::move(*value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    auto values = m_data_store->load_time_and_sales(query);
    for(auto& value : values) {
      queue.Push(std::move(value));
    }
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    auto values = m_data_store->load_time_and_sales(query);
    for(auto& value : values) {
      queue.Push(std::move(*value));
    }
  }

  template<typename D>
  std::vector<SecurityInfo> DataStoreMarketDataClient<D>::query(
      const SecurityInfoQuery& query) {
    return m_data_store->load_security_info(query);
  }

  template<typename D>
  SecuritySnapshot DataStoreMarketDataClient<D>::load_snapshot(
      const Security& security) {
    return {};
  }

  template<typename D>
  SecurityTechnicals DataStoreMarketDataClient<D>::load_technicals(
      const Security& security) {
    return {};
  }

  template<typename D>
  std::vector<SecurityInfo>
      DataStoreMarketDataClient<D>::load_security_info_from_prefix(
        const std::string& prefix) {
    return {};
  }

  template<typename D>
  void DataStoreMarketDataClient<D>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_data_store->close();
    m_open_state.Close();
  }
}

#endif
