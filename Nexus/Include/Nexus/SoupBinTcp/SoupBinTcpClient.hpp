#ifndef NEXUS_SOUP_BIN_TCP_CLIENT_HPP
#define NEXUS_SOUP_BIN_TCP_CLIENT_HPP
#include <cstdint>
#include <string>
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/Timer.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/SoupBinTcp/HeartbeatPackets.hpp"
#include "Nexus/SoupBinTcp/LoginPackets.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcp.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpPacket.hpp"

namespace Nexus::SoupBinTcp {

  /**
   * Implements a client using the SoupBinTCP protocol.
   * @param <C> The Channel connected to the SoupBinTCP server.
   * @param <T> The type of Timer used for heartbeats.
   */
  template<typename C, typename T>
  class SoupBinTcpClient : private boost::noncopyable {
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
      template<typename CF, typename TF>
      SoupBinTcpClient(const std::string& username, const std::string& password,
        CF&& channel, TF&& timer);

      /**
       * Constructs a SoupBinTcpClient.
       * @param username The username.
       * @param password The password.
       * @param session The existing session to log into.
       * @param sequenceNumber The next sequence number to receive from the
       *        server.
       * @param channel The Channel connected to the SoupBinTCP server.
       * @param timer The Timer used for heartbeats.
       */
      template<typename CF, typename TF>
      SoupBinTcpClient(const std::string& username, const std::string& password,
        const std::string& session, std::uint64_t sequenceNumber, CF&& channel,
        TF&& timer);

      /** Reads the next SoupBinTcpPacket. */
      SoupBinTcpPacket Read();

      /** Closes the connection to the server. */
      void Close();

    private:
      Beam::GetOptionalLocalPtr<C> m_channel;
      Beam::GetOptionalLocalPtr<T> m_timer;
      typename Channel::Reader::Buffer m_buffer;
      std::string m_session;
      std::uint64_t m_sequenceNumber;
      Beam::Routines::RoutineHandler m_heartbeatLoop;
      std::shared_ptr<Beam::Queue<Beam::Threading::Timer::Result>> m_timerQueue;
      Beam::IO::OpenState m_openState;

      void HeartbeatLoop();
  };

  template<typename C, typename T>
  template<typename CF, typename TF>
  SoupBinTcpClient<C, T>::SoupBinTcpClient(const std::string& username,
    const std::string& password, CF&& channel, TF&& timer)
    : SoupBinTcpClient(username, password, {}, 1, std::forward<CF>(channel),
        std::forward<TF>(timer)) {}

  template<typename C, typename T>
  template<typename CF, typename TF>
  SoupBinTcpClient<C, T>::SoupBinTcpClient(const std::string& username,
      const std::string& password, const std::string& session,
      std::uint64_t sequenceNumber, CF&& channel, TF&& timer)
      : m_channel(std::forward<CF>(channel)),
        m_timer(std::forward<TF>(timer)),
        m_timerQueue(
          std::make_shared<Beam::Queue<Beam::Threading::Timer::Result>>()) {
    m_timer->GetPublisher().Monitor(m_timerQueue);
    try {
      BuildLoginRequestPacket(username, password, session, sequenceNumber,
        Beam::Store(m_buffer));
      m_channel->GetWriter().Write(m_buffer);
      auto loginResponse = SoupBinTcpPacket();
      while(true)  {
        m_buffer.Reset();
        loginResponse = ReadPacket(m_channel->GetReader(),
          Beam::Store(m_buffer));
        if(loginResponse.m_type != '+') {
          break;
        }
      }
      if(loginResponse.m_type == 'J') {
        auto loginRejectedPacket = ParseLoginRejectedPacket(loginResponse);
        if(loginRejectedPacket.m_reason == "A") {
          BOOST_THROW_EXCEPTION(Beam::IO::ConnectException("Not authorized."));
        } else if(loginRejectedPacket.m_reason == "S") {
          BOOST_THROW_EXCEPTION(
            Beam::IO::ConnectException("Session unavailable."));
        } else {
          BOOST_THROW_EXCEPTION(Beam::IO::ConnectException("Unable to login."));
        }
      } else if(loginResponse.m_type != 'A') {
        BOOST_THROW_EXCEPTION(
          Beam::IO::ConnectException("Unrecognized login response."));
      }
      auto loginAcceptedPacket = ParseLoginAcceptedPacket(loginResponse);
      m_session = loginAcceptedPacket.m_session;
      m_sequenceNumber = loginAcceptedPacket.m_sequenceNumber;
      m_timer->Start();
      m_heartbeatLoop = Beam::Routines::Spawn(
        std::bind(&SoupBinTcpClient::HeartbeatLoop, this));
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C, typename T>
  SoupBinTcpPacket SoupBinTcpClient<C, T>::Read() {
    m_openState.EnsureOpen();
    m_buffer.Reset();
    return ReadPacket(m_channel->GetReader(), Beam::Store(m_buffer));
  }

  template<typename C, typename T>
  void SoupBinTcpClient<C, T>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_channel->GetConnection().Close();
    m_timer->Cancel();
    m_timerQueue->Break();
    m_heartbeatLoop.Wait();
    m_openState.Close();
  }

  template<typename C, typename T>
  void SoupBinTcpClient<C, T>::HeartbeatLoop() {
    auto heartbeatBuffer = typename Channel::Writer::Buffer();
    BuildClientHeartbeatPacket(Beam::Store(heartbeatBuffer));
    try {
      while(m_openState.IsOpen()) {
        auto result = m_timerQueue->Pop();
        if(result == Beam::Threading::Timer::Result::EXPIRED) {
          m_channel->GetWriter().Write(heartbeatBuffer);
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
