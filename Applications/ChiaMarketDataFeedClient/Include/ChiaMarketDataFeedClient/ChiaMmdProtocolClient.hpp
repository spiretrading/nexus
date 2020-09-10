#ifndef NEXUS_CHIA_MMD_PROTOCOL_CLIENT_HPP
#define NEXUS_CHIA_MMD_PROTOCOL_CLIENT_HPP
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

namespace Nexus::MarketDataService {

  /**
   * Parses packets from the CHIA multicast market data feed.
   * @param <C> The type of Channel receiving data.
   * @param <R> The type of Channel used for retransmissions.
   */
  template<typename C, typename R>
  class ChiaMmdProtocolClient : private boost::noncopyable {
    public:

      /** The type of Channel receiving data. */
      using Channel = Beam::GetTryDereferenceType<C>;

      /** The type of Channel used for retransmission. */
      using RetransmissionChannel = Beam::GetTryDereferenceType<R>;

      /** The factory function used to build RetransmissionChannels. */
      using RetransmissionChannelFactory =
        std::function<std::unique_ptr<RetransmissionChannel> ()>;

      /**
       * Constructs a ChiaMmdProtocolClient.
       * @param channel Initializes the Channel receiving data.
       * @param username The retransmission username.
       * @param password The retransmission password.
       * @param retransmissionFactory The factory function used to build
       *        RetransmissionChannels.
       */
      template<typename CF>
      ChiaMmdProtocolClient(CF&& channel, std::string username,
        std::string password,
        RetransmissionChannelFactory retransmissionChannelFactory);

      ~ChiaMmdProtocolClient();

      /** Reads the next message. */
      ChiaMessage Read();

      void Close();

    private:
      using ProtocolClient =
        BinarySequenceProtocol::BinarySequenceProtocolClient<C, std::uint32_t>;
      ProtocolClient m_client;
      std::string m_username;
      std::string m_password;
      RetransmissionChannelFactory m_retransmissionChannelFactory;
      typename ProtocolClient::Sequence m_sequenceNumber;
      std::deque<Beam::IO::SharedBuffer> m_pendingMessageBuffers;
      Beam::IO::SharedBuffer m_messageBuffer;
      Beam::IO::OpenState m_openState;
  };

  template<typename C, typename R>
  template<typename CF>
  ChiaMmdProtocolClient<C, R>::ChiaMmdProtocolClient(CF&& channel,
    std::string username, std::string password,
    RetransmissionChannelFactory retransmissionChannelFactory)
    : m_client(std::forward<CF>(channel)),
      m_username(std::move(username)),
      m_password(std::move(password)),
      m_retransmissionChannelFactory(std::move(retransmissionChannelFactory)),
      m_sequenceNumber(0) {}

  template<typename C, typename R>
  ChiaMmdProtocolClient<C, R>::~ChiaMmdProtocolClient() {
    Close();
  }

  template<typename C, typename R>
  ChiaMessage ChiaMmdProtocolClient<C, R>::Read() {
    m_openState.EnsureOpen();
    if(!m_pendingMessageBuffers.empty()) {
      m_messageBuffer = m_pendingMessageBuffers.front();
      m_pendingMessageBuffers.pop_front();
      ++m_sequenceNumber;
      auto message = ChiaMessage::Parse(m_messageBuffer.GetData(),
        m_messageBuffer.GetSize());
      return message;
    }
    while(true) {
      auto sequenceNumber = std::uint32_t();
      auto protocolMessage = m_client.Read(Beam::Store(sequenceNumber));
      if(sequenceNumber <= m_sequenceNumber) {
        continue;
      }
      if(m_sequenceNumber != 0 &&
          sequenceNumber > m_sequenceNumber + 1) {
        auto retransmissionChannel = m_retransmissionChannelFactory();
        if(retransmissionChannel != nullptr) {
          try {
            auto retransmissionClient = ChiaMdProtocolClient<
              std::unique_ptr<RetransmissionChannel>>(
              std::move(retransmissionChannel), m_username, m_password, "",
              m_sequenceNumber + 1);
            auto retransmissionSequence = std::uint32_t(0);
            while(retransmissionSequence < sequenceNumber) {
              auto buffer = retransmissionClient.ReadBuffer(
                Beam::Store(retransmissionSequence));
              if(retransmissionSequence == (m_sequenceNumber + 1) +
                  m_pendingMessageBuffers.size()) {
                m_pendingMessageBuffers.push_back(std::move(buffer));
              }
            }
          } catch(const std::exception&) {
            std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
          }
          if(!m_pendingMessageBuffers.empty()) {
            return Read();
          }
        }
        std::cout << "Packets dropped: " << (m_sequenceNumber + 1) <<
          " - " << (sequenceNumber - 1) << std::endl;
      }
      m_sequenceNumber = sequenceNumber;
      auto message = ChiaMessage::Parse(protocolMessage.m_data,
        protocolMessage.m_length);
      return message;
    }
  }

  template<typename C, typename R>
  void ChiaMmdProtocolClient<C, R>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_client.Close();
    m_openState.Close();
  }
}

#endif
