#ifndef ASX_TRADE_ITCH_CLIENT_HPP
#define ASX_TRADE_ITCH_CLIENT_HPP
#include <iterator>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchGlimpseClient.hpp"
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchRecoveryScheduler.hpp"
#include "Nexus/MoldUdp64/MoldUdp64Client.hpp"

namespace Nexus {

  /** Concept satisfied by clients delivering ordered ASX Trade ITCH data. */
  template<typename T>
  concept IsAsxTradeItchClient = requires(T& client) {
    { client.read() } -> std::same_as<AsxTradeItchMessage>;
    { client.close() } -> std::same_as<void>;
  };

  /**
   * Delivers one partition's snapshot followed by its ordered ITCH stream.
   * Follows one MoldUDP64 session through its end-of-session marker.
   * @tparam P The client receiving a multicast feed.
   * @tparam G The client requesting and receiving retransmissions.
   * @tparam S The client loading the optional startup snapshot.
   * @tparam R The client supplying the current local time.
   * @tparam T The timer driving feed expiry and recovery retries.
   */
  template<typename P, typename G, typename S, typename R, typename T> requires
    IsMoldUdp64Reader<Beam::dereference_t<P>> &&
      IsMoldUdp64Client<Beam::dereference_t<G>> &&
      IsAsxTradeItchGlimpseClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class AsxTradeItchClient {
    public:

      /** The client receiving a multicast feed. */
      using ProtocolClient = Beam::dereference_t<P>;

      /** The client requesting and receiving retransmissions. */
      using RecoveryClient = Beam::dereference_t<G>;

      /** The client loading the startup snapshot. */
      using GlimpseClient = Beam::dereference_t<S>;

      /** The client supplying the current local time. */
      using TimeClient = Beam::dereference_t<R>;

      /** The timer driving feed expiry and recovery retries. */
      using Timer = Beam::dereference_t<T>;

      /**
       * Constructs an AsxTradeItchClient.
       * @param feed_timeout How long a feed may remain silent or stalled.
       * @param request_timeout How long to wait before retrying recovery.
       * @param feed_clients The clients receiving redundant multicast feeds.
       * @param recovery_client The partition's rewind client.
       * @param glimpse_client The optional startup snapshot client.
       * @param time_client The client supplying local time.
       * @param timer The timer driving feed expiry and recovery retries.
       */
      template<Beam::Initializes<P> PF, Beam::Initializes<G> GF,
        Beam::Initializes<S> SF, Beam::Initializes<R> RF,
        Beam::Initializes<T> TF>
      AsxTradeItchClient(boost::posix_time::time_duration feed_timeout,
        boost::posix_time::time_duration request_timeout,
        std::vector<PF> feed_clients, GF&& recovery_client,
        boost::optional<SF> glimpse_client, RF&& time_client, TF&& timer);

      ~AsxTradeItchClient();

      /**
       * Reads the next snapshot or sequenced multicast message.
       * @return A message valid until the next read or client destruction.
       */
      AsxTradeItchMessage read();

      /** Closes all clients and interrupts pending reads and requests. */
      void close();

    private:
      struct State {
        AsxTradeItchSequencer m_sequencer;
        AsxTradeItchRecoveryScheduler m_recovery;
        int m_feed_count;
        bool m_is_ready;
        bool m_is_finished;
      };
      std::vector<Beam::local_ptr_t<P>> m_feed_clients;
      Beam::local_ptr_t<G> m_recovery_client;
      boost::optional<Beam::local_ptr_t<S>> m_glimpse_client;
      Beam::local_ptr_t<R> m_time_client;
      Beam::local_ptr_t<T> m_timer;
      Beam::Sync<State> m_state;
      Beam::Queue<Beam::SharedBuffer> m_messages;
      Beam::StateQueue<bool> m_requests;
      Beam::Queue<bool> m_snapshot_start;
      Beam::SharedBuffer m_payload;
      Beam::RoutineTaskQueue m_tasks;
      Beam::RoutineHandlerGroup m_routines;
      Beam::OpenState m_open_state;

      AsxTradeItchClient(const AsxTradeItchClient&) = delete;
      AsxTradeItchClient& operator =(const AsxTradeItchClient&) = delete;
      void flush(State& state);
      void fail(const std::exception_ptr& error);
      void feed_loop(int index);
      void recovery_loop();
      void request_loop();
      void snapshot_loop();
      void on_timer(typename Timer::Result result);
  };

