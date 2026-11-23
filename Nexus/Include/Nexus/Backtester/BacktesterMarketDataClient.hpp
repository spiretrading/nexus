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
      void query(const VenueQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);
      void query(
        const VenueQuery& query, Beam::ScopedQueueWriter<OrderImbalance> queue);
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
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<SequencedTickerStatus> queue);
      void query(
        const TickerQuery& query, Beam::ScopedQueueWriter<TickerStatus> queue);
      std::vector<TickerInfo> query(const TickerInfoQuery& query);
      TickerSnapshot load_snapshot(const Ticker& ticker);
      SequencedSessionTechnicals load_session_technicals(const Ticker& ticker);
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
      template<typename Q, typename T>
      void submit(const Q& query, Beam::ScopedQueueWriter<T> queue,
        void (BacktesterMarketDataService::*start_replay)(const Q&));
  };

  inline BacktesterMarketDataClient::BacktesterMarketDataClient(
    Beam::Ref<BacktesterMarketDataService> service,
    MarketDataClient market_data_client)
    : m_service(service.get()),
      m_market_data_client(std::move(market_data_client)) {}

  inline BacktesterMarketDataClient::~BacktesterMarketDataClient() {
    close();
  }

  inline void BacktesterMarketDataClient::query(const VenueQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    submit(query, std::move(queue),
      &BacktesterMarketDataService::query_order_imbalances);
  }

  inline void BacktesterMarketDataClient::query(
      const VenueQuery& query, Beam::ScopedQueueWriter<OrderImbalance> queue) {
    submit(query, std::move(queue),
      &BacktesterMarketDataService::query_order_imbalances);
  }

  inline void BacktesterMarketDataClient::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    submit(query, std::move(queue),
      &BacktesterMarketDataService::query_bbo_quotes);
  }

  inline void BacktesterMarketDataClient::query(
      const TickerQuery& query, Beam::ScopedQueueWriter<BboQuote> queue) {
    submit(query, std::move(queue),
      &BacktesterMarketDataService::query_bbo_quotes);
  }

  inline void BacktesterMarketDataClient::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    submit(query, std::move(queue),
      &BacktesterMarketDataService::query_book_quotes);
  }

  inline void BacktesterMarketDataClient::query(
      const TickerQuery& query, Beam::ScopedQueueWriter<BookQuote> queue) {
    submit(query, std::move(queue),
      &BacktesterMarketDataService::query_book_quotes);
  }

  inline void BacktesterMarketDataClient::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    submit(query, std::move(queue),
      &BacktesterMarketDataService::query_time_and_sales);
  }

  inline void BacktesterMarketDataClient::query(
      const TickerQuery& query, Beam::ScopedQueueWriter<TimeAndSale> queue) {
    submit(query, std::move(queue),
      &BacktesterMarketDataService::query_time_and_sales);
  }

  inline void BacktesterMarketDataClient::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedTickerStatus> queue) {
    submit(query, std::move(queue),
      &BacktesterMarketDataService::query_ticker_statuses);
  }

  inline void BacktesterMarketDataClient::query(
      const TickerQuery& query, Beam::ScopedQueueWriter<TickerStatus> queue) {
    submit(query, std::move(queue),
      &BacktesterMarketDataService::query_ticker_statuses);
  }

  inline std::vector<TickerInfo> BacktesterMarketDataClient::query(
      const TickerInfoQuery& query) {
    return m_market_data_client.query(query);
  }

  inline TickerSnapshot BacktesterMarketDataClient::load_snapshot(
      const Ticker& ticker) {
    return m_market_data_client.load_snapshot(ticker);
  }

  inline SequencedSessionTechnicals
      BacktesterMarketDataClient::load_session_technicals(
        const Ticker& ticker) {
    return m_market_data_client.load_session_technicals(ticker);
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

  template<typename Q, typename T>
  void BacktesterMarketDataClient::submit(const Q& query,
      Beam::ScopedQueueWriter<T> queue,
      void (BacktesterMarketDataService::*start_replay)(const Q&)) {
    m_market_data_client.query(query, std::move(queue));
    (m_service->*start_replay)(query);
  }
}

#endif
