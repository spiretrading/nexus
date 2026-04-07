#ifndef NEXUS_SOUP_BIN_TCP_CLIENT_HPP
#define NEXUS_SOUP_BIN_TCP_CLIENT_HPP
#include <cstdint>
#include <string_view>
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/SoupBinTcp/HeartbeatPackets.hpp"
#include "Nexus/SoupBinTcp/LoginPackets.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpPacket.hpp"

namespace Nexus {

  /**
   * Implements a client using the SoupBinTCP protocol.
   * @param <C> The Channel connected to the SoupBinTCP server.
   * @param <T> The type of Timer used for heartbeats.
   */
  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class SoupBinTcpClient {
    public:

      /** The Channel connected to the SoupBinTCP server. */
      using Channel = Beam::dereference_t<C>;

      /** The type of Timer used for heartbeats. */
      using Timer = Beam::dereference_t<T>;

      /**
       * Constructs a SoupBinTcpClient.
       * @param username The username.
       * @param password The password.
       * @param channel The Channel connected to the SoupBinTCP server.
       * @param timer The Timer used for heartbeats.
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
       * @param timer The Timer used for heartbeats.
       */
      template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
      SoupBinTcpClient(std::string_view username, std::string_view password,
        std::string_view session, std::uint64_t sequence_number, CF&& channel,
        TF&& timer);

      ~SoupBinTcpClient();

      /** Reads the next SoupBinTcpPacket. */
      SoupBinTcpPacket read();

      /** Closes the connection to the server. */
      void close();

    private:
      Beam::local_ptr_t<C> m_channel;
      Beam::local_ptr_t<T> m_timer;
      Beam::SharedBuffer m_buffer;
      std::string m_session;
      std::uint64_t m_sequence_number;
      Beam::RoutineHandler m_heartbeat_loop;
      std::shared_ptr<Beam::Queue<Beam::Timer::Result>> m_timer_queue;
      Beam::OpenState m_open_state;

      SoupBinTcpClient(const SoupBinTcpClient&) = delete;
      SoupBinTcpClient& operator =(const SoupBinTcpClient&) = delete;
      void heartbeat_loop();
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
            m_timer_queue(
              std::make_shared<Beam::Queue<Beam::Timer::Result>>()) {
    m_timer->get_publisher().monitor(m_timer_queue);
    try {
      make_login_request_packet(
        username, password, session, sequence_number, Beam::out(m_buffer));
      m_channel->get_writer().write(m_buffer);
      auto login_response = SoupBinTcpPacket();
      while(true)  {
        reset(m_buffer);
        login_response =
          read_packet(m_channel->get_reader(), Beam::out(m_buffer));
        if(login_response.m_type != '+') {
          break;
        }
      }
      if(login_response.m_type == 'J') {
        auto login_rejected_packet =
          parse_login_rejected_packet(login_response);
        if(login_rejected_packet.m_reason == "A") {
          boost::throw_with_location(Beam::ConnectException("Not authorized."));
        } else if(login_rejected_packet.m_reason == "S") {
          boost::throw_with_location(
            Beam::ConnectException("Session unavailable."));
        } else {
          boost::throw_with_location(
            Beam::ConnectException("Unable to login."));
        }
      } else if(login_response.m_type != 'A') {
        boost::throw_with_location(
          Beam::ConnectException("Unrecognized login response."));
      }
      auto login_accepted_packet = parse_login_accepted_packet(login_response);
      m_session = login_accepted_packet.m_session;
      m_sequence_number = login_accepted_packet.m_sequence_number;
      m_timer->start();
      m_heartbeat_loop = Beam::spawn(
        std::bind_front(&SoupBinTcpClient::heartbeat_loop, this));
    } catch(const std::exception&) {
      close();
      throw;
    }
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "SoupBinTCP client failed to connect."));
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
  SoupBinTcpPacket SoupBinTcpClient<C, T>::read() {
    reset(m_buffer);
    return Beam::try_or_nest([&] {
      return read_packet(m_channel->get_reader(), Beam::out(m_buffer));
    }, Beam::IOException("Failed to read SoupBinTCP packet."));
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void SoupBinTcpClient<C, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_channel->get_connection().close();
    m_timer->cancel();
    m_timer_queue->close();
    m_heartbeat_loop.wait();
    m_open_state.close();
  }

  template<typename C, typename T> requires
    Beam::IsChannel<Beam::dereference_t<C>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void SoupBinTcpClient<C, T>::heartbeat_loop() {
    auto heartbeat_buffer = Beam::SharedBuffer();
    make_client_heartbeat_packet(Beam::out(heartbeat_buffer));
    try {
      while(m_open_state.is_open()) {
        auto result = m_timer_queue->pop();
        if(result == Beam::Timer::Result::EXPIRED) {
          m_channel->get_writer().write(heartbeat_buffer);
        } else {
          break;
        }
        m_timer->start();
      }
    } catch(const Beam::PipeBrokenException&) {
      return;
    }
  }
}

#endif
