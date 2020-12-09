#ifndef NEXUS_BACKTESTER_MARKET_DATA_CLIENT_HPP
#define NEXUS_BACKTESTER_MARKET_DATA_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterMarketDataService.hpp"
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"

namespace Nexus {

  /** Implements a MarketDataClient used for backtesting. */
  class BacktesterMarketDataClient {
    public:

      /**
       * Constructs a BacktesterMarketDataClient.
       * @param service The BacktesterMarketDataService to connect to.
       * @param marketDataClient The underlying MarketDataClient to submit
       *        queries to.
       */
      BacktesterMarketDataClient(Beam::Ref<BacktesterMarketDataService> service,
        MarketDataService::MarketDataClientBox marketDataClient);

      ~BacktesterMarketDataClient();

      void QueryOrderImbalances(
        const MarketDataService::MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);

      void QueryOrderImbalances(
        const MarketDataService::MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);

      void QueryBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);

      void QueryBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);

      void QueryBookQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);

      void QueryBookQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);

      void QueryMarketQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedMarketQuote> queue);

      void QueryMarketQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<MarketQuote> queue);

      void QueryTimeAndSales(
        const MarketDataService::SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);

      void QueryTimeAndSales(
        const MarketDataService::SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);

      MarketDataService::SecuritySnapshot LoadSecuritySnapshot(
        const Security& security);

      SecurityTechnicals LoadSecurityTechnicals(
        const Security& security);

      boost::optional<SecurityInfo> LoadSecurityInfo(const Security& security);

      std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix);

      void Close();

    private:
      BacktesterMarketDataService* m_service;
      MarketDataService::MarketDataClientBox m_marketDataClient;
      Beam::IO::OpenState m_openState;

      BacktesterMarketDataClient(const BacktesterMarketDataClient&) = delete;
      BacktesterMarketDataClient& operator =(
        const BacktesterMarketDataClient&) = delete;
  };

  inline BacktesterMarketDataClient::BacktesterMarketDataClient(
    Beam::Ref<BacktesterMarketDataService> service,
    MarketDataService::MarketDataClientBox marketDataClient)
    : m_service(service.Get()),
      m_marketDataClient(std::move(marketDataClient)) {}

  inline BacktesterMarketDataClient::~BacktesterMarketDataClient() {
    Close();
  }

  inline void BacktesterMarketDataClient::QueryOrderImbalances(
      const MarketDataService::MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    m_service->QueryOrderImbalances(query);
    m_marketDataClient.QueryOrderImbalances(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::QueryOrderImbalances(
      const MarketDataService::MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    m_service->QueryOrderImbalances(query);
    m_marketDataClient.QueryOrderImbalances(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    m_service->QueryBboQuotes(query);
    m_marketDataClient.QueryBboQuotes(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    m_service->QueryBboQuotes(query);
    m_marketDataClient.QueryBboQuotes(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::QueryBookQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    m_service->QueryBookQuotes(query);
    m_marketDataClient.QueryBookQuotes(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::QueryBookQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    m_service->QueryBookQuotes(query);
    m_marketDataClient.QueryBookQuotes(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::QueryMarketQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedMarketQuote> queue) {
    m_service->QueryMarketQuotes(query);
    m_marketDataClient.QueryMarketQuotes(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::QueryMarketQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<MarketQuote> queue) {
    m_service->QueryMarketQuotes(query);
    m_marketDataClient.QueryMarketQuotes(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::QueryTimeAndSales(
      const MarketDataService::SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    m_service->QueryTimeAndSales(query);
    m_marketDataClient.QueryTimeAndSales(query, std::move(queue));
  }

  inline void BacktesterMarketDataClient::QueryTimeAndSales(
      const MarketDataService::SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    m_service->QueryTimeAndSales(query);
    m_marketDataClient.QueryTimeAndSales(query, std::move(queue));
  }

  inline MarketDataService::SecuritySnapshot
      BacktesterMarketDataClient::LoadSecuritySnapshot(
      const Security& security) {
    return m_marketDataClient.LoadSecuritySnapshot(security);
  }

  inline SecurityTechnicals BacktesterMarketDataClient::LoadSecurityTechnicals(
      const Security& security) {
    return m_marketDataClient.LoadSecurityTechnicals(security);
  }

  inline boost::optional<SecurityInfo>
      BacktesterMarketDataClient::LoadSecurityInfo(const Security& security) {
    return m_marketDataClient.LoadSecurityInfo(security);
  }

  inline std::vector<SecurityInfo> BacktesterMarketDataClient::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    return m_marketDataClient.LoadSecurityInfoFromPrefix(prefix);
  }

  inline void BacktesterMarketDataClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_marketDataClient.Close();
    m_openState.Close();
  }
}

#endif
