#ifndef NEXUS_BACKTESTER_MARKET_DATA_CLIENT_HPP
#define NEXUS_BACKTESTER_MARKET_DATA_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterMarketDataService.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"

namespace Nexus {

  /** Implements a MarketDataClient used for backtesting. */
  class BacktesterMarketDataClient : private boost::noncopyable {
    public:

      /**
       * Constructs a BacktesterMarketDataClient.
       * @param service The BacktesterMarketDataService to connect to.
       * @param marketDataClient The underlying MarketDataClient to submit
       *        queries to.
       */
      BacktesterMarketDataClient(Beam::Ref<BacktesterMarketDataService> service,
        std::unique_ptr<MarketDataService::VirtualMarketDataClient>
        marketDataClient);

      ~BacktesterMarketDataClient();

      void QueryOrderImbalances(
        const MarketDataService::MarketWideDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedOrderImbalance>>& queue);

      void QueryOrderImbalances(
        const MarketDataService::MarketWideDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue);

      void QueryBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue);

      void QueryBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue);

      void QueryBookQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedBookQuote>>& queue);

      void QueryBookQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue);

      void QueryMarketQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedMarketQuote>>& queue);

      void QueryMarketQuotes(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue);

      void QueryTimeAndSales(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<
        Beam::QueueWriter<SequencedTimeAndSale>>& queue);

      void QueryTimeAndSales(
        const MarketDataService::SecurityMarketDataQuery& query,
        const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue);

      MarketDataService::SecuritySnapshot LoadSecuritySnapshot(
        const Security& security);

      SecurityTechnicals LoadSecurityTechnicals(
        const Security& security);

      boost::optional<SecurityInfo> LoadSecurityInfo(const Security& security);

      std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix);

      void Open();

      void Close();

    private:
      BacktesterMarketDataService* m_service;
      std::unique_ptr<MarketDataService::VirtualMarketDataClient>
        m_marketDataClient;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline BacktesterMarketDataClient::BacktesterMarketDataClient(
    Beam::Ref<BacktesterMarketDataService> service,
    std::unique_ptr<MarketDataService::VirtualMarketDataClient>
    marketDataClient)
    : m_service{service.Get()},
      m_marketDataClient{std::move(marketDataClient)} {}

  inline BacktesterMarketDataClient::~BacktesterMarketDataClient() {
    Close();
  }

  inline void BacktesterMarketDataClient::QueryOrderImbalances(
      const MarketDataService::MarketWideDataQuery& query,
      const std::shared_ptr<
      Beam::QueueWriter<SequencedOrderImbalance>>& queue) {
    m_service->QueryOrderImbalances(query);
    m_marketDataClient->QueryOrderImbalances(query, queue);
  }

  inline void BacktesterMarketDataClient::QueryOrderImbalances(
      const MarketDataService::MarketWideDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) {
    m_service->QueryOrderImbalances(query);
    m_marketDataClient->QueryOrderImbalances(query, queue);
  }

  inline void BacktesterMarketDataClient::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue) {
    m_service->QueryBboQuotes(query);
    m_marketDataClient->QueryBboQuotes(query, queue);
  }

  inline void BacktesterMarketDataClient::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) {
    m_service->QueryBboQuotes(query);
    m_marketDataClient->QueryBboQuotes(query, queue);
  }

  inline void BacktesterMarketDataClient::QueryBookQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue) {
    m_service->QueryBookQuotes(query);
    m_marketDataClient->QueryBookQuotes(query, queue);
  }

  inline void BacktesterMarketDataClient::QueryBookQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) {
    m_service->QueryBookQuotes(query);
    m_marketDataClient->QueryBookQuotes(query, queue);
  }

  inline void BacktesterMarketDataClient::QueryMarketQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<
      Beam::QueueWriter<SequencedMarketQuote>>& queue) {
    m_service->QueryMarketQuotes(query);
    m_marketDataClient->QueryMarketQuotes(query, queue);
  }

  inline void BacktesterMarketDataClient::QueryMarketQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) {
    m_service->QueryMarketQuotes(query);
    m_marketDataClient->QueryMarketQuotes(query, queue);
  }

  inline void BacktesterMarketDataClient::QueryTimeAndSales(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue) {
    m_service->QueryTimeAndSales(query);
    m_marketDataClient->QueryTimeAndSales(query, queue);
  }

  inline void BacktesterMarketDataClient::QueryTimeAndSales(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) {
    m_service->QueryTimeAndSales(query);
    m_marketDataClient->QueryTimeAndSales(query, queue);
  }

  inline MarketDataService::SecuritySnapshot
      BacktesterMarketDataClient::LoadSecuritySnapshot(
      const Security& security) {
    return m_marketDataClient->LoadSecuritySnapshot(security);
  }

  inline SecurityTechnicals BacktesterMarketDataClient::LoadSecurityTechnicals(
      const Security& security) {
    return m_marketDataClient->LoadSecurityTechnicals(security);
  }

  inline boost::optional<SecurityInfo>
      BacktesterMarketDataClient::LoadSecurityInfo(const Security& security) {
    return m_marketDataClient->LoadSecurityInfo(security);
  }

  inline std::vector<SecurityInfo> BacktesterMarketDataClient::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    return m_marketDataClient->LoadSecurityInfoFromPrefix(prefix);
  }

  inline void BacktesterMarketDataClient::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataClient->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void BacktesterMarketDataClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void BacktesterMarketDataClient::Shutdown() {
    m_marketDataClient->Close();
    m_openState.SetClosed();
  }
}

#endif
