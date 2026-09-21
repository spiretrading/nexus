#ifndef TMX_IP_RECOVERY_CLIENT_HPP
#define TMX_IP_RECOVERY_CLIENT_HPP
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <stop_token>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/Reader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/StaticBuffer.hpp>
#include <Beam/Queues/CallbackQueueWriter.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Routines/Async.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/Timer.hpp>
#include "TmxIpMarketDataFeedClient/TmxIpProtocolClient.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpRecoveryMessages.hpp"

namespace Nexus {

  /** The server's result and packet counts for a recovery request. */
  struct TmxIpRecoveryResult {

    /** Whether the server acknowledged the request. */
    bool m_is_acknowledged;

    /** The server's request status. */
    TmxIpRecoveryResponse::Status m_status;

    /** The first sequence offered, or zero when no data is available. */
    std::uint32_t m_start_sequence;

    /** The last sequence offered, or zero when no data is available. */
    std::uint32_t m_end_sequence;

    /** The number of packets requested, as reported by the trailer. */
    std::uint32_t m_requested_count;

    /** The number sent by the server, which may exceed the number received. */
    std::uint32_t m_sent_count;

    /** The acknowledgement description or trailer status. */
    std::string m_description;
  };

  /** Concept satisfied by clients recovering a TMX IP stream. */
  template<typename T>
  concept IsTmxIpRecoveryClient = requires(T& t) {
    { std::as_const(t).get_maximum_count() } -> std::same_as<std::uint32_t>;
    { t.request(std::declval<const TmxIpRecoveryRequest&>()) } ->
      std::same_as<TmxIpRecoveryResult>;
    { t.read() } -> std::same_as<TmxIpPacket>;
    { t.request(std::declval<const TmxIpRecoveryRequest&>(),
        std::uint64_t()) } -> std::same_as<TmxIpRecoveryResult>;
    { t.read(std::declval<Beam::Out<std::uint64_t>>()) } ->
      std::same_as<TmxIpPacket>;
    { t.reset(std::uint64_t()) } -> std::same_as<void>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Recovers missing packets from a TMX IP stream.
   * Requests on this client are serialized. Sharing a customer allocation
   * across clients requires external coordination.
   * @tparam C The recovery request channel type.
   * @tparam P The protocol client type receiving recovered packets.
   * @tparam T The recovery deadline timer type.
   */
  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class TmxIpRecoveryClient {
    public:

      /** The recovery request channel type. */
      using Channel = C;

      /** The protocol client type receiving recovered packets. */
      using ProtocolClient = Beam::dereference_t<P>;

      /** The recovery deadline timer type. */
      using Timer = Beam::dereference_t<T>;

      /**
       * Constructs a recovery client.
       * @param connection_builder Opens a new connection per request.
       * @param protocol_client Receives this stream's recovered packets.
       * @param timer The timer measuring each recovery deadline.
       */
      template<Beam::Initializes<P> PF, Beam::Initializes<T> TF>
      TmxIpRecoveryClient(std::function<std::shared_ptr<Channel> (
          std::stop_token)> connection_builder, PF&& protocol_client,
        TF&& timer);

      ~TmxIpRecoveryClient();

      /** Returns the advertised request limit, or 10,000 before a heartbeat. */
      std::uint32_t get_maximum_count() const;

      /**
       * Requests an inclusive range.
       * Recovered packets become available through read while this waits.
       * Throws on recovery errors, transport failures, and expired deadlines.
       */
      TmxIpRecoveryResult request(const TmxIpRecoveryRequest& request);

      /** Requests a range for a local session, rejecting expired sessions. */
      TmxIpRecoveryResult request(
        const TmxIpRecoveryRequest& request, std::uint64_t session);

      /**
       * Reads a recovered packet, valid until the next read or destruction.
       * A completed request does not close this stream.
       */
      TmxIpPacket read();

      /**
       * Reads a recovered packet and its request's local session identifier.
       * The packet remains valid until the next read or destruction.
       */
      TmxIpPacket read(Beam::Out<std::uint64_t> session);

      /**
       * Starts a local session and asynchronously cancels active recovery.
       * Keeps the delivery channel open. Already queued packets retain their
       * original session identifiers.
       */
      void reset(std::uint64_t session);

      /** Cancels recovery and closes the protocol client. */
      void close();

    private:
      struct Entry {
        TmxIpHeader m_header;
        Beam::SharedBuffer m_buffer;
        std::uint64_t m_session = 0;
      };
      struct Operation {
        struct State {
          std::shared_ptr<Channel> m_channel;
          std::exception_ptr m_exception;
        };
        Beam::Sync<State> m_state;
        std::stop_source m_stop_source;
        Beam::Queue<Entry> m_packets;
        std::uint64_t m_session = 0;
      };
      struct State {
        std::shared_ptr<Operation> m_operation;
        std::exception_ptr m_exception;
        std::uint64_t m_session = 0;
      };
      mutable Beam::Mutex m_mutex;
      std::function<std::shared_ptr<Channel> (std::stop_token)>
        m_connection_builder;
      Beam::local_ptr_t<P> m_protocol_client;
      Beam::local_ptr_t<T> m_timer;
      std::atomic_uint32_t m_maximum_count;
      Beam::Sync<State> m_state;
      Beam::Queue<Entry> m_packets;
      Entry m_packet;
      Beam::RoutineTaskQueue m_tasks;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      static void cancel(const std::shared_ptr<Operation>& operation,
        const std::exception_ptr& exception);
      static void check(const Operation& operation);
      TmxIpRecoveryClient(const TmxIpRecoveryClient&) = delete;
      TmxIpRecoveryClient& operator =(const TmxIpRecoveryClient&) = delete;
      TmxIpRecoveryResult send_request(Operation& operation,
        const Beam::StaticBuffer<TmxIpRecoveryRequest::LENGTH>& request);
      void receive_recovery(
        Operation& operation, TmxIpRecoveryResult& result);
      void finish(Operation& operation,
        Beam::QueueWriter<typename Timer::Result>& slot);
      void read_loop();
  };

  template<typename C, typename PF, typename TF>
  TmxIpRecoveryClient(std::function<std::shared_ptr<C> (std::stop_token)>,
    PF&&, TF&&) ->
      TmxIpRecoveryClient<C, std::remove_cvref_t<PF>, std::remove_cvref_t<TF>>;

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<P> PF, Beam::Initializes<T> TF>
  TmxIpRecoveryClient<C, P, T>::TmxIpRecoveryClient(
      std::function<std::shared_ptr<Channel> (std::stop_token)>
        connection_builder, PF&& protocol_client,
      TF&& timer)
      : m_connection_builder(std::move(connection_builder)),
        m_protocol_client(std::forward<PF>(protocol_client)),
        m_timer(std::forward<TF>(timer)),
        m_maximum_count(10000) {
    try {
      m_read_loop =
        Beam::spawn(std::bind_front(&TmxIpRecoveryClient::read_loop, this));
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  TmxIpRecoveryClient<C, P, T>::~TmxIpRecoveryClient() {
    close();
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  std::uint32_t TmxIpRecoveryClient<C, P, T>::get_maximum_count() const {
    return m_maximum_count;
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  TmxIpRecoveryResult TmxIpRecoveryClient<C, P, T>::request(
      const TmxIpRecoveryRequest& request) {
    auto session = Beam::with(m_state, [] (const auto& state) {
      return state.m_session;
    });
    return this->request(request, session);
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  TmxIpRecoveryResult TmxIpRecoveryClient<C, P, T>::request(
      const TmxIpRecoveryRequest& request, std::uint64_t session) {
    auto buffer = Beam::StaticBuffer<TmxIpRecoveryRequest::LENGTH>();
    request.encode(Beam::out(buffer));
    auto operation = std::make_shared<Operation>();
    operation->m_session = session;
    auto slot = Beam::callback<typename Timer::Result>(
      [=, this] (const auto& result) {
        if(result != Timer::Result::CANCELED) {
          m_tasks.push([=] {
            cancel(operation, std::make_exception_ptr(Beam::IOException(
              "TMX IP recovery deadline failed or expired.")));
          });
        }
      });
    auto lock = std::lock_guard(m_mutex);
    if(request.m_end_sequence - request.m_start_sequence + 1 >
        get_maximum_count()) {
      boost::throw_with_location(
        Beam::IOException("TMX IP recovery request exceeds the limit."));
    }
    Beam::with(m_state, [&] (auto& state) {
      if(state.m_exception) {
        std::rethrow_exception(state.m_exception);
      }
      if(!m_open_state.is_open()) {
        boost::throw_with_location(Beam::EndOfFileException());
      }
      if(session != state.m_session) {
        boost::throw_with_location(
          Beam::IOException("TMX IP recovery session expired."));
      }
      state.m_operation = operation;
    });
    try {
      m_timer->get_publisher().monitor(slot);
      m_timer->start();
      auto result = send_request(*operation, buffer);
      receive_recovery(*operation, result);
      finish(*operation, *slot);
      check(*operation);
      return result;
    } catch(const std::exception&) {
      finish(*operation, *slot);
      throw;
    }
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  TmxIpPacket TmxIpRecoveryClient<C, P, T>::read() {
    auto session = std::uint64_t();
    return read(Beam::out(session));
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  TmxIpPacket TmxIpRecoveryClient<C, P, T>::read(
      Beam::Out<std::uint64_t> session) {
    m_packet = m_packets.pop();
    *session = m_packet.m_session;
    return TmxIpPacket(m_packet.m_header,
      std::string_view(m_packet.m_buffer.get_data() +
        m_packet.m_header.m_service.size(), m_packet.m_buffer.get_size() -
        m_packet.m_header.m_service.size()));
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpRecoveryClient<C, P, T>::reset(std::uint64_t session) {
    Beam::with(m_state, [&] (auto& state) {
      state.m_session = session;
      if(auto operation = state.m_operation) {
        m_tasks.push([=] {
          cancel(operation, std::make_exception_ptr(
            Beam::IOException("TMX IP recovery session expired.")));
        });
      }
    });
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpRecoveryClient<C, P, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_packets.close();
    auto operation = Beam::with(m_state, [] (const auto& state) {
      return state.m_operation;
    });
    if(operation) {
      cancel(operation, std::make_exception_ptr(Beam::EndOfFileException()));
    }
    m_protocol_client->close();
    m_read_loop.wait();
    auto lock = std::lock_guard(m_mutex);
    m_tasks.close();
    m_tasks.wait();
    m_open_state.close();
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpRecoveryClient<C, P, T>::cancel(
      const std::shared_ptr<Operation>& operation,
      const std::exception_ptr& exception) {
    auto channel = Beam::with(operation->m_state, [&] (auto& state) {
      if(!state.m_exception) {
        state.m_exception = exception;
      }
      return std::exchange(state.m_channel, {});
    });
    operation->m_packets.close(exception);
    operation->m_stop_source.request_stop();
    if(channel) {
      channel->get_connection().close();
    }
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpRecoveryClient<C, P, T>::check(const Operation& operation) {
    Beam::with(operation.m_state, [] (const auto& state) {
      if(state.m_exception) {
        std::rethrow_exception(state.m_exception);
      }
    });
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  TmxIpRecoveryResult TmxIpRecoveryClient<C, P, T>::send_request(
      Operation& operation,
      const Beam::StaticBuffer<TmxIpRecoveryRequest::LENGTH>& request) {
    auto channel = m_connection_builder(operation.m_stop_source.get_token());
    auto is_installed = Beam::with(operation.m_state, [&] (auto& state) {
      if(state.m_exception) {
        return false;
      }
      state.m_channel = channel;
      return true;
    });
    if(!is_installed) {
      channel->get_connection().close();
      check(operation);
    }
    channel->get_writer().write(request);
    auto buffer = Beam::StaticBuffer<TmxIpRecoveryResponse::LENGTH>();
    Beam::read_exact(
      channel->get_reader(), Beam::out(buffer), TmxIpRecoveryResponse::LENGTH);
    check(operation);
    auto response = TmxIpRecoveryResponse::parse(
      std::string_view(buffer.get_data(), buffer.get_size()));
    channel->get_connection().close();
    return TmxIpRecoveryResult(response.m_is_acknowledged, response.m_status,
      response.m_start_sequence, response.m_end_sequence, 0, 0,
      std::string(response.m_description));
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpRecoveryClient<C, P, T>::receive_recovery(
      Operation& operation, TmxIpRecoveryResult& result) {
    if(!result.m_is_acknowledged ||
        result.m_status != TmxIpRecoveryResponse::Status::ACCEPTED ||
        result.m_start_sequence == 0) {
      return;
    }
    auto is_current = false;
    while(true) {
      auto entry = operation.m_packets.pop();
      check(operation);
      auto packet = TmxIpPacket(entry.m_header,
        std::string_view(entry.m_buffer.get_data() +
          entry.m_header.m_service.size(), entry.m_buffer.get_size() -
          entry.m_header.m_service.size()));
      if(auto sequence = packet.m_header.m_sequence) {
        if(*sequence >= result.m_start_sequence &&
            *sequence <= result.m_end_sequence) {
          m_packets.push(std::move(entry));
        }
      } else if(packet.m_payload.starts_with(TmxIpRecoveryStart::TYPE)) {
        auto start = TmxIpRecoveryStart::parse(packet);
        is_current = start.m_start_sequence == result.m_start_sequence &&
          start.m_end_sequence == result.m_end_sequence;
      } else if(packet.m_payload.starts_with(TmxIpRecoveryEnd::TYPE)) {
        auto end = TmxIpRecoveryEnd::parse(packet);
        if(!is_current || end.m_requested_count !=
            result.m_end_sequence - result.m_start_sequence + 1) {
          continue;
        }
        result.m_requested_count = end.m_requested_count;
        result.m_sent_count = end.m_sent_count;
        result.m_description = end.m_status;
        break;
      } else if(packet.m_payload.starts_with(TmxIpRecoveryError::TYPE)) {
        auto error = TmxIpRecoveryError::parse(packet);
        if(!is_current) {
          continue;
        }
        boost::throw_with_location(Beam::IOException(std::format(
          "TMX IP recovery error: {}", error.m_description)));
      }
    }
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpRecoveryClient<C, P, T>::finish(
      Operation& operation, Beam::QueueWriter<typename Timer::Result>& slot) {
    m_timer->cancel();
    slot.close();
    auto completion = Beam::Async<void>();
    m_tasks.push([&] {
      completion.get_eval().set();
    });
    completion.get();
    operation.m_packets.close();
    operation.m_stop_source.request_stop();
    auto channel = Beam::with(operation.m_state, [] (auto& state) {
      return std::exchange(state.m_channel, {});
    });
    if(channel) {
      channel->get_connection().close();
    }
    Beam::with(m_state, [&] (auto& state) {
      state.m_operation.reset();
    });
  }

  template<Beam::IsChannel C, typename P, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpRecoveryClient<C, P, T>::read_loop() {
    try {
      while(true) {
        try {
          auto packet = m_protocol_client->read();
          if(is_recovery_control(packet.m_header) &&
              packet.m_payload.starts_with(TmxIpRecoveryHeartbeat::TYPE)) {
            m_maximum_count =
              TmxIpRecoveryHeartbeat::parse(packet).m_maximum_count;
            continue;
          }
          auto operation = Beam::with(m_state, [] (const auto& state) {
            return state.m_operation;
          });
          if(!operation) {
            continue;
          }
          auto entry = Entry(packet.m_header, Beam::SharedBuffer(
            packet.m_header.m_service.size() + packet.m_payload.size()),
            operation->m_session);
          entry.m_buffer.write(0, packet.m_header.m_service.data(),
            packet.m_header.m_service.size());
          entry.m_buffer.write(packet.m_header.m_service.size(),
            packet.m_payload.data(), packet.m_payload.size());
          entry.m_header.m_service = std::string_view(
            entry.m_buffer.get_data(), packet.m_header.m_service.size());
          try {
            operation->m_packets.push(std::move(entry));
          } catch(const std::exception&) {}
        } catch(const TmxIpParserException&) {}
      }
    } catch(const std::exception&) {
      auto exception = std::current_exception();
      auto operation = Beam::with(m_state, [&] (auto& state) {
        state.m_exception = exception;
        return state.m_operation;
      });
      m_packets.close(exception);
      if(operation) {
        cancel(operation, exception);
      }
    }
  }
}

#endif
