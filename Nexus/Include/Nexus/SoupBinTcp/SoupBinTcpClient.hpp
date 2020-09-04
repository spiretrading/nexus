#ifndef NEXUS_SOUPBINTCPCLIENT_HPP
#define NEXUS_SOUPBINTCPCLIENT_HPP
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

namespace Nexus {
namespace SoupBinTcp {

  /*! \struct SoupBinTcpClient
      \brief Implements a client using the SoupBinTCP protocol.
      \tparam ChannelType The Channel connected to the SoupBinTCP server.
      \tparam TimerType The type of Timer used for heartbeats.
   */
  template<typename ChannelType, typename TimerType>
  class SoupBinTcpClient : private boost::noncopyable {
    public:

      //! The Channel connected to the SoupBinTCP server.
      using Channel = Beam::GetTryDereferenceType<ChannelType>;

      //! The type of Timer used for heartbeats.
      using Timer = Beam::GetTryDereferenceType<TimerType>;

      //! Constructs a SoupBinTcpClient.
      /*!
        \param channel The Channel connected to the SoupBinTCP server.
        \param timer The Timer used for heartbeats.
      */
      template<typename ChannelForward, typename TimerForward>
      SoupBinTcpClient(ChannelForward&& channel, TimerForward&& timer);

      //! Reads the next SoupBinTcpPacket.
      SoupBinTcpPacket Read();

      //! Logs onto the server.
      /*!
        \param username The username.
        \param password The password.
      */
      void Login(const std::string& username, const std::string& password);

      //! Logs onto the server.
      /*!
        \param username The username.
        \param password The password.
        \param session The existing session to log into.
        \param sequenceNumber The next sequence number to receive from the
               server.
      */
      void Login(const std::string& username, const std::string& password,
        const std::string& session, std::uint64_t sequenceNumber);

      //! Closes the connection to the server.
      void Close();

    private:
      Beam::GetOptionalLocalPtr<ChannelType> m_channel;
      Beam::GetOptionalLocalPtr<TimerType> m_timer;
      typename Channel::Reader::Buffer m_buffer;
      std::string m_session;
      std::uint64_t m_sequenceNumber;
      Beam::Routines::RoutineHandler m_heartbeatLoop;
      std::shared_ptr<Beam::Queue<Beam::Threading::Timer::Result>> m_timerQueue;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      void HeartbeatLoop();
  };

  template<typename ChannelType, typename TimerType>
  template<typename ChannelForward, typename TimerForward>
  SoupBinTcpClient<ChannelType, TimerType>::SoupBinTcpClient(
      ChannelForward&& channel, TimerForward&& timer)
      : m_channel{std::forward<ChannelForward>(channel)},
        m_timer{std::forward<TimerForward>(timer)},
        m_timerQueue{std::make_shared<
          Beam::Queue<Beam::Threading::Timer::Result>>()} {
    m_timer->GetPublisher().Monitor(m_timerQueue);
  }

  template<typename ChannelType, typename TimerType>
  SoupBinTcpPacket SoupBinTcpClient<ChannelType, TimerType>::Read() {
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
    }
    m_buffer.Reset();
    return ReadPacket(m_channel->GetReader(), Beam::Store(m_buffer));
  }

  template<typename ChannelType, typename TimerType>
  void SoupBinTcpClient<ChannelType, TimerType>::Login(
      const std::string& username, const std::string& password) {
    Login(username, password, {}, 1);
  }

  /** TODO: Move to constructor. */
#if 0
  template<typename ChannelType, typename TimerType>
  void SoupBinTcpClient<ChannelType, TimerType>::Login(
      const std::string& username, const std::string& password,
      const std::string& session, std::uint64_t sequenceNumber) {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_buffer.Reset();
      BuildLoginRequestPacket(username, password, session, sequenceNumber,
        Beam::Store(m_buffer));
      m_channel->GetWriter().Write(m_buffer);
      SoupBinTcpPacket loginResponse;
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
          BOOST_THROW_EXCEPTION(Beam::IO::ConnectException{"Not authorized."});
        } else if(loginRejectedPacket.m_reason == "S") {
          BOOST_THROW_EXCEPTION(
            Beam::IO::ConnectException{"Session unavailable."});
        } else {
          BOOST_THROW_EXCEPTION(Beam::IO::ConnectException{"Unable to login."});
        }
      } else if(loginResponse.m_type != 'A') {
        BOOST_THROW_EXCEPTION(
          Beam::IO::ConnectException{"Unrecognized login response."});
      }
      auto loginAcceptedPacket = ParseLoginAcceptedPacket(loginResponse);
      m_session = loginAcceptedPacket.m_session;
      m_sequenceNumber = loginAcceptedPacket.m_sequenceNumber;
      m_timer->Start();
      m_heartbeatLoop = Beam::Routines::Spawn(
        std::bind(&SoupBinTcpClient::HeartbeatLoop, this));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }
#endif

  template<typename ChannelType, typename TimerType>
  void SoupBinTcpClient<ChannelType, TimerType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType, typename TimerType>
  void SoupBinTcpClient<ChannelType, TimerType>::Shutdown() {
    m_channel->GetConnection().Close();
    m_timer->Cancel();
    m_timerQueue->Break();
    m_heartbeatLoop.Wait();
    m_openState.SetClosed();
  }

  template<typename ChannelType, typename TimerType>
  void SoupBinTcpClient<ChannelType, TimerType>::HeartbeatLoop() {
    typename Channel::Writer::Buffer heartbeatBuffer;
    BuildClientHeartbeatPacket(Beam::Store(heartbeatBuffer));
    try {
      while(m_openState.IsRunning()) {
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
}

#endif
