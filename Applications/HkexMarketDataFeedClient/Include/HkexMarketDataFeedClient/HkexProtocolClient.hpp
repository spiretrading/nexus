#ifndef NEXUS_HKEX_PROTOCOL_CLIENT_HPP
#define NEXUS_HKEX_PROTOCOL_CLIENT_HPP
#include <cstdint>
#include <utility>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Utilities/Expect.hpp>
#include "HkexMarketDataFeedClient/HkexMessage.hpp"
#include "HkexMarketDataFeedClient/HkexPacket.hpp"

namespace Nexus::MarketDataService {

  /**
   * Parses packets from the HKEX market data feed.
   * @param <C> The type of channel receiving the packets to parse.
   */
  template<typename C>
  class HkexProtocolClient {
    public:

      /** The type of channel receiving messages. */
      using Channel = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a HkexProtocolClient.
       * @param channel The channel receiving the packets to parse.
       */
      template<typename CF>
      explicit HkexProtocolClient(CF&& channel);

      ~HkexProtocolClient();

      /** Reads the next message from the feed. */
      HkexMessage Read();

      /**
       * Reads the next message from the feed.
       * @param sequenceNumber The message's sequence number.
       */
      HkexMessage Read(Beam::Out<std::uint32_t> sequenceNumber);

      void Open();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<C> m_channel;
      Beam::IO::SharedBuffer m_buffer;
      std::uint32_t m_sequenceNumber;
      HkexPacket m_packet;
      const char* m_source;
      std::size_t m_remainingSize;

      HkexProtocolClient(const HkexProtocolClient&) = delete;
      HkexProtocolClient& operator =(const HkexProtocolClient&) = delete;
  };

  template<typename C>
  template<typename CF>
  HkexProtocolClient<C>::HkexProtocolClient(CF&& channel)
    try : m_channel(std::forward<CF>(channel)),
          m_sequenceNumber(-1),
          m_remainingSize(0) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to initialize the HKEX protocol client."));
  }

  template<typename C>
  HkexProtocolClient<C>::~HkexProtocolClient() {
    Close();
  }

  template<typename C>
  HkexMessage HkexProtocolClient<C>::Read() {
    auto sequenceNumber = std::uint32_t();
    return Read(Beam::Store(sequenceNumber));
  }

  template<typename C>
  HkexMessage HkexProtocolClient<C>::Read(
      Beam::Out<std::uint32_t> sequenceNumber) {
    return Beam::TryOrNest([&] {
      if(m_sequenceNumber == -1 ||
          m_sequenceNumber == m_packet.m_sequenceNumber + m_packet.m_count) {
        while(true) {
          m_buffer.Reset();
          m_channel->GetReader().Read(Beam::Store(m_buffer));
          m_packet = HkexPacket::Parse(m_buffer.GetData(), m_buffer.GetSize());
          if(m_packet.m_count != 0) {
            m_sequenceNumber = m_packet.m_sequenceNumber;
            m_source = m_packet.m_payload;
            m_remainingSize = m_buffer.GetSize() - HkexPacket::HEADER_LENGTH;
            break;
          }
        }
      }
      auto message = HkexMessage::Parse(&m_packet, m_source, m_remainingSize);
      m_remainingSize -= message.m_size;
      m_source += message.m_size;
      *sequenceNumber = m_sequenceNumber;
      ++m_sequenceNumber;
      return message;
    }, Beam::IO::IOException("Failed to read HKEX message."));
  }

  template<typename C>
  void HkexProtocolClient<C>::Open() {
    m_channel->GetConnection().Open();
  }

  template<typename C>
  void HkexProtocolClient<C>::Close() {
    m_channel->GetConnection().Close();
  }
}

#endif
