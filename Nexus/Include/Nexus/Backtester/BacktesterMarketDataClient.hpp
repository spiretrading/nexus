#ifndef NEXUS_BACKTESTER_MARKET_DATA_CLIENT_HPP
#define NEXUS_BACKTESTER_MARKET_DATA_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include "Nexus/Backtester/BacktesterMarketDataService.hpp"

namespace Nexus {

  /** Implements a MarketDataClient used for backtesting. */
  class BacktesterMarketDataClient {
    public:

      /**
       * Constructs a BacktesterMarketDataClient.
       * @param service The BacktesterMarketDataService to connect to.
       * @param market_data_client The underlying MarketDataClient to submit
       *        queries to.
       */
      BacktesterMarketDataClient(Beam::Ref<BacktesterMarketDataService> service,
        MarketDataClient market_data_client);

      ~BacktesterMarketDataClient();
      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);
      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);
      void query(
        const TickerQuery& query, Beam::ScopedQueueWriter<BboQuote> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);
      void query(
        const TickerQuery& query, Beam::ScopedQueueWriter<BookQuote> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);
      void query(
        const TickerQuery& query, Beam::ScopedQueueWriter<TimeAndSale> queue);
      std::vector<TickerInfo> query(const TickerInfoQuery& query);
      TickerSnapshot load_snapshot(const Ticker& ticker);
      PriceCandlestick load_session_candlestick(const Ticker& ticker);
      std::vector<TickerInfo> load_ticker_info_from_prefix(
        const std::string& prefix);
      void close();

    private:
      BacktesterMarketDataService* m_service;
      MarketDataClient m_market_data_client;
      Beam::OpenState m_open_state;

      BacktesterMarketDataClient(const BacktesterMarketDataClient&) = delete;
      BacktesterMarketDataClient& operator =(
        const BacktesterMarketDataClient&) = delete;
  };

  inline BacktesterMarketDataClient::BacktesterMarketDataClient(
    Beam::Ref<BacktesterMarketDataService> service,
    MarketDataClient market_data_client)
    : m_service(service.get()),
      m_market_data_client(std::move(market_data_client)) {}

  inline BacktesterMarketDataClient::~BacktesterMarketDataClient() {
    close();
  }

  inline void BacktesterMarketDataClient::query(
      const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    m_service->query_order_imbalances(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::query(
      const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    m_service->query_order_imbalances(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    m_service->query_bbo_quotes(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::query(
      const TickerQuery& query, Beam::ScopedQueueWriter<BboQuote> queue) {
    m_service->query_bbo_quotes(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    m_service->query_book_quotes(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::query(
      const TickerQuery& query, Beam::ScopedQueueWriter<BookQuote> queue) {
    m_service->query_book_quotes(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    m_service->query_time_and_sales(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::query(
      const TickerQuery& query, Beam::ScopedQueueWriter<TimeAndSale> queue) {
    m_service->query_time_and_sales(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline std::vector<TickerInfo> BacktesterMarketDataClient::query(
      const TickerInfoQuery& query) {
    return m_market_data_client.query(query);
  }

  inline TickerSnapshot BacktesterMarketDataClient::load_snapshot(
      const Ticker& ticker) {
    return m_market_data_client.load_snapshot(ticker);
  }

  inline PriceCandlestick BacktesterMarketDataClient::load_session_candlestick(
      const Ticker& ticker) {
    return m_market_data_client.load_session_candlestick(ticker);
  }

  inline std::vector<TickerInfo> BacktesterMarketDataClient::
      load_ticker_info_from_prefix(const std::string& prefix) {
    return m_market_data_client.load_ticker_info_from_prefix(prefix);
  }

  inline void BacktesterMarketDataClient::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_market_data_client.close();
    m_open_state.close();
  }
}

#endif
