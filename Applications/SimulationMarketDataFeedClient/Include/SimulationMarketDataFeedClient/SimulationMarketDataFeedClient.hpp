#ifndef NEXUS_SIMULATION_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_SIMULATION_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/Timer.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus {

  /**
   * Feeds simulated market data to the MarketDataFeedServer.
   * @param <F> The type of MarketDataFeedClient connected to the
   *        MarketDataFeedServer.
   * @param <T> The type of TimeClient used for timestamps.
   * @param <B> Controls the frequency of BboQuote updates.
   * @param <M> Controls the frequency of MarketQuote updates.
   * @param <S> Controls the frequency of TimeAndSale prints.
   */
  template<typename F, typename T, typename B, typename M, typename S>
  class SimulationMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient connected to the
       * MarketDataFeedServer.
       */
      using MarketDataFeedClient = Beam::GetTryDereferenceType<F>;

      /** The type of TimeClient used for timestamps. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /** Controls the frequency of BboQuote updates. */
      using BboTimer = Beam::GetTryDereferenceType<B>;

      /** Controls the frequency of MarketQuote updates. */
      using MarketQuoteTimer = Beam::GetTryDereferenceType<M>;

      /** Controls the frequency of TimeAndSale prints. */
      using TimeAndSaleTimer = Beam::GetTryDereferenceType<S>;

      /**
       * Constructs a SimulationMarketDataFeedClient.
       * @param securities The list of Securities to simulate market data for.
       * @param marketDatabase The MarketDatabase to use.
       * @param marketDataClient The MarketDataClient used to query for
       *        SecurityInfo on each simulated security.
       * @param marketDataFeedClient Initializes the MarketDataFeedClient to
       *        send the simulated data through.
       * @param timeClient Initializes the TimeClient.
       * @param bboTimer Initializes the Timer used to update the BboQuotes.
       * @param marketQuoteTimer Initializes the Timer used to update the
       *        MarketQuotes.
       * @param timeAndSaleTimer Initializes the Timer used to update the
       *        TimeAndSales.
       */
      template<typename MarketDataClient, typename FF, typename TF, typename BF,
        typename MF, typename SF>
      SimulationMarketDataFeedClient(const std::vector<Security>& securities,
        const MarketDatabase& marketDatabase,
        MarketDataClient& marketDataClient, FF&& marketDataFeedClient,
        TF&& timeClient, BF&& bboTimer, MF&& marketQuoteTimer,
        SF&& timeAndSaleTimer);

      ~SimulationMarketDataFeedClient();

      void Close();

    private:
      std::vector<MarketDataService::SecuritySnapshot> m_securities;
      MarketDatabase m_marketDatabase;
      Beam::GetOptionalLocalPtr<F> m_feedClient;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      Beam::GetOptionalLocalPtr<B> m_bboTimer;
      Beam::GetOptionalLocalPtr<M> m_marketQuoteTimer;
      Beam::GetOptionalLocalPtr<S> m_timeAndSaleTimer;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void OnBboTimerExpired(Beam::Threading::Timer::Result result);
      void OnMarketQuoteTimerExpired(Beam::Threading::Timer::Result result);
      void OnTimeAndSaleTimerExpired(Beam::Threading::Timer::Result result);
  };

  template<typename F, typename T, typename B, typename M, typename S>
  template<typename MarketDataClient, typename FF, typename TF, typename BF,
    typename MF, typename SF>
  SimulationMarketDataFeedClient<F, T, B, M, S>::SimulationMarketDataFeedClient(
      const std::vector<Security>& securities,
      const MarketDatabase& marketDatabase, MarketDataClient& marketDataClient,
      FF&& marketDataFeedClient, TF&& timeClient, BF&& bboTimer,
      MF&& marketQuoteTimer, SF&& timeAndSaleTimer)
      : m_marketDatabase(marketDatabase),
        m_feedClient(std::forward<FF>(marketDataFeedClient)),
        m_timeClient(std::forward<TF>(timeClient)),
        m_bboTimer(std::forward<BF>(bboTimer)),
        m_marketQuoteTimer(std::forward<MF>(marketQuoteTimer)),
        m_timeAndSaleTimer(std::forward<SF>(timeAndSaleTimer)) {
    for(auto& security : securities) {
      auto info = LoadSecurityInfo(security, marketDataClient);
      if(!info) {
        m_feedClient->Add(SecurityInfo(security, ToString(security), "", 100));
      }
      m_securities.emplace_back();
      auto& snapshot = m_securities.back();
      snapshot.m_security = security;
    }
    try {
      for(auto& snapshot : m_securities) {
        snapshot.m_bboQuote->m_bid.m_side = Side::BID;
        snapshot.m_bboQuote->m_bid.m_price =
          ((std::rand() % 100) + 1) * Money::ONE;
        snapshot.m_bboQuote->m_ask.m_side = Side::ASK;
        snapshot.m_bboQuote->m_ask.m_price =
          snapshot.m_bboQuote->m_bid.m_price + Money::CENT;
        snapshot.m_bboQuote->m_timestamp = m_timeClient->GetTime();
        auto markets = m_marketDatabase.FromCountry(
          snapshot.m_security.GetCountry());
        for(auto& market : markets) {
          auto quote =
            MarketQuote(market.m_code, Quote(Money::CENT, 100, Side::BID),
              Quote(2 * Money::CENT, 100, Side::ASK), m_timeClient->GetTime());
          *snapshot.m_marketQuotes[market.m_code] = quote;
        }
      }
      m_bboTimer->GetPublisher().Monitor(
        m_tasks.GetSlot<Beam::Threading::Timer::Result>(
          std::bind(&SimulationMarketDataFeedClient::OnBboTimerExpired, this,
            std::placeholders::_1)));
      m_marketQuoteTimer->GetPublisher().Monitor(
        m_tasks.GetSlot<Beam::Threading::Timer::Result>(
          std::bind(&SimulationMarketDataFeedClient::OnMarketQuoteTimerExpired,
            this, std::placeholders::_1)));
      m_timeAndSaleTimer->GetPublisher().Monitor(
        m_tasks.GetSlot<Beam::Threading::Timer::Result>(
          std::bind(&SimulationMarketDataFeedClient::OnTimeAndSaleTimerExpired,
            this, std::placeholders::_1)));
      m_bboTimer->Start();
      m_marketQuoteTimer->Start();
      m_timeAndSaleTimer->Start();
    } catch(std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename F, typename T, typename B, typename M, typename S>
  SimulationMarketDataFeedClient<F, T, B, M, S>::
      ~SimulationMarketDataFeedClient() {
    Close();
  }

  template<typename F, typename T, typename B, typename M, typename S>
  void SimulationMarketDataFeedClient<F, T, B, M, S>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_feedClient->Close();
    m_bboTimer->Cancel();
    m_marketQuoteTimer->Cancel();
    m_timeAndSaleTimer->Cancel();
    m_tasks.Break();
    m_openState.Close();
  }

  template<typename F, typename T, typename B, typename M, typename S>
  void SimulationMarketDataFeedClient<F, T, B, M, S>::OnBboTimerExpired(
      Beam::Threading::Timer::Result result) {
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
      m_feedClient->Publish(
        SecurityBboQuote(snapshot.m_bboQuote, snapshot.m_security));
    }
    m_bboTimer->Start();
  }

  template<typename F, typename T, typename B, typename M, typename S>
  void SimulationMarketDataFeedClient<F, T, B, M, S>::OnMarketQuoteTimerExpired(
      Beam::Threading::Timer::Result result) {
    for(auto& snapshot : m_securities) {
      for(auto& marketQuote : snapshot.m_marketQuotes |
          boost::adaptors::map_values) {
        marketQuote->m_ask.m_price = snapshot.m_bboQuote->m_ask.m_price;
        marketQuote->m_bid.m_price = snapshot.m_bboQuote->m_bid.m_price;
        marketQuote->m_bid.m_size = 100 + (std::rand() % 1000);
        marketQuote->m_ask.m_size = 100 + (std::rand() % 1000);
        marketQuote->m_timestamp = m_timeClient->GetTime();
        m_feedClient->Publish(
          SecurityMarketQuote(marketQuote, snapshot.m_security));
      }
    }
    m_marketQuoteTimer->Start();
  }

  template<typename F, typename T, typename B, typename M, typename S>
  void SimulationMarketDataFeedClient<F, T, B, M, S>::OnTimeAndSaleTimerExpired(
      Beam::Threading::Timer::Result result) {
    for(auto& snapshot : m_securities) {
      auto condition =
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@");
      auto price = [&] {
        if(std::rand() % 2 == 0) {
          return snapshot.m_bboQuote->m_bid.m_price;
        }
        return snapshot.m_bboQuote->m_ask.m_price;
      }();
      auto timeAndSale = TimeAndSale(m_timeClient->GetTime(), price, 100,
        condition, snapshot.m_security.GetMarket().GetData(), "M1", "M2");
      m_feedClient->Publish(
        SecurityTimeAndSale(timeAndSale, snapshot.m_security));
    }
    m_timeAndSaleTimer->Start();
  }
}

#endif
