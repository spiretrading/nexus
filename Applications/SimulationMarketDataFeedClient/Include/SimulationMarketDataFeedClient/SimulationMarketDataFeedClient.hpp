#ifndef NEXUS_SIMULATION_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_SIMULATION_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/Timer.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus {

  /*! \class SimulationMarketDataFeedClient
      \brief Feeds simulated market data to the MarketDataFeedServer.
      \tparam MarketDataFeedClientType The type of MarketDataFeedClient
              connected to the MarketDataFeedServer.
      \tparam TimeClientType The type of TimeClient used for timestamps.
      \tparam BboTimerType Controls the frequency of BboQuote updates.
      \tparam MarketQuoteTimerType Controls the frequency of MarketQuote
              updates.
      \tparam TimeAndSaleTimerType Controls the frequency of TimeAndSale prints.
   */
  template<typename MarketDataFeedClientType, typename TimeClientType,
    typename BboTimerType, typename MarketQuoteTimerType,
    typename TimeAndSaleTimerType>
  class SimulationMarketDataFeedClient : private boost::noncopyable {
    public:

      //! The type of MarketDataFeedClient connected to the
      //! MarketDataFeedServer.
      using MarketDataFeedClient = Beam::GetTryDereferenceType<
        MarketDataFeedClientType>;

      //! The type of TimeClient used for timestamps.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! Controls the frequency of BboQuote updates.
      using BboTimer = Beam::GetTryDereferenceType<BboTimerType>;

      //! Controls the frequency of MarketQuote updates.
      using MarketQuoteTimer = Beam::GetTryDereferenceType<
        MarketQuoteTimerType>;

      //! Controls the frequency of TimeAndSale prints.
      using TimeAndSaleTimer = Beam::GetTryDereferenceType<
        TimeAndSaleTimerType>;

      //! Constructs a SimulationMarketDataFeedClient.
      /*!
        \param securities The list of Securities to simulate market data for.
        \param marketDatabase The MarketDatabase to use.
        \param marketDataFeedClient Initializes the MarketDataFeedClient to send
               the simulated data through.
        \param timeClient Initializes the TimeClient.
        \param bboTimer Initializes the Timer used to update the BboQuotes.
        \param marketQuoteTimer Initializes the Timer used to update the
               MarketQuotes.
        \param timeAndSaleTimer Initializes the Timer used to update the
               TimeAndSales.
      */
      template<typename MarketDataFeedClientForward, typename TimeClientForward,
        typename BboTimerForward, typename MarketQuoteTimerForward,
        typename TimeAndSaleTimerForward>
      SimulationMarketDataFeedClient(const std::vector<Security>& securities,
        const MarketDatabase& marketDatabase,
        MarketDataFeedClientForward&& marketDataFeedClient,
        TimeClientForward&& timeClient, BboTimerForward&& bboTimer,
        MarketQuoteTimerForward&& marketQuoteTimer,
        TimeAndSaleTimerForward&& timeAndSaleTimer);

      ~SimulationMarketDataFeedClient();

      void Open();

      void Close();

    private:
      std::vector<MarketDataService::SecuritySnapshot> m_securities;
      MarketDatabase m_marketDatabase;
      Beam::GetOptionalLocalPtr<MarketDataFeedClientType> m_feedClient;
      Beam::GetOptionalLocalPtr<TimeClientType> m_timeClient;
      Beam::GetOptionalLocalPtr<BboTimerType> m_bboTimer;
      Beam::GetOptionalLocalPtr<MarketQuoteTimerType> m_marketQuoteTimer;
      Beam::GetOptionalLocalPtr<TimeAndSaleTimerType> m_timeAndSaleTimer;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_callbacks;

      void Shutdown();
      void OnBboTimerExpired(const Beam::Threading::Timer::Result& result);
      void OnMarketQuoteTimerExpired(
        const Beam::Threading::Timer::Result& result);
      void OnTimeAndSaleTimerExpired(
        const Beam::Threading::Timer::Result& result);
  };

  template<typename MarketDataFeedClientType, typename TimeClientType,
    typename BboTimerType, typename MarketQuoteTimerType,
    typename TimeAndSaleTimerType>
  template<typename MarketDataFeedClientForward, typename TimeClientForward,
    typename BboTimerForward, typename MarketQuoteTimerForward,
    typename TimeAndSaleTimerForward>
  SimulationMarketDataFeedClient<MarketDataFeedClientType, TimeClientType,
      BboTimerType, MarketQuoteTimerType, TimeAndSaleTimerType>::
      SimulationMarketDataFeedClient(const std::vector<Security>& securities,
        const MarketDatabase& marketDatabase,
        MarketDataFeedClientForward&& marketDataFeedClient,
        TimeClientForward&& timeClient, BboTimerForward&& bboTimer,
        MarketQuoteTimerForward&& marketQuoteTimer,
        TimeAndSaleTimerForward&& timeAndSaleTimer)
      : m_marketDatabase(marketDatabase),
        m_feedClient(std::forward<MarketDataFeedClientForward>(
          marketDataFeedClient)),
        m_timeClient(std::forward<TimeClientForward>(timeClient)),
        m_bboTimer(std::forward<BboTimerForward>(bboTimer)),
        m_marketQuoteTimer(std::forward<MarketQuoteTimerForward>(
          marketQuoteTimer)),
        m_timeAndSaleTimer(std::forward<TimeAndSaleTimerForward>(
          timeAndSaleTimer)) {
    for(auto& security : securities) {
      m_securities.emplace_back();
      auto& snapshot = m_securities.back();
      snapshot.m_security = security;
    }
  }

  template<typename MarketDataFeedClientType, typename TimeClientType,
    typename BboTimerType, typename MarketQuoteTimerType,
    typename TimeAndSaleTimerType>
  SimulationMarketDataFeedClient<MarketDataFeedClientType, TimeClientType,
      BboTimerType, MarketQuoteTimerType, TimeAndSaleTimerType>::
      ~SimulationMarketDataFeedClient() {
    Close();
  }

  template<typename MarketDataFeedClientType, typename TimeClientType,
    typename BboTimerType, typename MarketQuoteTimerType,
    typename TimeAndSaleTimerType>
  void SimulationMarketDataFeedClient<MarketDataFeedClientType, TimeClientType,
      BboTimerType, MarketQuoteTimerType, TimeAndSaleTimerType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_feedClient->Open();
      m_timeClient->Open();
      for(auto& snapshot : m_securities) {
        snapshot.m_bboQuote->m_bid.m_side = Side::BID;
        snapshot.m_bboQuote->m_bid.m_price = ((std::rand() % 100) + 1) *
          Money::ONE;
        snapshot.m_bboQuote->m_ask.m_side = Side::ASK;
        snapshot.m_bboQuote->m_ask.m_price =
          snapshot.m_bboQuote->m_bid.m_price + Money::CENT;
        snapshot.m_bboQuote->m_timestamp = m_timeClient->GetTime();
        auto markets = m_marketDatabase.FromCountry(
          snapshot.m_security.GetCountry());
        for(const auto& market : markets) {
          MarketQuote quote(market.m_code, Quote(Money::CENT, 100, Side::BID),
            Quote(2 * Money::CENT, 100, Side::ASK), m_timeClient->GetTime());
          *snapshot.m_marketQuotes[market.m_code] = quote;
        }
      }
      m_bboTimer->GetPublisher().Monitor(
        m_callbacks.GetSlot<Beam::Threading::Timer::Result>(
        std::bind(&SimulationMarketDataFeedClient::OnBboTimerExpired, this,
        std::placeholders::_1)));
      m_marketQuoteTimer->GetPublisher().Monitor(
        m_callbacks.GetSlot<Beam::Threading::Timer::Result>(
        std::bind(&SimulationMarketDataFeedClient::OnMarketQuoteTimerExpired,
        this, std::placeholders::_1)));
      m_timeAndSaleTimer->GetPublisher().Monitor(
        m_callbacks.GetSlot<Beam::Threading::Timer::Result>(
        std::bind(&SimulationMarketDataFeedClient::OnTimeAndSaleTimerExpired,
        this, std::placeholders::_1)));
      m_bboTimer->Start();
      m_marketQuoteTimer->Start();
      m_timeAndSaleTimer->Start();
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename MarketDataFeedClientType, typename TimeClientType,
    typename BboTimerType, typename MarketQuoteTimerType,
    typename TimeAndSaleTimerType>
  void SimulationMarketDataFeedClient<MarketDataFeedClientType, TimeClientType,
      BboTimerType, MarketQuoteTimerType, TimeAndSaleTimerType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename MarketDataFeedClientType, typename TimeClientType,
    typename BboTimerType, typename MarketQuoteTimerType,
    typename TimeAndSaleTimerType>
  void SimulationMarketDataFeedClient<MarketDataFeedClientType, TimeClientType,
      BboTimerType, MarketQuoteTimerType, TimeAndSaleTimerType>::Shutdown() {
    m_feedClient->Close();
    m_bboTimer->Cancel();
    m_marketQuoteTimer->Cancel();
    m_timeAndSaleTimer->Cancel();
    m_callbacks.Break();
    m_openState.SetClosed();
  }

  template<typename MarketDataFeedClientType, typename TimeClientType,
    typename BboTimerType, typename MarketQuoteTimerType,
    typename TimeAndSaleTimerType>
  void SimulationMarketDataFeedClient<MarketDataFeedClientType, TimeClientType,
      BboTimerType, MarketQuoteTimerType, TimeAndSaleTimerType>::
      OnBboTimerExpired(const Beam::Threading::Timer::Result& result) {
    for(auto& snapshot : m_securities) {
      snapshot.m_bboQuote->m_bid.m_size = 100 + (std::rand() % 1000);
      snapshot.m_bboQuote->m_ask.m_size = 100 + (std::rand() % 1000);
      snapshot.m_bboQuote->m_timestamp = m_timeClient->GetTime();
      auto direction = std::rand() % 2;
      if(direction == 1) {
        snapshot.m_bboQuote->m_bid.m_price += Money::CENT;
        snapshot.m_bboQuote->m_ask.m_price += Money::CENT;
      } else if(snapshot.m_bboQuote->m_bid.m_price > Money::CENT) {
        snapshot.m_bboQuote->m_bid.m_price -= Money::CENT;
        snapshot.m_bboQuote->m_ask.m_price -= Money::CENT;
      }
      m_feedClient->PublishBboQuote(SecurityBboQuote(snapshot.m_bboQuote,
        snapshot.m_security));
    }
    m_bboTimer->Start();
  }

  template<typename MarketDataFeedClientType, typename TimeClientType,
    typename BboTimerType, typename MarketQuoteTimerType,
    typename TimeAndSaleTimerType>
  void SimulationMarketDataFeedClient<MarketDataFeedClientType, TimeClientType,
      BboTimerType, MarketQuoteTimerType, TimeAndSaleTimerType>::
      OnMarketQuoteTimerExpired(const Beam::Threading::Timer::Result& result) {
    for(auto& snapshot : m_securities) {
      for(auto& marketQuote : snapshot.m_marketQuotes |
          boost::adaptors::map_values) {
        marketQuote->m_ask.m_price = snapshot.m_bboQuote->m_ask.m_price;
        marketQuote->m_bid.m_price = snapshot.m_bboQuote->m_bid.m_price;
        marketQuote->m_bid.m_size = 100 + (std::rand() % 1000);
        marketQuote->m_ask.m_size = 100 + (std::rand() % 1000);
        marketQuote->m_timestamp = m_timeClient->GetTime();
        m_feedClient->PublishMarketQuote(
          SecurityMarketQuote(marketQuote, snapshot.m_security));
      }
    }
    m_marketQuoteTimer->Start();
  }

  template<typename MarketDataFeedClientType, typename TimeClientType,
    typename BboTimerType, typename MarketQuoteTimerType,
    typename TimeAndSaleTimerType>
  void SimulationMarketDataFeedClient<MarketDataFeedClientType, TimeClientType,
      BboTimerType, MarketQuoteTimerType, TimeAndSaleTimerType>::
      OnTimeAndSaleTimerExpired(const Beam::Threading::Timer::Result& result) {
    for(auto& snapshot : m_securities) {
      TimeAndSale::Condition condition;
      condition.m_code = "@";
      condition.m_type = TimeAndSale::Condition::Type::REGULAR;
      Money price;
      if(std::rand() % 2 == 0) {
        price = snapshot.m_bboQuote->m_bid.m_price;
      } else {
        price = snapshot.m_bboQuote->m_ask.m_price;
      }
      std::string marketCode;
      if(snapshot.m_security.GetMarket() == DefaultMarkets::TSX()) {
        marketCode = "TSE";
      } else if(snapshot.m_security.GetMarket() == DefaultMarkets::TSXV()) {
        marketCode = "CDX";
      } else if(snapshot.m_security.GetMarket() == DefaultMarkets::ASX()) {
        marketCode = "ASX";
      } else {
        marketCode = "?";
      }
      TimeAndSale timeAndSale(m_timeClient->GetTime(), price, 100, condition,
        marketCode);
      m_feedClient->PublishTimeAndSale(
        SecurityTimeAndSale(timeAndSale, snapshot.m_security));
    }
    m_timeAndSaleTimer->Start();
  }
}

#endif
