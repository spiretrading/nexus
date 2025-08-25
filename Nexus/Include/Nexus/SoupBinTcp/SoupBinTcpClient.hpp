#ifndef NEXUS_SOUP_BIN_TCP_CLIENT_HPP
#define NEXUS_SOUP_BIN_TCP_CLIENT_HPP
#include <cstdint>
#include <string>
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/Timer.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/SoupBinTcp/HeartbeatPackets.hpp"
#include "Nexus/SoupBinTcp/LoginPackets.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpPacket.hpp"

namespace Nexus {

  /**
   * Implements a client using the SoupBinTCP protocol.
   * @param <C> The Channel connected to the SoupBinTCP server.
   * @param <T> The type of Timer used for heartbeats.
   */
  template<typename C, typename T>
  class SoupBinTcpClient {
    public:

      /** The Channel connected to the SoupBinTCP server. */
      using Channel = Beam::GetTryDereferenceType<C>;

      /** The type of Timer used for heartbeats. */
      using Timer = Beam::GetTryDereferenceType<T>;

      /**
       * Constructs a SoupBinTcpClient.
       * @param username The username.
       * @param password The password.
       * @param channel The Channel connected to the SoupBinTCP server.
       * @param timer The Timer used for heartbeats.
       */
      template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
      SoupBinTcpClient(const std::string& username, const std::string& password,
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
      SoupBinTcpClient(const std::string& username, const std::string& password,
        const std::string& session, std::uint64_t sequence_number, CF&& channel,
        TF&& timer);

      ~SoupBinTcpClient();

      /** Reads the next SoupBinTcpPacket. */
      SoupBinTcpPacket read();

      /** Closes the connection to the server. */
      void close();

    private:
      Beam::GetOptionalLocalPtr<C> m_channel;
      Beam::GetOptionalLocalPtr<T> m_timer;
      Beam::IO::SharedBuffer m_buffer;
      std::string m_session;
      std::uint64_t m_sequence_number;
      Beam::Routines::RoutineHandler m_heartbeat_loop;
      std::shared_ptr<Beam::Queue<Beam::Threading::Timer::Result>>
        m_timer_queue;
      Beam::IO::OpenState m_open_state;

      SoupBinTcpClient(const SoupBinTcpClient&) = delete;
      SoupBinTcpClient& operator =(const SoupBinTcpClient&) = delete;
      void heartbeat_loop();
  };

  template<typename C, typename T>
  SoupBinTcpClient(const std::string&, const std::string&, C&&, T&&) ->
    SoupBinTcpClient<std::remove_reference_t<C>, std::remove_reference_t<T>>;

  template<typename C, typename T>
  SoupBinTcpClient(const std::string&, const std::string&, const std::string&,
    std::uint64_t, C&&, T&&) ->
      SoupBinTcpClient<std::remove_reference_t<C>, std::remove_reference_t<T>>;

  template<typename C, typename T>
  template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
  SoupBinTcpClient<C, T>::SoupBinTcpClient(const std::string& username,
    const std::string& password, CF&& channel, TF&& timer)
    : SoupBinTcpClient(username, password, {}, 1, std::forward<CF>(channel),
        std::forward<TF>(timer)) {}

  template<typename C, typename T>
  template<Beam::Initializes<C> CF, Beam::Initializes<T> TF>
  SoupBinTcpClient<C, T>::SoupBinTcpClient(const std::string& username,
      const std::string& password, const std::string& session,
      std::uint64_t sequence_number, CF&& channel, TF&& timer)
      try : m_channel(std::forward<CF>(channel)),
            m_timer(std::forward<TF>(timer)),
            m_timer_queue(
              std::make_shared<Beam::Queue<Beam::Threading::Timer::Result>>()) {
    m_timer->GetPublisher().Monitor(m_timer_queue);
    try {
      make_login_request_packet(
        username, password, session, sequence_number, Beam::Store(m_buffer));
      m_channel->GetWriter().Write(m_buffer);
      auto login_response = SoupBinTcpPacket();
      while(true)  {
        m_buffer.Reset();
        login_response =
          read_packet(m_channel->GetReader(), Beam::Store(m_buffer));
        if(login_response.m_type != '+') {
          break;
        }
      }
      if(login_response.m_type == 'J') {
        auto login_rejected_packet =
          parse_login_rejected_packet(login_response);
        if(login_rejected_packet.m_reason == "A") {
          BOOST_THROW_EXCEPTION(Beam::IO::ConnectException("Not authorized."));
        } else if(login_rejected_packet.m_reason == "S") {
          BOOST_THROW_EXCEPTION(
            Beam::IO::ConnectException("Session unavailable."));
        } else {
          BOOST_THROW_EXCEPTION(Beam::IO::ConnectException("Unable to login."));
        }
      } else if(login_response.m_type != 'A') {
        BOOST_THROW_EXCEPTION(
          Beam::IO::ConnectException("Unrecognized login response."));
      }
      auto login_accepted_packet = parse_login_accepted_packet(login_response);
      m_session = login_accepted_packet.m_session;
      m_sequence_number = login_accepted_packet.m_sequence_number;
      m_timer->Start();
      m_heartbeat_loop = Beam::Routines::Spawn(
        std::bind_front(&SoupBinTcpClient::heartbeat_loop, this));
    } catch(const std::exception&) {
      close();
      BOOST_RETHROW;
    }
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "SoupBinTCP client failed to connect."));
  }

  template<typename C, typename T>
  SoupBinTcpClient<C, T>::~SoupBinTcpClient() {
    close();
  }

  template<typename C, typename T>
  SoupBinTcpPacket SoupBinTcpClient<C, T>::read() {
    m_buffer.Reset();
    return Beam::TryOrNest([&] {
      return read_packet(m_channel->GetReader(), Beam::Store(m_buffer));
    }, Beam::IO::IOException("Failed to read SoupBinTCP packet."));
  }

  template<typename C, typename T>
  void SoupBinTcpClient<C, T>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_channel->GetConnection().Close();
    m_timer->Cancel();
    m_timer_queue->Break();
    m_heartbeat_loop.Wait();
    m_open_state.Close();
  }

  template<typename C, typename T>
  void SoupBinTcpClient<C, T>::heartbeat_loop() {
    auto heartbeat_buffer = typename Channel::Writer::Buffer();
    make_client_heartbeat_packet(Beam::Store(heartbeat_buffer));
    try {
      while(m_open_state.IsOpen()) {
        auto result = m_timer_queue->Pop();
        if(result == Beam::Threading::Timer::Result::EXPIRED) {
          m_channel->GetWriter().Write(heartbeat_buffer);
        } else {
          break;
        }
        m_timer->Start();
      }
    } catch(const Beam::PipeBrokenException&) {
      return;
    }
  }
}

#endif
