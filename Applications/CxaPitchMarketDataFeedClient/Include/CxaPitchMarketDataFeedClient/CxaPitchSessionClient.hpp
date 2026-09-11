#ifndef CXA_PITCH_SESSION_CLIENT_HPP
#define CXA_PITCH_SESSION_CLIENT_HPP
#include <atomic>
#include <concepts>
#include <exception>
#include <functional>
#include <stop_token>
#include <type_traits>
#include <utility>
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/IOException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/StaticBuffer.hpp>
#include <Beam/IO/SyncWriter.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <boost/throw_exception.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"
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
   * @tparam C The type of Channel connected to the server.
   * @tparam T The type of Timer used to schedule heartbeats.
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
       * @param stop_token Cancels a pending login.
       */
      template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
      CxaPitchSessionClient(CxaPitchLogin login, CF&& channel, TF&& timer,
        std::stop_token stop_token);

      ~CxaPitchSessionClient();

      /**
       * Reads the next message sent by the server.
       * The returned payload is valid until the next read or destruction.
       */
      CxaPitchMessage read();

      /**
       * Sends a message to the server.
       * @param message The message to send.
       */
      template<IsCxaPitchSessionMessage M>
      void write(const M& message);

      /** Closes the connection to the server. */
      void close();

    private:
      Beam::local_ptr_t<C> m_channel;
      Beam::SyncWriter<typename Channel::Writer*> m_writer;
      Beam::local_ptr_t<T> m_timer;
      Beam::SharedBuffer m_buffer;
      std::string_view m_payload;
      std::atomic_bool m_is_receiving;
      std::atomic_bool m_is_logged_in;
      int m_silence;
      Beam::RoutineTaskQueue m_tasks;
      Beam::OpenState m_open_state;

      CxaPitchSessionClient(const CxaPitchSessionClient&) = delete;
      CxaPitchSessionClient& operator =(const CxaPitchSessionClient&) = delete;
      void log_in(const CxaPitchLogin& login);
      void read_until(std::size_t size);
      void write_heartbeat();
      void on_timer(typename Timer::Result result);
  };

  template<typename C, typename T>
  CxaPitchSessionClient(CxaPitchLogin, C&&, T&&) ->
    CxaPitchSessionClient<std::remove_cvref_t<C>, std::remove_cvref_t<T>>;

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
            m_writer(&m_channel->get_writer()),
            m_timer(std::forward<TF>(timer)),
            m_is_receiving(false),
            m_is_logged_in(false),
            m_silence(0) {
    try {
      m_timer->get_publisher().monitor(m_tasks.get_slot<typename Timer::Result>(
        std::bind_front(&CxaPitchSessionClient::on_timer, this)));
      auto cancellation = std::stop_callback(stop_token, [&] {
        m_channel->get_connection().close();
      });
      log_in(login);
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
      while(m_payload.empty()) {
        reset(m_buffer);
        read_until(CxaPitchHeader::LENGTH);
        auto header = CxaPitchHeader::parse(
          std::string_view(m_buffer.get_data(), m_buffer.get_size()));
        if(header.m_length < CxaPitchHeader::LENGTH) {
          boost::throw_with_location(CxaPitchParserException(
            "Sequenced unit header length out of range."));
        }
        read_until(header.m_length);
        auto source =
          std::string_view(m_buffer.get_data(), m_buffer.get_size());
        CxaPitchBlock::parse(source);
        m_payload = source.substr(CxaPitchHeader::LENGTH);
      }
      auto message = CxaPitchMessage::parse(m_payload);
      m_payload.remove_prefix(message.m_length);
      return message;
    }, Beam::IOException("Failed to read from the CXA PITCH server."));
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<IsCxaPitchSessionMessage M>
  void CxaPitchSessionClient<C, T>::write(const M& message) {
    Beam::try_or_nest([&] {
      auto buffer = Beam::StaticBuffer<CxaPitchHeader::LENGTH + M::LENGTH>();
      auto header = CxaPitchHeader(static_cast<std::uint16_t>(
        CxaPitchHeader::LENGTH + M::LENGTH), 1, 0, 0);
      header.encode(Beam::out(buffer));
      message.encode(Beam::out(buffer));
      m_writer.write(buffer);
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
    m_tasks.close();
    m_tasks.wait();
    m_open_state.close();
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchSessionClient<C, T>::log_in(const CxaPitchLogin& login) {
    write(login);
    m_timer->start();
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
  void CxaPitchSessionClient<C, T>::read_until(std::size_t size) {
    auto available = m_buffer.get_size();
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
  void CxaPitchSessionClient<C, T>::write_heartbeat() {
    auto buffer = Beam::StaticBuffer<CxaPitchHeader::LENGTH>();
    auto header = CxaPitchHeader(CxaPitchHeader::LENGTH, 0, 0, 0);
    header.encode(Beam::out(buffer));
    m_writer.write(buffer);
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchSessionClient<C, T>::on_timer(typename Timer::Result result) {
    if(result == Timer::Result::CANCELED || !m_open_state.is_open()) {
      return;
    }
    try {
      if(result == Timer::Result::FAIL) {
        m_channel->get_connection().close();
        return;
      }
      if(m_is_logged_in && m_is_receiving.exchange(false)) {
        m_silence = 0;
      } else {
        ++m_silence;
      }
      if(m_silence == SILENT_HEARTBEAT_LIMIT) {
        m_channel->get_connection().close();
        return;
      }
      write_heartbeat();
      m_timer->start();
    } catch(const std::exception&) {
      m_channel->get_connection().close();
    }
  }
}

#endif
