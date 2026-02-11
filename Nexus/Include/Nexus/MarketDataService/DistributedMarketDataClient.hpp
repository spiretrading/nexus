#ifndef NEXUS_DISTRIBUTED_MARKET_DATA_CLIENT_HPP
#define NEXUS_DISTRIBUTED_MARKET_DATA_CLIENT_HPP
#include <memory>
#include <unordered_set>
#include <Beam/IO/OpenState.hpp>
#include "Nexus/Definitions/ScopeTable.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus {

  /**
   * Implements a MarketDataClient whose servers are distributed among multiple
   * instances.
   */
  class DistributedMarketDataClient {
    public:

      /**
       * Constructs a DistributedMarketDataClient.
       * @param market_data_clients Maps regions to the appropriate market data
       *        client.
       */
      explicit DistributedMarketDataClient(
        ScopeTable<std::shared_ptr<MarketDataClient>> market_data_clients);

      ~DistributedMarketDataClient();

      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);
      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);
      void query(const TickerMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);
      void query(const TickerMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);
      void query(const TickerMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);
      void query(const TickerMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);
      void query(const TickerMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);
      void query(const TickerMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);
      std::vector<TickerInfo> query(const TickerInfoQuery& query);
      TickerSnapshot load_snapshot(const Ticker& ticker);
      PriceCandlestick load_session_candlestick(const Ticker& ticker);
      std::vector<TickerInfo> load_ticker_info_from_prefix(
        const std::string& prefix);
      void close();

    private:
      ScopeTable<std::shared_ptr<MarketDataClient>> m_market_data_clients;
      Beam::OpenState m_open_state;

      DistributedMarketDataClient(const DistributedMarketDataClient&) = delete;
      DistributedMarketDataClient& operator =(
        const DistributedMarketDataClient&) = delete;
  };

  inline DistributedMarketDataClient::DistributedMarketDataClient(
    ScopeTable<std::shared_ptr<MarketDataClient>> market_data_clients)
    : m_market_data_clients(std::move(market_data_clients)) {}

  inline DistributedMarketDataClient::~DistributedMarketDataClient() {
    close();
  }

  inline void DistributedMarketDataClient::query(
      const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    if(auto client = m_market_data_clients.get(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  inline void DistributedMarketDataClient::query(
      const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    if(auto client = m_market_data_clients.get(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  inline void DistributedMarketDataClient::query(
      const TickerMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    if(auto client = m_market_data_clients.get(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  inline void DistributedMarketDataClient::query(
      const TickerMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    if(auto client = m_market_data_clients.get(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  inline void DistributedMarketDataClient::query(
      const TickerMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    if(auto client = m_market_data_clients.get(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  inline void DistributedMarketDataClient::query(
      const TickerMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    if(auto client = m_market_data_clients.get(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  inline void DistributedMarketDataClient::query(
      const TickerMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    if(auto client = m_market_data_clients.get(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  inline void DistributedMarketDataClient::query(
      const TickerMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    if(auto client = m_market_data_clients.get(query.get_index())) {
      client->query(query, std::move(queue));
    } else {
      queue.close();
    }
  }

  inline std::vector<TickerInfo> DistributedMarketDataClient::query(
      const TickerInfoQuery& query) {
    for(auto& c : m_market_data_clients) {
      if(auto client = std::get<1>(c)) {
        return client->query(query);
      }
    }
    return {};
  }

  inline TickerSnapshot DistributedMarketDataClient::load_snapshot(
      const Ticker& ticker) {
    if(auto client = m_market_data_clients.get(ticker)) {
      return client->load_snapshot(ticker);
    }
    return {};
  }

  inline PriceCandlestick
      DistributedMarketDataClient::load_session_candlestick(
        const Ticker& ticker) {
    if(auto client = m_market_data_clients.get(ticker)) {
      return client->load_session_candlestick(ticker);
    }
    return {};
  }

  inline std::vector<TickerInfo>
      DistributedMarketDataClient::load_ticker_info_from_prefix(
        const std::string& prefix) {
    auto ticker_infos = std::vector<TickerInfo>();
    auto clients = std::unordered_set<std::shared_ptr<MarketDataClient>>();
    for(auto& c : m_market_data_clients) {
      if(auto client = std::get<1>(c)) {
        clients.insert(client);
      }
    }
    for(auto& client : clients) {
      auto result = client->load_ticker_info_from_prefix(prefix);
      ticker_infos.insert(ticker_infos.end(),
        std::make_move_iterator(result.begin()),
        std::make_move_iterator(result.end()));
    }
    return ticker_infos;
  }

  inline void DistributedMarketDataClient::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    for(auto& client : m_market_data_clients) {
      std::get<1>(*m_market_data_clients.begin()) = nullptr;
    }
    m_open_state.close();
  }
}

#endif
