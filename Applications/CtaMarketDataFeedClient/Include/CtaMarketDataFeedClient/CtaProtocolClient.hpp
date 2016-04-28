#ifndef NEXUS_CTAPROTOCOLCLIENT_HPP
#define NEXUS_CTAPROTOCOLCLIENT_HPP
#include <cstdint>
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include "CtaMarketDataFeedClient/CtaMessage.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class CtaProtocolClient
      \brief Implements a client using the CTA protocol.
      \tparam ChannelType The type of Channel connected to the server.
   */
  template<typename ChannelType>
  class CtaProtocolClient : private boost::noncopyable {
    public:

      //! The type of Channel connected to the server.
      using Channel = Beam::GetTryDereferenceType<ChannelType>;

      //! Constructs a CtaProtocolClient.
      /*!
        \param channel The Channel to connect to the server
      */
      template<typename ChannelForward>
      CtaProtocolClient(ChannelForward&& channel);

      ~CtaProtocolClient();

      //! Reads the next message from the feed.
      /*!
        \return The next CtaMessage in the data feed.
      */
      CtaMessage Read();

      void Open();

      void Close();

    private:
      using Buffer = typename Channel::Reader::Buffer;
      Beam::GetOptionalLocalPtr<ChannelType> m_channel;
      Buffer m_buffer;
      const char* m_token;
      std::uint32_t m_sequenceNumber;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ChannelType>
  template<typename ChannelForward>
  CtaProtocolClient<ChannelType>::CtaProtocolClient(ChannelForward&& channel)
      : m_channel{std::forward<ChannelType>(channel)} {}

  template<typename ChannelType>
  CtaProtocolClient<ChannelType>::~CtaProtocolClient() {
    Close();
  }

  template<typename ChannelType>
  CtaMessage CtaProtocolClient<ChannelType>::Read() {
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
    }
    while(true) {
      if(m_token == m_buffer.GetData() + m_buffer.GetSize()) {
        m_buffer.Reset();
        m_channel->GetReader().Read(Beam::Store(m_buffer));
        m_token = m_buffer.GetData() + 1;
      }
      auto remainingSize = static_cast<std::uint16_t>(
        (m_buffer.GetData() + m_buffer.GetSize()) - m_token);
      auto message = CtaMessage::Parse(Beam::Store(m_token), remainingSize);
      if(m_sequenceNumber == -1 || message.m_sequenceNumber ==
          m_sequenceNumber + 1) {
        m_sequenceNumber = message.m_sequenceNumber;
        return message;
      } else if(message.m_sequenceNumber > m_sequenceNumber + 1) {
        std::cout << "Packets dropped: " << (m_sequenceNumber + 1) << " - " <<
          (message.m_sequenceNumber - 1) << std::endl;
        m_sequenceNumber = message.m_sequenceNumber;
        return message;
      }
    }
  }

  template<typename ChannelType>
  void CtaProtocolClient<ChannelType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_channel->GetConnection().Open();
      m_sequenceNumber = -1;
      m_token = m_buffer.GetData();
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ChannelType>
  void CtaProtocolClient<ChannelType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType>
  void CtaProtocolClient<ChannelType>::Shutdown() {
    m_channel->GetConnection().Close();
    m_openState.SetClosed();
  }
}
}

#endif
