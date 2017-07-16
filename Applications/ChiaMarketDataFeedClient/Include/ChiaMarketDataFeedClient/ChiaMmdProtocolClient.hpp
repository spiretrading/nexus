#ifndef NEXUS_CHIAMMDPROTOCOLCLIENT_HPP
#define NEXUS_CHIAMMDPROTOCOLCLIENT_HPP
#include <deque>
#include <functional>
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "ChiaMarketDataFeedClient/ChiaMdProtocolClient.hpp"
#include "ChiaMarketDataFeedClient/ChiaMessage.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolClient.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolMessage.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class ChiaMmdProtocolClient
      \brief Parses packets from the CHIA multicast market data feed.
      \tparam ChannelType The type of Channel receiving data.
      \tparam RetransmissionChannelType The type of Channel used for
              retransmissions.
   */
  template<typename ChannelType, typename RetransmissionChannelType>
  class ChiaMmdProtocolClient : private boost::noncopyable {
    public:

      //! The type of Channel receiving data.
      using Channel = Beam::GetTryDereferenceType<ChannelType>;

      //! The type of Channel used for retransmission.
      using RetransmissionChannel = Beam::GetTryDereferenceType<
        RetransmissionChannelType>;

      //! The factory function used to build RetransmissionChannels.
      using RetransmissionChannelFactory =
        std::function<std::unique_ptr<RetransmissionChannel> ()>;

      //! Constructs a ChiaMmdProtocolClient.
      /*!
        \param channel Initializes the Channel receiving data.
        \param username The retransmission username.
        \param password The retransmission password.
        \param retransmissionFactory The factory function used to build
               RetransmissionChannels.
      */
      template<typename ChannelForward>
      ChiaMmdProtocolClient(ChannelForward&& channel, std::string username,
        std::string password,
        RetransmissionChannelFactory retransmissionChannelFactory);

      ~ChiaMmdProtocolClient();

      //! Reads the next message.
      ChiaMessage Read();

      void Open();

      void Close();

    private:
      using ProtocolClient =
        Nexus::BinarySequenceProtocol::BinarySequenceProtocolClient<
        ChannelType, std::uint32_t>;
      Nexus::BinarySequenceProtocol::BinarySequenceProtocolClient<
        ChannelType, std::uint32_t> m_client;
      std::string m_username;
      std::string m_password;
      RetransmissionChannelFactory m_retransmissionChannelFactory;
      typename ProtocolClient::Sequence m_sequenceNumber;
      std::deque<Beam::IO::SharedBuffer> m_pendingMessageBuffers;
      Beam::IO::SharedBuffer m_messageBuffer;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ChannelType, typename RetransmissionChannelType>
  template<typename ChannelForward>
  ChiaMmdProtocolClient<ChannelType, RetransmissionChannelType>::
      ChiaMmdProtocolClient(ChannelForward&& channel, std::string username,
      std::string password,
      RetransmissionChannelFactory retransmissionChannelFactory)
      : m_client{std::forward<ChannelForward>(channel)},
        m_username{std::move(username)},
        m_password{std::move(password)},
        m_retransmissionChannelFactory{
          std::move(retransmissionChannelFactory)} {}

  template<typename ChannelType, typename RetransmissionChannelType>
  ChiaMmdProtocolClient<ChannelType, RetransmissionChannelType>::
      ~ChiaMmdProtocolClient() {
    Close();
  }

  template<typename ChannelType, typename RetransmissionChannelType>
  ChiaMessage ChiaMmdProtocolClient<ChannelType, RetransmissionChannelType>::
      Read() {
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
    }
    if(!m_pendingMessageBuffers.empty()) {
      m_messageBuffer = m_pendingMessageBuffers.front();
      m_pendingMessageBuffers.pop_front();
      ++m_sequenceNumber;
      auto message = ChiaMessage::Parse(m_messageBuffer.GetData(),
        m_messageBuffer.GetSize());
      return message;
    }
    while(true) {
      std::uint32_t sequenceNumber;
      auto protocolMessage = m_client.Read(Beam::Store(sequenceNumber));
      if(sequenceNumber <= m_sequenceNumber) {
        continue;
      }
      if(m_sequenceNumber != 0 &&
          sequenceNumber > m_sequenceNumber + 1) {
        std::cout << "Packets dropped: " << (m_sequenceNumber + 1) <<
          " - " << (sequenceNumber - 1) << std::endl;
        ChiaMdProtocolClient<std::unique_ptr<RetransmissionChannel>>
          retransmissionClient{m_retransmissionChannelFactory(), m_username,
          m_password, "", m_sequenceNumber + 1};
        retransmissionClient.Open();
        std::uint32_t retransmissionSequence = 0;
        try {
          while(true) {
            auto buffer = retransmissionClient.ReadBuffer(
              Beam::Store(retransmissionSequence));
            if(retransmissionSequence < m_sequenceNumber) {
              continue;
            } else if(retransmissionSequence > sequenceNumber) {
              break;
            }
            m_pendingMessageBuffers.push_back(std::move(buffer));
          }
        } catch(const std::exception&) {
        }
        std::cout << "Recovered packets: " << (m_sequenceNumber + 1) <<
          " - " << retransmissionSequence << std::endl;
        return Read();
      }
      m_sequenceNumber = sequenceNumber;
      auto message = ChiaMessage::Parse(protocolMessage.m_data,
        protocolMessage.m_length);
      return message;
    }
  }

  template<typename ChannelType, typename RetransmissionChannelType>
  void ChiaMmdProtocolClient<ChannelType, RetransmissionChannelType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_client.Open();
      m_sequenceNumber = 0;
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ChannelType, typename RetransmissionChannelType>
  void ChiaMmdProtocolClient<ChannelType, RetransmissionChannelType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType, typename RetransmissionChannelType>
  void ChiaMmdProtocolClient<ChannelType, RetransmissionChannelType>::
      Shutdown() {
    m_client.Close();
    m_openState.SetClosed();
  }
}
}

#endif
