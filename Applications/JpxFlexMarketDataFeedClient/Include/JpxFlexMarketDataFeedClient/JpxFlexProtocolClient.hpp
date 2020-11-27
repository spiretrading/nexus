#ifndef NEXUS_JPX_FLEX_PROTOCOL_CLIENT_HPP
#define NEXUS_JPX_FLEX_PROTOCOL_CLIENT_HPP
#include <utility>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Utilities/Expect.hpp>
#include "JpxFlexMarketDataFeedClient/JpxFlexMessage.hpp"

namespace Nexus::MarketDataService {

  /**
   * Parses packets from the JPX Flex market data feed.
   * @param <C> The type of channel receiving the packets to parse.
   */
  template<typename C>
  class JpxFlexProtocolClient {
    public:

      /** The type of channel receiving messages. */
      using Channel = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a JpxFlexProtocolClient.
       * @param channel The channel receiving the packets to parse.
       */
      template<typename CF>
      explicit JpxFlexProtocolClient(CF&& channel);

      ~JpxFlexProtocolClient();

      /** Reads the next message from the feed. */
      JpxFlexMessage Read();

      /**
       * Reads the next message from the feed.
       * @param sequenceNumber The message's sequence number.
       */
      JpxFlexMessage Read(Beam::Out<std::uint32_t> sequenceNumber);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<C> m_channel;
      Beam::IO::SharedBuffer m_buffer;
      std::uint32_t m_sequenceNumber;
      JpxFlexPacket m_packet;
      const char* m_source;
      std::size_t m_remainingSize;

      JpxFlexProtocolClient(const JpxFlexProtocolClient&) = delete;
      JpxFlexProtocolClient& operator =(const JpxFlexProtocolClient&) = delete;
  };

  template<typename C>
  template<typename CF>
  JpxFlexProtocolClient<C>::JpxFlexProtocolClient(CF&& channel)
    try : m_channel(std::forward<CF>(channel)),
          m_sequenceNumber(-1),
          m_remainingSize(0) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to initialize the JPX Flex protocol client."));
  }

  template<typename C>
  JpxFlexProtocolClient<C>::~JpxFlexProtocolClient() {
    Close();
  }

  template<typename C>
  JpxFlexMessage JpxFlexProtocolClient<C>::Read() {
    auto sequenceNumber = std::uint32_t();
    return Read(Beam::Store(sequenceNumber));
  }

  template<typename C>
  JpxFlexMessage JpxFlexProtocolClient<C>::Read(
      Beam::Out<std::uint32_t> sequenceNumber) {
    return Beam::TryOrNest([&] {
      if(m_remainingSize == 0) {
        m_buffer.Reset();
        m_channel->GetReader().Read(Beam::Store(m_buffer));
        m_packet = JpxFlexPacket::Parse(m_buffer.GetData(), m_buffer.GetSize());
        m_sequenceNumber = m_packet.m_sequenceNumber;
        m_source = m_packet.m_payload;
        m_remainingSize = m_buffer.GetSize() - JpxFlexPacket::HEADER_LENGTH;
      }
      auto message = JpxFlexMessage::Parse(&m_packet, Beam::Store(m_source),
        Beam::Store(m_remainingSize));
      *sequenceNumber = m_sequenceNumber;
      return message;
    }, Beam::IO::IOException("Failed to read JPX Flex message."));
  }

  template<typename C>
  void JpxFlexProtocolClient<C>::Close() {
    m_channel->GetConnection().Close();
  }
}

#endif
