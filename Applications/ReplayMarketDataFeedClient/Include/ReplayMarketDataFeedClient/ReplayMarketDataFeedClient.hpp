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
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
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
  template<typename M, typename D, typename T, typename R> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsHistoricalDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<T>> && Beam::IsTimer<R>
  class ReplayMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient connected to the
       * MarketDataFeedServer.
       */
      using MarketDataFeedClient = Beam::dereference_t<M>;

      /** The type of HistoricalDataStore to load market data from. */
      using HistoricalDataStore = Beam::dereference_t<D>;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::dereference_t<T>;

      /** The type of Timer to use. */
      using Timer = R;

      /** The builder used to build Timer instances. */
      using TimerBuilder = std::function<
        std::unique_ptr<Timer> (boost::posix_time::time_duration)>;

      /**
       * Constructs a ReplayMarketDataFeedClient.
       * @param tickers The list of Tickers to replay.
       * @param replay_time The timestamp to begin loading data to replay.
       * @param feed_client Initializes the MarketDataFeedClient to send the
       *        replayed data to.
       * @param data_store The HistoricalDataStore to load market data from.
       * @param time_client Initializes the TimeClient.
       * @param timer_builder The builder used to build Timer instances.
       */
      template<Beam::Initializes<M> MF, Beam::Initializes<D> DF,
        Beam::Initializes<T> TF>
      ReplayMarketDataFeedClient(std::vector<Ticker> tickers,
        boost::posix_time::ptime replay_time, MF&& feed_client, DF&& data_store,
        TF&& time_client, TimerBuilder timer_builder);

      ~ReplayMarketDataFeedClient();

      void close();

    private:
      std::vector<Ticker> m_tickers;
      boost::posix_time::ptime m_replay_time;
      Beam::local_ptr_t<M> m_feed_client;
      Beam::local_ptr_t<D> m_data_store;
      Beam::local_ptr_t<T> m_time_client;
      boost::posix_time::ptime m_open_time;
      TimerBuilder m_timer_builder;
      Beam::Mutex m_pending_mutex;
      std::size_t m_pending_load_count;
      Beam::ConditionVariable m_is_pending_load;
      Beam::OpenState m_open_state;
      Beam::RoutineHandlerGroup m_routines;

      ReplayMarketDataFeedClient(const ReplayMarketDataFeedClient&) = delete;
      ReplayMarketDataFeedClient& operator =(
        const ReplayMarketDataFeedClient&) = delete;
      template<typename F, typename P>
      void replay(const Ticker& ticker, F&& query_loader, P&& publisher);
  };

  template<typename M, typename D, typename T, typename R> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsHistoricalDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<T>> && Beam::IsTimer<R>
  template<Beam::Initializes<M> MF, Beam::Initializes<D> DF,
    Beam::Initializes<T> TF>
  ReplayMarketDataFeedClient<M, D, T, R>::ReplayMarketDataFeedClient(
      std::vector<Ticker> tickers, boost::posix_time::ptime replay_time,
      MF&& feed_client, DF&& data_store, TF&& time_client,
      TimerBuilder timer_builder)
      : m_tickers(std::move(tickers)),
        m_replay_time(replay_time),
        m_feed_client(std::forward<MF>(feed_client)),
        m_data_store(std::forward<DF>(data_store)),
        m_time_client(std::forward<TF>(time_client)),
        m_timer_builder(std::move(timer_builder)) {
    try {
      m_open_time = m_time_client->get_time();
      m_pending_load_count = 4 * m_tickers.size();
      for(auto& ticker : m_tickers) {
        m_routines.spawn([=, this] {
          replay(ticker, [this] (const auto& query) {
            return m_data_store->load_bbo_quotes(query);
          },
          [this] (const auto& value) {
            return m_feed_client->publish(value);
          });
        });
        m_routines.spawn([=, this] {
          replay(ticker, [this] (const auto& query) {
            return m_data_store->load_book_quotes(query);
          },
          [this] (const auto& value) {
            return m_feed_client->publish(value);
          });
        });
        m_routines.spawn([=, this] {
          replay(ticker, [this] (const auto& query) {
            return m_data_store->load_time_and_sales(query);
          },
          [this] (const auto& value) {
            return m_feed_client->publish(value);
          });
        });
      }
    } catch(std::exception&) {
      close();
      throw;
    }
  }

  template<typename M, typename D, typename T, typename R> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsHistoricalDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<T>> && Beam::IsTimer<R>
  ReplayMarketDataFeedClient<M, D, T, R>::~ReplayMarketDataFeedClient() {
    close();
  }

  template<typename M, typename D, typename T, typename R> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsHistoricalDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<T>> && Beam::IsTimer<R>
  void ReplayMarketDataFeedClient<M, D, T, R>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_routines.wait();
    m_open_state.close();
  }

  template<typename M, typename D, typename T, typename R> requires
    IsMarketDataFeedClient<Beam::dereference_t<M>> &&
      IsHistoricalDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<T>> && Beam::IsTimer<R>
  template<typename F, typename P>
  void ReplayMarketDataFeedClient<M, D, T, R>::replay(
      const Ticker& ticker, F&& query_loader, P&& publisher) {
    const auto QUERY_SIZE = 1000;
    const auto WAIT_QUANTUM =
      boost::posix_time::time_duration(boost::posix_time::seconds(1));
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(m_replay_time, Beam::Sequence::LAST);
    query.set_snapshot_limit(Beam::SnapshotLimit::from_head(QUERY_SIZE));
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
    auto current_time = m_time_client->get_time();
    auto replay_time = m_replay_time + (current_time - m_open_time);
    while(!data.empty() && m_open_state.is_open()) {
      for(auto& item : data) {
        auto wait = Beam::get_timestamp(*item) - replay_time;
        while(m_open_state.is_open() && wait > boost::posix_time::seconds(0)) {
          auto timer = m_timer_builder(std::min(wait, WAIT_QUANTUM));
          timer->start();
          timer->wait();
          wait -= WAIT_QUANTUM;
        }
        if(!m_open_state.is_open()) {
          return;
        }
        Beam::get_timestamp(*item) = m_time_client->get_time();
        publisher(Beam::IndexedValue(*item, ticker));
        auto updated_time = m_time_client->get_time();
        replay_time += updated_time - current_time;
        current_time = updated_time;
      }
      query.set_range(
        Beam::increment(data.back().get_sequence()), Beam::Sequence::LAST);
      data = query_loader(query);
      auto updated_time = m_time_client->get_time();
      replay_time += updated_time - current_time;
      current_time = updated_time;
    }
  }
}

#endif
