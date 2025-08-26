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
  template<IsMarketDataFeedClient M, IsHistoricalDataStore D, typename T,
    typename R>
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
       * @param replay_time The timestamp to begin loading data to replay.
       * @param feed_client Initializes the MarketDataFeedClient to send the
       *        replayed data to.
       * @param data_store The HistoricalDataStore to load market data from.
       * @param time_client Initializes the TimeClient.
       * @param timer_builder The builder used to build Timer instances.
       */
      template<Beam::Initializes<M> MF, Beam::Initializes<D> DF,
        Beam::Initializes<T> TF>
      ReplayMarketDataFeedClient(std::vector<Security> securities,
        boost::posix_time::ptime replay_time, MF&& feed_client, DF&& data_store,
        TF&& time_client, TimerBuilder timer_builder);

      ~ReplayMarketDataFeedClient();

      void close();

    private:
      std::vector<Security> m_securities;
      boost::posix_time::ptime m_replay_time;
      Beam::GetOptionalLocalPtr<M> m_feed_client;
      Beam::GetOptionalLocalPtr<D> m_data_store;
      Beam::GetOptionalLocalPtr<T> m_time_client;
      boost::posix_time::ptime m_open_time;
      TimerBuilder m_timer_builder;
      Beam::Threading::Mutex m_pending_mutex;
      std::size_t m_pending_load_count;
      Beam::Threading::ConditionVariable m_is_pending_load;
      Beam::IO::OpenState m_open_state;
      Beam::Routines::RoutineHandlerGroup m_routines;

      ReplayMarketDataFeedClient(const ReplayMarketDataFeedClient&) = delete;
      ReplayMarketDataFeedClient& operator =(
        const ReplayMarketDataFeedClient&) = delete;
      template<typename F, typename P>
      void replay(const Security& security, F&& query_loader, P&& publisher);
  };

  template<IsMarketDataFeedClient M, IsHistoricalDataStore D, typename T,
    typename R>
  template<Beam::Initializes<M> MF, Beam::Initializes<D> DF,
    Beam::Initializes<T> TF>
  ReplayMarketDataFeedClient<M, D, T, R>::ReplayMarketDataFeedClient(
      std::vector<Security> securities, boost::posix_time::ptime replay_time,
      MF&& feed_client, DF&& data_store, TF&& time_client,
      TimerBuilder timer_builder)
      : m_securities(std::move(securities)),
        m_replay_time(replay_time),
        m_feed_client(std::forward<MF>(feed_client)),
        m_data_store(std::forward<DF>(data_store)),
        m_time_client(std::forward<TF>(time_client)),
        m_timer_builder(std::move(timer_builder)) {
    try {
      m_open_time = m_time_client->GetTime();
      m_pending_load_count = 4 * m_securities.size();
      for(auto& security : m_securities) {
        m_routines.Spawn([=, this] {
          replay(security, [this] (const auto& query) {
            return m_data_store->load_bbo_quotes(query);
          },
          [this] (const auto& value) {
            return m_feed_client->publish(value);
          });
        });
        m_routines.Spawn([=, this] {
          replay(security, [this] (const auto& query) {
            return m_data_store->load_book_quotes(query);
          },
          [this] (const auto& value) {
            return m_feed_client->publish(value);
          });
        });
        m_routines.Spawn([=, this] {
          replay(security, [this] (const auto& query) {
            return m_data_store->load_time_and_sales(query);
          },
          [this] (const auto& value) {
            return m_feed_client->publish(value);
          });
        });
      }
    } catch(std::exception&) {
      close();
      BOOST_RETHROW;
    }
  }

  template<IsMarketDataFeedClient M, IsHistoricalDataStore D, typename T,
    typename R>
  ReplayMarketDataFeedClient<M, D, T, R>::~ReplayMarketDataFeedClient() {
    close();
  }

  template<IsMarketDataFeedClient M, IsHistoricalDataStore D, typename T,
    typename R>
  void ReplayMarketDataFeedClient<M, D, T, R>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_routines.Wait();
    m_open_state.Close();
  }

  template<IsMarketDataFeedClient M, IsHistoricalDataStore D, typename T,
    typename R>
  template<typename F, typename P>
  void ReplayMarketDataFeedClient<M, D, T, R>::replay(
      const Security& security, F&& query_loader, P&& publisher) {
    const auto QUERY_SIZE = 1000;
    const auto WAIT_QUANTUM =
      boost::posix_time::time_duration(boost::posix_time::seconds(1));
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(m_replay_time, Beam::Queries::Sequence::Last());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::FromHead(QUERY_SIZE));
    auto data = query_loader(query);
    {
      auto lock = std::unique_lock(m_pending_mutex);
      --m_pending_load_count;
      if(m_pending_load_count == 0) {
        m_is_pending_load.notify_all();
      } else {
        m_is_pending_load.wait(lock);
      }
    }
    auto current_time = m_time_client->GetTime();
    auto replay_time = m_replay_time + (current_time - m_open_time);
    while(!data.empty() && m_open_state.IsOpen()) {
      for(auto& item : data) {
        auto wait = Beam::Queries::GetTimestamp(*item) - replay_time;
        while(m_open_state.IsOpen() && wait > boost::posix_time::seconds(0)) {
          auto timer = m_timer_builder(std::min(wait, WAIT_QUANTUM));
          timer->Start();
          timer->Wait();
          wait -= WAIT_QUANTUM;
        }
        if(!m_open_state.IsOpen()) {
          return;
        }
        Beam::Queries::GetTimestamp(*item) = m_time_client->GetTime();
        publisher(Beam::Queries::IndexedValue(*item, security));
        auto updated_time = m_time_client->GetTime();
        replay_time += updated_time - current_time;
        current_time = updated_time;
      }
      query.SetRange(Beam::Queries::Increment(data.back().GetSequence()),
        Beam::Queries::Sequence::Last());
      data = query_loader(query);
      auto updated_time = m_time_client->GetTime();
      replay_time += updated_time - current_time;
      current_time = updated_time;
    }
  }
}

#endif
