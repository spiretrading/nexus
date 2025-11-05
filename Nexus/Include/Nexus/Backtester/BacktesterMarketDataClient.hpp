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

  inline void BacktesterMarketDataClient::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    m_service->query_bbo_quotes(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    m_service->query_bbo_quotes(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    m_service->query_book_quotes(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    m_service->query_book_quotes(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    m_service->query_time_and_sales(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::query(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    m_service->query_time_and_sales(query);
    m_market_data_client.query(query, std::move(queue));
  }

  inline std::vector<SecurityInfo> BacktesterMarketDataClient::query(
      const SecurityInfoQuery& query) {
    return m_market_data_client.query(query);
  }

  inline SecuritySnapshot BacktesterMarketDataClient::load_snapshot(
      const Security& security) {
    return m_market_data_client.load_snapshot(security);
  }

  inline SecurityTechnicals BacktesterMarketDataClient::load_technicals(
      const Security& security) {
    return m_market_data_client.load_technicals(security);
  }

  inline std::vector<SecurityInfo> BacktesterMarketDataClient::
      load_security_info_from_prefix(const std::string& prefix) {
    return m_market_data_client.load_security_info_from_prefix(prefix);
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
