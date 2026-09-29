#ifndef NEXUS_SOUP_BIN_TCP_CLIENT_HPP
#define NEXUS_SOUP_BIN_TCP_CLIENT_HPP
#include <atomic>
#include <cstdint>
#include <functional>
#include <string_view>
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/SoupBinTcp/HeartbeatPackets.hpp"
#include "Nexus/SoupBinTcp/LoginPackets.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpPacket.hpp"

namespace Nexus {

  /**
   * Implements a client using the SoupBinTCP protocol.
   * @tparam C The Channel connected to the SoupBinTCP server.
   * @tparam T The type of Timer used for heartbeats and timeouts.
   */
  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class SoupBinTcpClient {
    public:

      /** The number of timer periods allowed for login or receive silence. */
      static constexpr auto TIMEOUT_TICKS = 10;

      /** The Channel connected to the SoupBinTCP server. */
      using Channel = Beam::dereference_t<C>;

      /** The type of Timer used for heartbeats and timeouts. */
      using Timer = Beam::dereference_t<T>;

      /**
       * Constructs a SoupBinTcpClient.
       * @param username The username.
       * @param password The password.
       * @param channel The Channel connected to the SoupBinTCP server.
       * @param timer A one-second timer for heartbeats and timeouts.
       */
      template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
      SoupBinTcpClient(std::string_view username, std::string_view password,
        CF&& channel, TF&& timer);

      /**
       * Constructs a SoupBinTcpClient.
       * @param username The username.
       * @param password The password.
       * @param session The existing session to log into.
       * @param sequence_number The next sequence number to receive from the
       *        server.
       * @param channel The Channel connected to the SoupBinTCP server.
       * @param timer A one-second timer for heartbeats and timeouts.
       */
      template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
      SoupBinTcpClient(std::string_view username, std::string_view password,
        std::string_view session, std::uint64_t sequence_number, CF&& channel,
        TF&& timer);

      ~SoupBinTcpClient();

      /** Returns the session accepted by the server. */
      const std::string& get_session() const;

      /** Returns the sequence number of the next sequenced data packet. */
      std::uint64_t get_sequence_number() const;

      /**
       * Reads the next packet, including control packets. Supports one reader.
       * The returned payload is valid until the next read or destruction.
       */
      SoupBinTcpPacket read();

      /** Closes the connection to the server. */
      void close();

    private:
      struct ActivityReader {
        typename Channel::Reader* m_reader;
        std::atomic_bool* m_is_receiving;

        bool poll() const;
        template<Beam::IsBuffer B>
        std::size_t read(Beam::Out<B> buffer, std::size_t size);
      };
      Beam::local_ptr_t<C> m_channel;
      Beam::local_ptr_t<T> m_timer;
      Beam::SharedBuffer m_buffer;
      std::string m_session;
      std::atomic_uint64_t m_sequence_number;
      std::atomic_bool m_is_logged_in;
      std::atomic_bool m_is_receiving;
      int m_ticks;
      Beam::Sync<std::exception_ptr> m_error;
      Beam::RoutineTaskQueue m_tasks;
      Beam::RoutineTaskQueue m_writes;
      Beam::OpenState m_open_state;

      SoupBinTcpClient(const SoupBinTcpClient&) = delete;
      SoupBinTcpClient& operator =(const SoupBinTcpClient&) = delete;
      void write(Beam::SharedBuffer buffer);
      void fail(const std::exception_ptr& error);
      void on_timer(typename Timer::Result result);
  };

  template<typename C, typename T>
  SoupBinTcpClient(std::string_view, std::string_view, C&&, T&&) ->
    SoupBinTcpClient<std::remove_cvref_t<C>, std::remove_cvref_t<T>>;

