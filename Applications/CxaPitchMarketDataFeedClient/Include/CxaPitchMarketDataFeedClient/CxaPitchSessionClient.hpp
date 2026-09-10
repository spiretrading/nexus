#ifndef CXA_PITCH_SESSION_CLIENT_HPP
#define CXA_PITCH_SESSION_CLIENT_HPP
#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <stop_token>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/IOException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/Reader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/throw_exception.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchHeader.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSessionMessages.hpp"

namespace Nexus {

  /** Concept satisfied by types implementing a CXA PITCH server session. */
  template<typename T>
  concept IsCxaPitchSessionClient = requires(T& t) {
    { t.read() } -> std::same_as<CxaPitchMessage>;
    { t.write(std::declval<const CxaPitchGapRequest&>()) } ->
      std::same_as<void>;
    { t.write(std::declval<const CxaPitchSpinRequest&>()) } ->
      std::same_as<void>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Maintains a logged in session with a CXA PITCH gap request proxy or spin
   * server.
   * @param <C> The type of Channel connected to the server.
   * @param <T> The type of Timer used to schedule heartbeats.
   */
  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class CxaPitchSessionClient {
    public:

      /** The type of channel connected to the server. */
      using Channel = Beam::dereference_t<C>;

      /** The type of timer used to schedule heartbeats. */
      using Timer = Beam::dereference_t<T>;

      /** The number of heartbeat periods to tolerate without progress. */
      static constexpr auto SILENT_HEARTBEAT_LIMIT = 10;

      /**
       * Constructs a CxaPitchSessionClient.
       * @param login The credentials to log in with.
       * @param channel The channel connected to the server.
       * @param timer The timer measuring the period between heartbeats.
       */
      template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
      CxaPitchSessionClient(CxaPitchLogin login, CF&& channel, TF&& timer);

      /**
       * Constructs a session whose login can be cancelled.
       * @param login The credentials to log in with.
       * @param channel The channel connected to the server.
       * @param timer The timer measuring the period between heartbeats.
       * @param stop_token Cancels a pending login by closing the channel.
       */
      template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
      CxaPitchSessionClient(CxaPitchLogin login, CF&& channel, TF&& timer,
        std::stop_token stop_token);

      ~CxaPitchSessionClient();

      /** Reads the next message sent by the server. */
      CxaPitchMessage read();

      /**
       * Sends a message to the server.
       * @param message The message to send.
       */
      template<typename M>
      void write(const M& message);

      /** Closes the connection to the server. */
      void close();

    private:
      mutable Beam::Mutex m_mutex;
      Beam::local_ptr_t<C> m_channel;
      Beam::local_ptr_t<T> m_timer;
      Beam::SharedBuffer m_buffer;
      std::size_t m_position;
      std::size_t m_end;
      std::uint8_t m_remaining;
      std::atomic_bool m_is_receiving;
      std::atomic_bool m_is_logged_in;
      Beam::SharedBuffer m_write_buffer;
      Beam::RoutineHandler m_timeout_loop;
      Beam::RoutineHandler m_heartbeat_loop;
      std::shared_ptr<Beam::Queue<Beam::Timer::Result>> m_timer_queue;
      std::shared_ptr<Beam::Queue<Beam::Timer::Result>> m_heartbeat_queue;
      Beam::OpenState m_open_state;

      CxaPitchSessionClient(const CxaPitchSessionClient&) = delete;
      CxaPitchSessionClient& operator =(const CxaPitchSessionClient&) = delete;
      void log_in(const CxaPitchLogin& login);
      void compact();
      void fill(std::size_t size);
      void write_header(std::uint8_t count, std::uint16_t length);
      void write_heartbeat();
      void timeout_loop();
      void heartbeat_loop();
  };

  template<typename C, typename T>
  CxaPitchSessionClient(CxaPitchLogin, C&&, T&&) -> CxaPitchSessionClient<
    std::remove_cvref_t<C>, std::remove_cvref_t<T>>;

  template<typename C, typename T>
  CxaPitchSessionClient(CxaPitchLogin, C&&, T&&, std::stop_token) ->
    CxaPitchSessionClient<std::remove_cvref_t<C>, std::remove_cvref_t<T>>;

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
  CxaPitchSessionClient<C, T>::CxaPitchSessionClient(
      CxaPitchLogin login, CF&& channel, TF&& timer)
    : CxaPitchSessionClient(std::move(login), std::forward<CF>(channel),
        std::forward<TF>(timer), std::stop_token()) {}

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
  CxaPitchSessionClient<C, T>::CxaPitchSessionClient(CxaPitchLogin login,
      CF&& channel, TF&& timer, std::stop_token stop_token)
      try : m_channel(std::forward<CF>(channel)),
            m_timer(std::forward<TF>(timer)),
            m_position(0),
            m_end(0),
            m_remaining(0),
            m_is_receiving(false),
            m_is_logged_in(false),
            m_timer_queue(std::make_shared<Beam::Queue<Beam::Timer::Result>>()),
            m_heartbeat_queue(
              std::make_shared<Beam::Queue<Beam::Timer::Result>>()) {
    try {
      m_timer->get_publisher().monitor(m_timer_queue);
      m_timer->get_publisher().monitor(m_heartbeat_queue);
      {
        auto cancellation = std::stop_callback(stop_token, [&] {
          m_channel->get_connection().close();
        });
        log_in(login);
      }
    } catch(const std::exception&) {
      close();
      throw;
    }
  } catch(const std::exception&) {
    Beam::throw_nested_with_location(Beam::ConnectException(
      "Failed to initialize the CXA PITCH session client."));
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  CxaPitchSessionClient<C, T>::~CxaPitchSessionClient() {
    close();
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  CxaPitchMessage CxaPitchSessionClient<C, T>::read() {
    return Beam::try_or_nest([&] {
      while(m_remaining == 0) {
        m_position = m_end;
        compact();
        fill(CxaPitchHeader::LENGTH);
        auto header = CxaPitchHeader::parse(std::string_view(
          m_buffer.get_data() + m_position, m_buffer.get_size() - m_position));
        if(header.m_length < CxaPitchHeader::LENGTH) {
          boost::throw_with_location(CxaPitchParserException(
            "Sequenced unit header length out of range."));
        }
        fill(header.m_length);
        CxaPitchBlock::parse(
          std::string_view(m_buffer.get_data() + m_position, header.m_length));
        m_end = m_position + header.m_length;
        m_position += CxaPitchHeader::LENGTH;
        m_remaining = header.m_count;
      }
      auto message = CxaPitchMessage::parse(std::string_view(
        m_buffer.get_data() + m_position, m_end - m_position));
      m_position += message.m_length;
      --m_remaining;
      return message;
    }, Beam::IOException("Failed to read from the CXA PITCH server."));
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<typename M>
  void CxaPitchSessionClient<C, T>::write(const M& message) {
    Beam::try_or_nest([&] {
      auto lock = boost::lock_guard(m_mutex);
      reset(m_write_buffer);
      write_header(1, static_cast<std::uint16_t>(M::LENGTH));
      message.encode(Beam::out(m_write_buffer));
      m_channel->get_writer().write(m_write_buffer);
    }, Beam::IOException("Failed to write to the CXA PITCH server."));
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchSessionClient<C, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_channel->get_connection().close();
    m_timer->cancel();
    m_timer_queue->close();
    m_heartbeat_queue->close();
    m_timeout_loop.wait();
    m_heartbeat_loop.wait();
    m_open_state.close();
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchSessionClient<C, T>::log_in(const CxaPitchLogin& login) {
    m_timer->start();
    m_timeout_loop =
      Beam::spawn(std::bind_front(&CxaPitchSessionClient::timeout_loop, this));
    write(login);
    m_heartbeat_loop = Beam::spawn(
      std::bind_front(&CxaPitchSessionClient::heartbeat_loop, this));
    auto response = read();
    if(response.m_type != CxaPitchLoginResponse::TYPE) {
      boost::throw_with_location(Beam::ConnectException(
        "Received an unexpected message while logging in."));
    }
    auto status = CxaPitchLoginResponse::parse(response).m_status;
    if(status != CxaPitchLoginResponse::ACCEPTED) {
      boost::throw_with_location(Beam::ConnectException(
        "The CXA PITCH server rejected the login with status " +
          std::string(1, status) + "."));
    }
    m_is_logged_in = true;
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchSessionClient<C, T>::compact() {
    if(m_position == 0) {
      return;
    }
    if(m_position == m_buffer.get_size()) {
      reset(m_buffer);
    } else {
      m_buffer = Beam::SharedBuffer(
        m_buffer.get_data() + m_position, m_buffer.get_size() - m_position);
    }
    m_position = 0;
    m_end = 0;
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchSessionClient<C, T>::fill(std::size_t size) {
    auto available = m_buffer.get_size() - m_position;
    if(available >= size) {
      return;
    }
    Beam::read_exact(
      m_channel->get_reader(), Beam::out(m_buffer), size - available);
    m_is_receiving = true;
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchSessionClient<C, T>::write_header(
      std::uint8_t count, std::uint16_t length) {
    auto header = CxaPitchHeader();
    header.m_length =
      static_cast<std::uint16_t>(CxaPitchHeader::LENGTH + length);
    header.m_count = count;
    header.m_unit = 0;
    header.m_sequence = 0;
    header.encode(Beam::out(m_write_buffer));
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchSessionClient<C, T>::write_heartbeat() {
    auto lock = boost::lock_guard(m_mutex);
    reset(m_write_buffer);
    write_header(0, 0);
    m_channel->get_writer().write(m_write_buffer);
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchSessionClient<C, T>::timeout_loop() {
    auto silence = 0;
    try {
      while(m_open_state.is_open()) {
        if(m_timer_queue->pop() != Beam::Timer::Result::EXPIRED) {
          break;
        }
        if(m_is_logged_in.load() && m_is_receiving.exchange(false)) {
          silence = 0;
        } else if(++silence == SILENT_HEARTBEAT_LIMIT) {
          m_channel->get_connection().close();
          break;
        }
        m_timer->start();
      }
    } catch(const std::exception&) {
      return;
    }
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchSessionClient<C, T>::heartbeat_loop() {
    try {
      while(m_open_state.is_open()) {
        if(m_heartbeat_queue->pop() != Beam::Timer::Result::EXPIRED) {
          break;
        }
        write_heartbeat();
      }
    } catch(const std::exception&) {
      return;
    }
  }
}

#endif
