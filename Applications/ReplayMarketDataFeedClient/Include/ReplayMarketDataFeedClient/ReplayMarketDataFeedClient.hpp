#ifndef NEXUS_REPLAY_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_REPLAY_MARKET_DATA_FEED_CLIENT_HPP
#include <atomic>
#include <functional>
#include <memory>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Timer.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"

namespace Nexus {

  /**
   * Sends historical market data from a data store to a market data server.
   * @param <M> The type of MarketDataFeedClient connected to the
   *        MarketDataFeedServer.
   * @param <D> The type of HistoricalDataStore to load market data from.
   * @param <T> The type of TimeClient to use.
   * @param <R> The type of Timer to use.
   */
  template<typename M, typename D, typename T, typename R>
  class ReplayMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient connected to the
       * MarketDataFeedServer.
       */
      using MarketDataFeedClient = Beam::GetTryDereferenceType<M>;

      /** The type of HistoricalDataStore to load market data from. */
      using HistoricalDataStore = Beam::GetTryDereferenceType<D>;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /** The type of Timer to use. */
      using Timer = R;

      /** The builder used to build Timer instances. */
      using TimerBuilder = std::function<
        std::unique_ptr<Timer> (boost::posix_time::time_duration)>;

      /**
       * Constructs a ReplayMarketDataFeedClient.
       * @param securities The list of Securities to replay.
       * @param replayTime The timestamp to begin loading data to replay.
       * @param feedClient Initializes the MarketDataFeedClient to send the
       *        replayed data to.
       * @param dataStore The HistoricalDataStore to load market data from.
       * @param timeClient Initializes the TimeClient.
       * @param timerBuilder The builder used to build Timer instances.
       */
      template<typename MF, typename DF, typename TF>
      ReplayMarketDataFeedClient(std::vector<Security> securities,
        boost::posix_time::ptime replayTime, MF&& feedClient, DF&& dataStore,
        TF&& timeClient, TimerBuilder timerBuilder);

      ~ReplayMarketDataFeedClient();

      void Open();

      void Close();

    private:
      std::vector<Security> m_securities;
      boost::posix_time::ptime m_replayTime;
      Beam::GetOptionalLocalPtr<M> m_feedClient;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      boost::posix_time::ptime m_openTime;
      TimerBuilder m_timerBuilder;
      Beam::Threading::Mutex m_pendingMutex;
      std::size_t m_pendingLoadCount;
      Beam::Threading::ConditionVariable m_isPendingLoad;
      Beam::IO::OpenState m_openState;
      Beam::Routines::RoutineHandlerGroup m_routines;

      ReplayMarketDataFeedClient(const ReplayMarketDataFeedClient&) = delete;
      ReplayMarketDataFeedClient& operator =(
        const ReplayMarketDataFeedClient&) = delete;
      template<typename F, typename P>
      void ReplayMarketData(const Security& security, F&& queryLoader,
        P&& publisher);
  };

  template<typename M, typename D, typename T, typename R>
  template<typename MF, typename DF, typename TF>
  ReplayMarketDataFeedClient<M, D, T, R>::ReplayMarketDataFeedClient(
      std::vector<Security> securities, boost::posix_time::ptime replayTime,
      MF&& feedClient, DF&& dataStore, TF&& timeClient,
      TimerBuilder timerBuilder)
      : m_securities(std::move(securities)),
        m_replayTime(replayTime),
        m_feedClient(std::forward<MF>(feedClient)),
        m_dataStore(std::forward<DF>(dataStore)),
        m_timeClient(std::forward<TF>(timeClient)),
        m_timerBuilder(std::move(timerBuilder)) {
    try {
      m_openTime = m_timeClient->GetTime();
      m_pendingLoadCount = 4 * m_securities.size();
      for(auto& security : m_securities) {
        m_routines.Spawn([=, this] {
          ReplayMarketData(security,
            [this] (const auto& query) {
              return m_dataStore->LoadBboQuotes(query);
            },
            [this] (const auto& value) {
              return m_feedClient->Publish(value);
            });
        });
        m_routines.Spawn([=, this] {
          ReplayMarketData(security,
            [this] (const auto& query) {
              return m_dataStore->LoadMarketQuotes(query);
            },
            [this] (const auto& value) {
              return m_feedClient->Publish(value);
            });
        });
        m_routines.Spawn([=, this] {
          ReplayMarketData(security,
            [this] (const auto& query) {
              return m_dataStore->LoadBookQuotes(query);
            },
            [this] (const auto& value) {
              return m_feedClient->Publish(value);
            });
        });
        m_routines.Spawn([=, this] {
          ReplayMarketData(security,
            [this] (const auto& query) {
              return m_dataStore->LoadTimeAndSales(query);
            },
            [this] (const auto& value) {
              return m_feedClient->Publish(value);
            });
        });
      }
    } catch(std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename M, typename D, typename T, typename R>
  ReplayMarketDataFeedClient<M, D, T, R>::~ReplayMarketDataFeedClient() {
    Close();
  }

  template<typename M, typename D, typename T, typename R>
  void ReplayMarketDataFeedClient<M, D, T, R>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_routines.Wait();
    m_openState.Close();
  }

  template<typename M, typename D, typename T, typename R>
  template<typename F, typename P>
  void ReplayMarketDataFeedClient<M, D, T, R>::ReplayMarketData(
      const Security& security, F&& queryLoader, P&& publisher) {
    constexpr auto QUERY_SIZE = 1000;
    const auto WAIT_QUANTUM = boost::posix_time::time_duration(
      boost::posix_time::seconds(1));
    auto query = MarketDataService::SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(m_replayTime, Beam::Queries::Sequence::Last());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::FromHead(QUERY_SIZE));
    auto data = queryLoader(query);
    {
      auto lock = boost::unique_lock(m_pendingMutex);
      --m_pendingLoadCount;
      if(m_pendingLoadCount == 0) {
        m_isPendingLoad.notify_all();
      } else {
        m_isPendingLoad.wait(lock);
      }
    }
    auto currentTime = m_timeClient->GetTime();
    auto replayTime = m_replayTime + (currentTime - m_openTime);
    while(!data.empty() && m_openState.IsOpen()) {
      for(auto& item : data) {
        auto wait = Beam::Queries::GetTimestamp(*item) - replayTime;
        while(m_openState.IsOpen() && wait > boost::posix_time::seconds(0)) {
          auto timer = m_timerBuilder(std::min(wait, WAIT_QUANTUM));
          timer->Start();
          timer->Wait();
          wait -= WAIT_QUANTUM;
        }
        if(!m_openState.IsOpen()) {
          return;
        }
        Beam::Queries::GetTimestamp(*item) = m_timeClient->GetTime();
        publisher(Beam::Queries::IndexedValue(*item, security));
        auto updatedTime = m_timeClient->GetTime();
        replayTime += updatedTime - currentTime;
        currentTime = updatedTime;
      }
      query.SetRange(Beam::Queries::Increment(data.back().GetSequence()),
        Beam::Queries::Sequence::Last());
      data = queryLoader(query);
      auto updatedTime = m_timeClient->GetTime();
      replayTime += updatedTime - currentTime;
      currentTime = updatedTime;
    }
  }
}

#endif
