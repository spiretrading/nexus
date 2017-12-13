#ifndef NEXUS_UTPPROTOCOLCLIENT_HPP
#define NEXUS_UTPPROTOCOLCLIENT_HPP
#include <cstdint>
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MoldUdp64/MoldUdp64Client.hpp"
#include "UtpMarketDataFeedClient/UtpMessage.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class UtpProtocolClient
      \brief Implements a client using the UTP protocol.
      \tparam ChannelType The type of Channel connected to the server.
   */
  template<typename ChannelType>
  class UtpProtocolClient : private boost::noncopyable {
    public:

      //! The type of Channel connected to the server.
      using Channel = Beam::GetTryDereferenceType<ChannelType>;

      //! Constructs a UtpProtocolClient.
      /*!
        \param channel The Channel to connect to the server
      */
      template<typename ChannelForward>
      UtpProtocolClient(ChannelForward&& channel);

      ~UtpProtocolClient();

      //! Reads the next message from the feed.
      /*!
        \return The next UtpMessage in the data feed.
      */
      UtpMessage Read();

      void Open();

      void Close();

    private:
      MoldUdp64::MoldUdp64Client<ChannelType> m_moldClient;
      std::uint64_t m_sequenceNumber;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ChannelType>
  template<typename ChannelForward>
  UtpProtocolClient<ChannelType>::UtpProtocolClient(ChannelForward&& channel)
      : m_moldClient{std::forward<ChannelForward>(channel)} {}

  template<typename ChannelType>
  UtpProtocolClient<ChannelType>::~UtpProtocolClient() {
    Close();
  }

  template<typename ChannelType>
  UtpMessage UtpProtocolClient<ChannelType>::Read() {
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
    }
    std::uint64_t sequenceNumber;
    auto moldMessage = m_moldClient.Read(Beam::Store(sequenceNumber));
    if(m_sequenceNumber != -1 && sequenceNumber > m_sequenceNumber + 1) {
      std::cout << "Packets dropped: " << (m_sequenceNumber + 1) << " - " <<
        (sequenceNumber - 1) << std::endl;
    }
    m_sequenceNumber = sequenceNumber;
    auto token = moldMessage.m_data;
    auto message = UtpMessage::Parse(Beam::Store(token), moldMessage.m_length);
    return message;
  }

  template<typename ChannelType>
  void UtpProtocolClient<ChannelType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_moldClient.Open();
      m_sequenceNumber = -1;
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ChannelType>
  void UtpProtocolClient<ChannelType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType>
  void UtpProtocolClient<ChannelType>::Shutdown() {
    m_moldClient.Close();
    m_openState.SetClosed();
  }
}
}

#endif