  template<typename C, typename T>
  SoupBinTcpClient(std::string_view, std::string_view, std::string_view,
    std::uint64_t, C&&, T&&) ->
      SoupBinTcpClient<std::remove_cvref_t<C>, std::remove_cvref_t<T>>;

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
  SoupBinTcpClient<C, T>::SoupBinTcpClient(std::string_view username,
    std::string_view password, CF&& channel, TF&& timer)
    : SoupBinTcpClient(username, password, {}, 1, std::forward<CF>(channel),
        std::forward<TF>(timer)) {}

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
  SoupBinTcpClient<C, T>::SoupBinTcpClient(std::string_view username,
      std::string_view password, std::string_view session,
      std::uint64_t sequence_number, CF&& channel, TF&& timer)
      try : m_channel(std::forward<CF>(channel)),
            m_timer(std::forward<TF>(timer)),
            m_sequence_number(0),
            m_is_logged_in(false),
            m_is_receiving(false),
            m_ticks(0) {
    try {
      m_timer->get_publisher().monitor(m_tasks.get_slot<typename Timer::Result>(
        std::bind_front(&SoupBinTcpClient::on_timer, this)));
      m_timer->start();
      make_login_request_packet(
        username, password, session, sequence_number, Beam::out(m_buffer));
      write(m_buffer);
      auto login_response = SoupBinTcpPacket();
      while(true) {
        login_response = read();
        if(login_response.m_type != '+') {
          break;
        }
      }
      if(login_response.m_type == LoginRejectedPacket::TYPE) {
        auto login_rejected_packet =
          parse_login_rejected_packet(login_response);
        if(login_rejected_packet.m_reason == 'A') {
          boost::throw_with_location(Beam::ConnectException("Not authorized."));
        } else if(login_rejected_packet.m_reason == 'S') {
          boost::throw_with_location(
            Beam::ConnectException("Session unavailable."));
        } else {
          boost::throw_with_location(
            Beam::ConnectException("Unable to login."));
        }
      } else if(login_response.m_type != LoginAcceptedPacket::TYPE) {
        boost::throw_with_location(
          Beam::ConnectException("Unrecognized login response."));
      }
      auto login_accepted_packet = parse_login_accepted_packet(login_response);
      m_session = login_accepted_packet.m_session;
      m_sequence_number = login_accepted_packet.m_sequence_number;
      m_is_logged_in = true;
    } catch(const std::exception&) {
      close();
      throw;
    }
  } catch(const std::exception&) {
    Beam::throw_nested_with_location(
      Beam::ConnectException("SoupBinTCP client failed to connect."));
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  SoupBinTcpClient<C, T>::~SoupBinTcpClient() {
    close();
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  const std::string& SoupBinTcpClient<C, T>::get_session() const {
    return m_session;
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  std::uint64_t SoupBinTcpClient<C, T>::get_sequence_number() const {
    return m_sequence_number;
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  SoupBinTcpPacket SoupBinTcpClient<C, T>::read() {
    try {
      if(auto error = m_error.load()) {
        std::rethrow_exception(error);
      }
      reset(m_buffer);
      auto reader = ActivityReader(&m_channel->get_reader(), &m_is_receiving);
      auto packet = read_packet(reader, Beam::out(m_buffer));
      if(auto error = m_error.load()) {
        std::rethrow_exception(error);
      }
      if(m_is_logged_in && packet.m_type == 'S') {
        ++m_sequence_number;
      }
      return packet;
    } catch(const std::exception&) {
      fail(std::current_exception());
      Beam::throw_nested_with_location(
        Beam::IOException("Failed to read SoupBinTCP packet."));
    }
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void SoupBinTcpClient<C, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_channel->get_connection().close();
    m_tasks.close();
    m_tasks.wait();
    m_timer->cancel();
    m_writes.close();
    m_writes.wait();
    m_open_state.close();
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  bool SoupBinTcpClient<C, T>::ActivityReader::poll() const {
    return m_reader->poll();
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::IsBuffer B>
  std::size_t SoupBinTcpClient<C, T>::ActivityReader::read(
      Beam::Out<B> buffer, std::size_t size) {
    auto count = m_reader->read(buffer, size);
    if(count != 0) {
      *m_is_receiving = true;
    }
    return count;
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void SoupBinTcpClient<C, T>::write(Beam::SharedBuffer buffer) {
    m_writes.push([=, this] {
      if(!m_open_state.is_open()) {
        return;
      }
      try {
        m_channel->get_writer().write(buffer);
      } catch(const std::exception&) {
        fail(std::current_exception());
      }
    });
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void SoupBinTcpClient<C, T>::fail(const std::exception_ptr& error) {
    auto is_first = Beam::with(m_error, [&] (auto& current) {
      if(current) {
        return false;
      }
      current = error;
      return true;
    });
    if(is_first) {
      m_channel->get_connection().close();
    }
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void SoupBinTcpClient<C, T>::on_timer(typename Timer::Result result) {
    if(result == Timer::Result::CANCELED || !m_open_state.is_open()) {
      return;
    }
    if(m_error.load()) {
      return;
    }
    try {
      if(result == Timer::Result::FAIL) {
        fail(std::make_exception_ptr(
          Beam::IOException("SoupBinTCP timer failed.")));
        return;
      }
      if(m_is_logged_in && m_is_receiving.exchange(false)) {
        m_ticks = 0;
      } else {
        ++m_ticks;
      }
      if(m_ticks >= TIMEOUT_TICKS) {
        fail(
          std::make_exception_ptr(Beam::IOException("SoupBinTCP timed out.")));
        return;
      }
      if(m_is_logged_in) {
        auto buffer = Beam::SharedBuffer();
        make_client_heartbeat_packet(Beam::out(buffer));
        write(std::move(buffer));
      }
      m_timer->start();
    } catch(const std::exception&) {
      fail(std::current_exception());
    }
  }
}

#endif
