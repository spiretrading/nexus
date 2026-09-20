#ifndef TMX_IP_CLIENT_HPP
#define TMX_IP_CLIENT_HPP
#include <algorithm>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Threading/Sync.hpp>
#include "TmxIpMarketDataFeedClient/TmxIpMessageBuilder.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpRecoveryClient.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpSequencer.hpp"

namespace Nexus {

  /** Concept satisfied by clients delivering ordered STAMP messages. */
  template<typename T>
  concept IsTmxIpClient = requires(T& t) {
    { t.read() } -> std::same_as<StampMessage>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Delivers complete, ordered messages from one TMX IP stream.
   * Both clients must receive the same service from the same site.
   * Starts at the first live packet or heartbeat; recreate for a new day.
   * @tparam P The live protocol client type.
   * @tparam R The recovery client type.
   * @tparam T The timer pacing subsequent recovery requests.
   */
  template<typename P, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class TmxIpClient {
    public:

      /** The live protocol client type. */
      using ProtocolClient = Beam::dereference_t<P>;

      /** The recovery client type. */
      using RecoveryClient = Beam::dereference_t<R>;

      /** The timer pacing subsequent recovery requests. */
      using Timer = Beam::dereference_t<T>;

      /**
       * Constructs a TMX IP client.
       * @param protocol_client Receives live packets.
       * @param recovery_client Requests and receives missing packets.
       * @param timer Drives retries and remaining recovery chunks.
       */
      template<Beam::Initializes<P> PF, Beam::Initializes<R> RF,
        Beam::Initializes<T> TF>
      TmxIpClient(PF&& protocol_client, RF&& recovery_client, TF&& timer);

      ~TmxIpClient();

      /**
       * Reads the next complete STAMP message.
       * The returned views remain valid until the next read or destruction.
       * Throws on transport failures, failed recovery, or malformed messages.
       */
      StampMessage read();

      /** Closes both clients and interrupts pending reads and requests. */
      void close();

    private:
      enum class RequestState {
        READY,
        PENDING,
        DEFERRED
      };
      struct State {
        TmxIpSequencer m_sequencer;
        TmxIpMessageBuilder m_builder;
        RequestState m_request_state = RequestState::READY;
        bool m_is_finished = false;
      };
      Beam::local_ptr_t<P> m_protocol_client;
      Beam::local_ptr_t<R> m_recovery_client;
      Beam::local_ptr_t<T> m_timer;
      Beam::Sync<State> m_state;
      Beam::Queue<Beam::SharedBuffer> m_messages;
      Beam::Queue<bool> m_requests;
      Beam::SharedBuffer m_payload;
      Beam::RoutineTaskQueue m_tasks;
      Beam::RoutineHandlerGroup m_routines;
      Beam::OpenState m_open_state;

      TmxIpClient(const TmxIpClient&) = delete;
      TmxIpClient& operator =(const TmxIpClient&) = delete;
      void add(const TmxIpPacket& packet);
      void request(State& state);
      void fail(const std::exception_ptr& error);
      void feed_loop();
      void recovery_loop();
      void request_loop();
      void on_timer(typename Timer::Result result);
  };

  template<typename PF, typename RF, typename TF>
  TmxIpClient(PF&&, RF&&, TF&&) -> TmxIpClient<
    std::remove_cvref_t<PF>, std::remove_cvref_t<RF>, std::remove_cvref_t<TF>>;

  template<typename P, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<P> PF, Beam::Initializes<R> RF,
    Beam::Initializes<T> TF>
  TmxIpClient<P, R, T>::TmxIpClient(
      PF&& protocol_client, RF&& recovery_client, TF&& timer)
      : m_protocol_client(std::forward<PF>(protocol_client)),
        m_recovery_client(std::forward<RF>(recovery_client)),
        m_timer(std::forward<TF>(timer)) {
    try {
      m_timer->get_publisher().monitor(m_tasks.get_slot<typename Timer::Result>(
        std::bind_front(&TmxIpClient::on_timer, this)));
      m_timer->start();
      m_routines.spawn(std::bind_front(&TmxIpClient::feed_loop, this));
      m_routines.spawn(std::bind_front(&TmxIpClient::recovery_loop, this));
      m_routines.spawn(std::bind_front(&TmxIpClient::request_loop, this));
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename P, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  TmxIpClient<P, R, T>::~TmxIpClient() {
    close();
  }

  template<typename P, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  StampMessage TmxIpClient<P, R, T>::read() {
    m_payload = m_messages.pop();
    return StampMessage::parse(
      std::string_view(m_payload.get_data(), m_payload.get_size()));
  }

  template<typename P, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, R, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    fail(std::make_exception_ptr(Beam::EndOfFileException()));
    m_protocol_client->close();
    m_recovery_client->close();
    m_tasks.close();
    m_tasks.wait();
    m_timer->cancel();
    m_routines.wait();
    m_open_state.close();
  }

  template<typename P, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, R, T>::add(const TmxIpPacket& packet) {
    Beam::with(m_state, [&] (auto& state) {
      if(state.m_is_finished) {
        return;
      }
      try {
        state.m_sequencer.add(packet);
        while(auto packet = state.m_sequencer.read()) {
          if(auto payload = state.m_builder.add(*packet)) {
            StampMessage::parse(
              std::string_view(payload->get_data(), payload->get_size()));
            m_messages.push(std::move(*payload));
          }
        }
        request(state);
      } catch(const std::exception&) {
        state.m_is_finished = true;
        throw;
      }
    });
  }

  template<typename P, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, R, T>::request(State& state) {
    if(!state.m_is_finished && state.m_request_state == RequestState::READY &&
        state.m_sequencer.get_gap()) {
      state.m_request_state = RequestState::PENDING;
      m_requests.push(true);
    }
  }

  template<typename P, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, R, T>::fail(const std::exception_ptr& error) {
    Beam::with(m_state, [&] (auto& state) {
      state.m_is_finished = true;
      m_messages.close(error);
      m_requests.close(error);
    });
  }

  template<typename P, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, R, T>::feed_loop() {
    try {
      while(m_open_state.is_open()) {
        auto packet = [&] () -> boost::optional<TmxIpPacket> {
          try {
            return m_protocol_client->read();
          } catch(const TmxIpParserException&) {
            return {};
          }
        }();
        if(packet) {
          if(is_heartbeat(packet->m_header)) {
            try {
              TmxIpHeartbeat::parse(*packet);
            } catch(const TmxIpParserException&) {
              continue;
            }
          }
          add(*packet);
        }
      }
    } catch(const std::exception&) {
      fail(std::current_exception());
    }
  }

  template<typename P, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, R, T>::recovery_loop() {
    try {
      while(m_open_state.is_open()) {
        add(m_recovery_client->read());
      }
    } catch(const std::exception&) {
      fail(std::current_exception());
    }
  }

  template<typename P, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, R, T>::request_loop() {
    try {
      while(m_requests.pop()) {
        auto maximum_count = m_recovery_client->get_maximum_count();
        auto range = Beam::with(m_state,
          [&] (auto& state) -> boost::optional<TmxIpRecoveryRequest> {
            if(state.m_is_finished) {
              return {};
            }
            auto gap = state.m_sequencer.get_gap();
            if(!gap) {
              state.m_request_state = RequestState::READY;
              return {};
            }
            if(maximum_count == 0) {
              state.m_request_state = RequestState::DEFERRED;
              return {};
            }
            auto count = std::min(gap->m_count, maximum_count);
            return TmxIpRecoveryRequest(
              gap->m_sequence, gap->m_sequence + count - 1);
          });
        if(!range) {
          continue;
        }
        auto result = m_recovery_client->request(*range);
        if(!result.m_is_acknowledged ||
            result.m_status != TmxIpRecoveryResponse::Status::ACCEPTED ||
            result.m_start_sequence == 0) {
          boost::throw_with_location(Beam::IOException(
            "TMX IP recovery failed: " + result.m_description));
        }
        Beam::with(m_state, [&] (auto& state) {
          state.m_request_state = RequestState::DEFERRED;
        });
      }
    } catch(const std::exception&) {
      fail(std::current_exception());
    }
  }

  template<typename P, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, R, T>::on_timer(typename Timer::Result result) {
    try {
      if(result == Timer::Result::CANCELED) {
        return;
      }
      if(result == Timer::Result::FAIL) {
        boost::throw_with_location(
          Beam::IOException("TMX IP recovery retry timer failed."));
      }
      auto is_finished = Beam::with(m_state, [&] (auto& state) {
        if(state.m_request_state == RequestState::DEFERRED) {
          state.m_request_state = RequestState::READY;
        }
        request(state);
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