  template<typename P, typename G, typename S, typename R, typename T>
  AsxTradeItchClient(boost::posix_time::time_duration,
    boost::posix_time::time_duration, std::vector<P>, G&&,
    boost::optional<S>, R&&, T&&) ->
      AsxTradeItchClient<P, std::remove_cvref_t<G>, S,
        std::remove_cvref_t<R>, std::remove_cvref_t<T>>;

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsMoldUdp64Reader<Beam::dereference_t<P>> &&
      IsMoldUdp64Client<Beam::dereference_t<G>> &&
      IsAsxTradeItchGlimpseClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<P> PF, Beam::Initializes<G> GF,
    Beam::Initializes<S> SF, Beam::Initializes<R> RF, Beam::Initializes<T> TF>
  AsxTradeItchClient<P, G, S, R, T>::AsxTradeItchClient(
      boost::posix_time::time_duration feed_timeout,
      boost::posix_time::time_duration request_timeout,
      std::vector<PF> feed_clients, GF&& recovery_client,
      boost::optional<SF> glimpse_client, RF&& time_client, TF&& timer)
      : m_feed_clients(std::make_move_iterator(feed_clients.begin()),
          std::make_move_iterator(feed_clients.end())),
        m_recovery_client(std::forward<GF>(recovery_client)),
        m_glimpse_client(std::move(glimpse_client)),
        m_time_client(std::forward<RF>(time_client)),
        m_timer(std::forward<TF>(timer)),
        m_state(AsxTradeItchSequencer(
            static_cast<int>(m_feed_clients.size()), feed_timeout),
          AsxTradeItchRecoveryScheduler(request_timeout),
          static_cast<int>(m_feed_clients.size()), !m_glimpse_client, false) {
    try {
      if(m_feed_clients.empty() || feed_timeout.is_special() ||
          feed_timeout <= boost::posix_time::seconds(0)) {
        boost::throw_with_location(
          std::invalid_argument("A feed and a positive timeout are required."));
      }
      for(auto i = std::size_t(0); i != m_feed_clients.size(); ++i) {
        m_routines.spawn(std::bind_front(
          &AsxTradeItchClient::feed_loop, this, static_cast<int>(i)));
      }
      m_routines.spawn(
        std::bind_front(&AsxTradeItchClient::recovery_loop, this));
      m_routines.spawn(
        std::bind_front(&AsxTradeItchClient::request_loop, this));
      if(m_glimpse_client) {
        m_routines.spawn(
          std::bind_front(&AsxTradeItchClient::snapshot_loop, this));
      }
      m_timer->get_publisher().monitor(m_tasks.get_slot<typename Timer::Result>(
        std::bind_front(&AsxTradeItchClient::on_timer, this)));
      m_timer->start();
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsMoldUdp64Reader<Beam::dereference_t<P>> &&
      IsMoldUdp64Client<Beam::dereference_t<G>> &&
      IsAsxTradeItchGlimpseClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  AsxTradeItchClient<P, G, S, R, T>::~AsxTradeItchClient() {
    close();
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsMoldUdp64Reader<Beam::dereference_t<P>> &&
      IsMoldUdp64Client<Beam::dereference_t<G>> &&
      IsAsxTradeItchGlimpseClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  AsxTradeItchMessage AsxTradeItchClient<P, G, S, R, T>::read() {
    m_payload = m_messages.pop();
    return AsxTradeItchMessage::parse(
      std::string_view(m_payload.get_data(), m_payload.get_size()));
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsMoldUdp64Reader<Beam::dereference_t<P>> &&
      IsMoldUdp64Client<Beam::dereference_t<G>> &&
      IsAsxTradeItchGlimpseClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void AsxTradeItchClient<P, G, S, R, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_messages.close(std::make_exception_ptr(Beam::EndOfFileException()));
    m_requests.close();
    m_snapshot_start.close();
    if(m_glimpse_client) {
      (*m_glimpse_client)->close();
    }
    m_recovery_client->close();
    for(auto& client : m_feed_clients) {
      client->close();
    }
    m_tasks.close();
    m_tasks.wait();
    m_timer->cancel();
    m_routines.wait();
    m_open_state.close();
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsMoldUdp64Reader<Beam::dereference_t<P>> &&
      IsMoldUdp64Client<Beam::dereference_t<G>> &&
      IsAsxTradeItchGlimpseClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void AsxTradeItchClient<P, G, S, R, T>::flush(State& state) {
    if(!state.m_is_ready || state.m_is_finished) {
      return;
    }
    while(auto payload = state.m_sequencer.read()) {
      m_messages.push(std::move(*payload));
    }
    if(state.m_sequencer.is_end_of_session()) {
      state.m_is_finished = true;
      m_messages.close(std::make_exception_ptr(Beam::EndOfFileException()));
      m_requests.close();
    } else if(state.m_sequencer.get_gap()) {
      m_requests.push(true);
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsMoldUdp64Reader<Beam::dereference_t<P>> &&
      IsMoldUdp64Client<Beam::dereference_t<G>> &&
      IsAsxTradeItchGlimpseClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void AsxTradeItchClient<P, G, S, R, T>::fail(
      const std::exception_ptr& error) {
    Beam::with(m_state, [&] (auto& state) {
      state.m_is_finished = true;
      m_messages.close(error);
      m_requests.close(error);
      m_snapshot_start.close(error);
    });
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsMoldUdp64Reader<Beam::dereference_t<P>> &&
      IsMoldUdp64Client<Beam::dereference_t<G>> &&
      IsAsxTradeItchGlimpseClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void AsxTradeItchClient<P, G, S, R, T>::feed_loop(int index) {
    while(m_open_state.is_open()) {
      try {
        auto packet = m_feed_clients[index]->read();
        validate(packet);
        auto timestamp = m_time_client->get_time();
        auto is_finished = Beam::with(m_state, [&] (auto& state) {
          if(state.m_is_finished) {
            return true;
          }
          auto has_session = state.m_sequencer.get_session().has_value();
          state.m_sequencer.add(index, packet, timestamp);
          if(!has_session && m_glimpse_client) {
            m_snapshot_start.push(true);
          }
          flush(state);
          return state.m_is_finished;
        });
        if(is_finished) {
          return;
        }
      } catch(const MoldUdp64ParserException&) {
      } catch(const AsxTradeItchParserException&) {
      } catch(const std::exception&) {
        auto error = std::current_exception();
        auto has_feeds = Beam::with(m_state, [&] (auto& state) {
          --state.m_feed_count;
          return state.m_feed_count != 0;
        });
        if(!has_feeds && m_open_state.is_open()) {
          fail(error);
        }
        return;
      }
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsMoldUdp64Reader<Beam::dereference_t<P>> &&
      IsMoldUdp64Client<Beam::dereference_t<G>> &&
      IsAsxTradeItchGlimpseClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void AsxTradeItchClient<P, G, S, R, T>::recovery_loop() {
    while(m_open_state.is_open()) {
      try {
        auto packet = m_recovery_client->read();
        validate(packet);
        auto is_finished = Beam::with(m_state, [&] (auto& state) {
          if(state.m_is_finished) {
            return true;
          }
          state.m_sequencer.recover(packet);
          flush(state);
          return state.m_is_finished;
        });
        if(is_finished) {
          return;
        }
      } catch(const MoldUdp64ParserException&) {
      } catch(const AsxTradeItchParserException&) {
      } catch(const std::exception&) {
        if(m_open_state.is_open()) {
          fail(std::current_exception());
        }
        return;
      }
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsMoldUdp64Reader<Beam::dereference_t<P>> &&
      IsMoldUdp64Client<Beam::dereference_t<G>> &&
      IsAsxTradeItchGlimpseClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void AsxTradeItchClient<P, G, S, R, T>::request_loop() {
    try {
      while(m_open_state.is_open()) {
        m_requests.pop();
        auto timestamp = m_time_client->get_time();
        auto request = Beam::with(m_state,
          [&] (auto& state) -> boost::optional<MoldUdp64Request> {
            if(state.m_is_finished || !state.m_is_ready) {
              return boost::none;
            }
            return state.m_recovery.request(state.m_sequencer, timestamp);
          });
        if(request) {
          m_recovery_client->request(*request);
        }
      }
    } catch(const std::exception&) {
      if(m_open_state.is_open()) {
        fail(std::current_exception());
      }
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsMoldUdp64Reader<Beam::dereference_t<P>> &&
      IsMoldUdp64Client<Beam::dereference_t<G>> &&
      IsAsxTradeItchGlimpseClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void AsxTradeItchClient<P, G, S, R, T>::snapshot_loop() {
    try {
      m_snapshot_start.pop();
      auto snapshot = (*m_glimpse_client)->load_snapshot();
      Beam::with(m_state, [&] (auto& state) {
        if(state.m_is_finished) {
          return;
        }
        state.m_sequencer.reset(snapshot.m_sequence);
        state.m_recovery.reset();
        for(auto& payload : snapshot.m_messages) {
          m_messages.push(std::move(payload));
        }
        state.m_is_ready = true;
        flush(state);
      });
    } catch(const std::exception&) {
      if(m_open_state.is_open()) {
        fail(std::current_exception());
      }
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsMoldUdp64Reader<Beam::dereference_t<P>> &&
      IsMoldUdp64Client<Beam::dereference_t<G>> &&
      IsAsxTradeItchGlimpseClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void AsxTradeItchClient<P, G, S, R, T>::on_timer(
      typename Timer::Result result) {
    if(!m_open_state.is_open() || result == Timer::Result::CANCELED) {
      return;
    }
    try {
      if(result == Timer::Result::FAIL) {
        boost::throw_with_location(Beam::IOException("ITCH timer failed."));
      }
      auto timestamp = m_time_client->get_time();
      auto is_finished = Beam::with(m_state, [&] (auto& state) {
        state.m_sequencer.update(timestamp);
        flush(state);
        return state.m_is_finished;
      });
      if(!is_finished) {
        m_timer->start();
      }
    } catch(const std::exception&) {
      fail(std::current_exception());
    }
  }
}

#endif
