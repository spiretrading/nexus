#ifndef NEXUS_BACKTESTERMARKETDATACLIENT_HPP
#define NEXUS_BACKTESTERMARKETDATACLIENT_HPP
#include <atomic>
#include <Beam/IO/OpenState.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"

namespace Nexus {

  /*! \class BacktesterMarketDataClient
      \brief Implements a MarketDataClient used for backtesting.
   */
  class BacktesterMarketDataClient : private boost::noncopyable {
    public:
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

      std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix);

      void Open();

      void Close();

    private:
      std::unique_ptr<MarketDataService::VirtualMarketDataClient>
        m_marketDataClient;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  BacktesterMarketDataClient::~BacktesterMarketDataClient() {
    Close();
  }

  void BacktesterMarketDataClient::QueryOrderImbalances(
      const MarketDataService::MarketWideDataQuery& query,
      const std::shared_ptr<
      Beam::QueueWriter<SequencedOrderImbalance>>& queue) {}

  void BacktesterMarketDataClient::QueryOrderImbalances(
      const MarketDataService::MarketWideDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<OrderImbalance>>& queue) {}

  void BacktesterMarketDataClient::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBboQuote>>& queue) {}

  void BacktesterMarketDataClient::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BboQuote>>& queue) {}

  void BacktesterMarketDataClient::QueryBookQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedBookQuote>>& queue) {}

  void BacktesterMarketDataClient::QueryBookQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<BookQuote>>& queue) {}

  void BacktesterMarketDataClient::QueryMarketQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<
      Beam::QueueWriter<SequencedMarketQuote>>& queue) {}

  void BacktesterMarketDataClient::QueryMarketQuotes(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<MarketQuote>>& queue) {}

  void BacktesterMarketDataClient::QueryTimeAndSales(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<SequencedTimeAndSale>>& queue) {}

  void BacktesterMarketDataClient::QueryTimeAndSales(
      const MarketDataService::SecurityMarketDataQuery& query,
      const std::shared_ptr<Beam::QueueWriter<TimeAndSale>>& queue) {}

  MarketDataService::SecuritySnapshot
      BacktesterMarketDataClient::LoadSecuritySnapshot(
      const Security& security) {
    return m_marketDataClient->LoadSecuritySnapshot(security);
  }

  SecurityTechnicals BacktesterMarketDataClient::LoadSecurityTechnicals(
      const Security& security) {
    return m_marketDataClient->LoadSecurityTechnicals(security);
  }

  std::vector<SecurityInfo> BacktesterMarketDataClient::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    return m_marketDataClient->LoadSecurityInfoFromPrefix(prefix);
  }

  void BacktesterMarketDataClient::Open() {
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

  void BacktesterMarketDataClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  void BacktesterMarketDataClient::Shutdown() {
    m_marketDataClient->Close();
    m_openState.SetClosed();
  }
}

#endif
