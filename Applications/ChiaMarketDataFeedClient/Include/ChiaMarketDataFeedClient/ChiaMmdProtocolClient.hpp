#ifndef NEXUS_CHIAMMDPROTOCOLCLIENT_HPP
#define NEXUS_CHIAMMDPROTOCOLCLIENT_HPP
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "ChiaMarketDataFeedClient/ChiaMessage.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolClient.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolMessage.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class ChiaMmdProtocolClient
      \brief Parses packets from the CHIA multicast market data feed.
      \tparam ChannelType The type of Channel receiving data.
   */
  template<typename ChannelType>
  class ChiaMmdProtocolClient : private boost::noncopyable {
    public:

      //! The type of Channel receiving data.
      using Channel = Beam::GetTryDereferenceType<ChannelType>;

      //! Constructs a ChiaMmdProtocolClient.
      /*!
        \param channel Initializes the Channel receiving data.
      */
      template<typename ChannelForward>
      ChiaMmdProtocolClient(ChannelForward&& channel);

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
      typename ProtocolClient::Sequence m_sequenceNumber;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ChannelType>
  template<typename ChannelForward>
  ChiaMmdProtocolClient<ChannelType>::ChiaMmdProtocolClient(
      ChannelForward&& channel)
      : m_client{std::forward<ChannelForward>(channel)} {}

  template<typename ChannelType>
  ChiaMmdProtocolClient<ChannelType>::~ChiaMmdProtocolClient() {
    Close();
  }

  template<typename ChannelType>
  ChiaMessage ChiaMmdProtocolClient<ChannelType>::Read() {
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
    }
    while(true) {
      std::uint32_t sequenceNumber;
      auto protocolMessage = m_client.Read(Beam::Store(sequenceNumber));
      if(sequenceNumber <= m_sequenceNumber) {
        continue;
      }
      m_sequenceNumber = sequenceNumber;
      auto message = ChiaMessage::Parse(protocolMessage.m_data,
        protocolMessage.m_length);
      return message;
    }
  }

  template<typename ChannelType>
  void ChiaMmdProtocolClient<ChannelType>::Open() {
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

  template<typename ChannelType>
  void ChiaMmdProtocolClient<ChannelType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType>
  void ChiaMmdProtocolClient<ChannelType>::Shutdown() {
    m_client.Close();
    m_openState.SetClosed();
  }
}
}

#endif
