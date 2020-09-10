#ifndef NEXUS_CTA_PROTOCOL_CLIENT_HPP
#define NEXUS_CTA_PROTOCOL_CLIENT_HPP
#include <cstdint>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include "CtaMarketDataFeedClient/CtaMessage.hpp"

namespace Nexus::MarketDataService {

  /**
   * Implements a client using the CTA protocol.
   * @param <C> The type of Channel connected to the server.
   */
  template<typename C>
  class CtaProtocolClient : private boost::noncopyable {
    public:

      /** The type of Channel connected to the server. */
      using Channel = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a CtaProtocolClient.
       * @param channel The Channel to connect to the server
       */
      template<typename CF>
      CtaProtocolClient(CF&& channel);

      ~CtaProtocolClient();

      /**
       * Reads the next message from the feed.
       * @return The next CtaMessage in the data feed.
       */
      CtaMessage Read();

      void Close();

    private:
      using Buffer = typename Channel::Reader::Buffer;
      Beam::GetOptionalLocalPtr<C> m_channel;
      Buffer m_buffer;
      const char* m_token;
      CtaBlock m_block;
      std::uint8_t m_blockIndex;
      std::uint32_t m_sequenceNumber;
      Beam::IO::OpenState m_openState;
  };

  template<typename C>
  template<typename CF>
  CtaProtocolClient<C>::CtaProtocolClient(CF&& channel)
      : m_channel(std::forward<C>(channel)),
        m_token(m_buffer.GetData()),
        m_blockIndex(0),
        m_sequenceNumber(-1) {
    m_block.m_header.m_messageCount = 0;
  }

  template<typename C>
  CtaProtocolClient<C>::~CtaProtocolClient() {
    Close();
  }

  template<typename C>
  CtaMessage CtaProtocolClient<C>::Read() {
    m_openState.EnsureOpen();
    while(m_blockIndex == m_block.m_header.m_messageCount) {
      m_buffer.Reset();
      m_channel->GetReader().Read(Beam::Store(m_buffer));
      auto blockToken = m_buffer.GetData();
      m_block = CtaBlock::Parse(Beam::Store(blockToken),
        static_cast<std::uint16_t>(m_buffer.GetSize()));
      m_blockIndex = 0;
      m_token = m_block.m_messages;
      if(m_sequenceNumber == -1 || m_block.m_header.m_sequenceNumber ==
          m_sequenceNumber + 1) {
        m_sequenceNumber = m_block.m_header.m_sequenceNumber;
      } else if(m_block.m_header.m_sequenceNumber > m_sequenceNumber + 1) {
        std::cout << "Packets dropped: " << (m_sequenceNumber + 1) << " - " <<
          (m_block.m_header.m_sequenceNumber - 1) << std::endl;
        m_sequenceNumber = m_block.m_header.m_sequenceNumber;
      }
    }
    auto remainingSize = static_cast<std::uint16_t>(
      (m_buffer.GetData() + m_buffer.GetSize()) - m_token);
    try {
      auto message = CtaMessage::Parse(m_block, Beam::Store(m_token),
        remainingSize);
      ++m_blockIndex;
      return message;
    } catch(const std::exception&) {
      m_blockIndex = m_block.m_header.m_messageCount;
      throw;
    }
  }

  template<typename C>
  void CtaProtocolClient<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_channel->GetConnection().Close();
    m_openState.Close();
  }
}

#endif
