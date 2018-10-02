#ifndef NEXUS_REPLAY_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_REPLAY_MARKET_DATA_FEED_CLIENT_HPP
#include <functional>
#include <memory>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Threading/Timer.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"

namespace Nexus {

  /** Sends historical market data from a data store to a market data server.
      \tparam M The type of MarketDataFeedClient connected to the
              MarketDataFeedServer.
      \tparam D The type of HistoricalDataStore to load market data from.
      \tparam T The type of TimeClient to use.
      \tparam R The type of Timer to use.
   */
  template<typename M, typename D, typename T, typename R>
  class ReplayMarketDataFeedClient : private boost::noncopyable {
    public:

      //! The type of MarketDataFeedClient connected to the
      //! MarketDataFeedServer.
      using MarketDataFeedClient = Beam::GetTryDereferenceType<M>;

      //! The type of HistoricalDataStore to load market data from.
      using HistoricalDataStore = Beam::GetTryDereferenceType<D>;

      //! The type of TimeClient to use.
      using TimeClient = Beam::GetTryDereferenceType<T>;

      //! The type of Timer to use.
      using Timer = R;

      //! The builder used to build Timer instances.
      using TimerBuilder = std::function<
        std::unique_ptr<Timer> (boost::posix_time::time_duration d)>;

      //! Constructs a ReplayMarketDataFeedClient.
      /*!
        \param securities The list of Securities to replay.
        \param startTime The timestamp to begin loading data to replay.
        \param feedClient Initializes the MarketDataFeedClient to send the
               replayed data to.
        \param dataStore The HistoricalDataStore to load market data from.
        \param timeClient Initializes the TimeClient.
        \param timerBuilder The builder used to build Timer instances.
      */
      template<typename MF, typename DF, typename TF>
      ReplayMarketDataFeedClient(std::vector<Security> securities,
        boost::posix_time::ptime startTime, MF&& feedClient, DF&& dataStore,
        TF&& timeClient, TimerBuilder timerBuilder);

      ~ReplayMarketDataFeedClient();

      void Open();

      void Close();

    private:
      std::vector<Security> m_securities;
      boost::posix_time::ptime m_startTime;
      Beam::GetOptionalLocalPtr<M> m_feedClient;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      TimerBuilder m_timerBuilder;
      Beam::IO::OpenState m_openState;
      Beam::Routines::RoutineHandlerGroup m_routines;

      void Shutdown();
      void ReplayBboQuotes(const Security& security);
      void ReplayMarketQuotes(const Security& security);
      void ReplayBookQuotes(const Security& security);
      void ReplayTimeAndSales(const Security& security);
  };

  template<typename M, typename D, typename T, typename R>
  template<typename MF, typename DF, typename TF>
  ReplayMarketDataFeedClient<M, D, T, R>::ReplayMarketDataFeedClient(
      std::vector<Security> securities, boost::posix_time::ptime startTime,
      MF&& feedClient, DF&& dataStore, TF&& timeClient,
      TimerBuilder timerBuilder)
      : m_securities(std::move(securities)),
        m_startTime(startTime),
        m_feedClient(std::forward<MF>(feedClient)),
        m_dataStore(std::forward<DF>(dataStore)),
        m_timeClient(std::forward<TF>(timeClient)),
        m_timerBuilder(std::move(timerBuilder)) {}

  template<typename M, typename D, typename T, typename R>
  ReplayMarketDataFeedClient<M, D, T, R>::~ReplayMarketDataFeedClient() {
    Close();
  }

  template<typename M, typename D, typename T, typename R>
  void ReplayMarketDataFeedClient<M, D, T, R>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_feedClient->Open();
      m_dataStore->Open();
      m_timeClient->Open();
      for(auto& security : m_securities) {
        m_routines.Spawn(
          [=] {
            ReplayBboQuotes(security);
          });
        m_routines.Spawn(
          [=] {
            ReplayMarketQuotes(security);
          });
        m_routines.Spawn(
          [=] {
            ReplayBookQuotes(security);
          });
        m_routines.Spawn(
          [=] {
            ReplayTimeAndSales(security);
          });
      }
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename M, typename D, typename T, typename R>
  void ReplayMarketDataFeedClient<M, D, T, R>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename M, typename D, typename T, typename R>
  void ReplayMarketDataFeedClient<M, D, T, R>::Shutdown() {
    m_openState.SetClosed();
  }

  template<typename M, typename D, typename T, typename R>
  void ReplayMarketDataFeedClient<M, D, T, R>::ReplayBboQuotes(
      const Security& security) {
    constexpr auto QUERY_SIZE = 1000;
    auto currentTime = m_timeClient->GetTime();
    auto replayTime = m_startTime;
    auto query = MarketDataService::SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(replayTime, Beam::Queries::Sequence::Last());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::FromHead(QUERY_SIZE));
    auto data = m_dataStore->LoadBboQuotes(query);
    auto updateTime = m_timeClient->GetTime();
    auto timeDelta = updateTime - currentTime;
    replayTime += timeDelta;
    currentTime = updateTime;
    for(auto& item : data) {
      auto wait = Beam::Queries::GetTimestamp(*item) - replayTime;
      if(wait > boost::posix_time::seconds(0)) {
        auto timer = m_timerBuilder(wait);
        timer->Start();
        timer->Wait();
      }
      auto replayValue = *item;
      Beam::Queries::GetTimestamp(replayValue) = m_timeClient->GetTime();
      m_feedClient->PublishBboQuote(Beam::Queries::MakeIndexedValue(
        replayValue, security));
      updateTime = m_timeClient->GetTime();
      timeDelta = updateTime - currentTime;
      replayTime += timeDelta;
      currentTime = updateTime;
    }
  }

  template<typename M, typename D, typename T, typename R>
  void ReplayMarketDataFeedClient<M, D, T, R>::ReplayMarketQuotes(
      const Security& security) {
  }

  template<typename M, typename D, typename T, typename R>
  void ReplayMarketDataFeedClient<M, D, T, R>::ReplayBookQuotes(
      const Security& security) {
  }

  template<typename M, typename D, typename T, typename R>
  void ReplayMarketDataFeedClient<M, D, T, R>::ReplayTimeAndSales(
      const Security& security) {
  }
}

#endif
