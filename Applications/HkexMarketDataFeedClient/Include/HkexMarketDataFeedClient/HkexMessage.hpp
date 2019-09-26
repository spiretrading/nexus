#ifndef NEXUS_HKEX_MESSAGE_HPP
#define NEXUS_HKEX_MESSAGE_HPP
#include <cstdint>
#include <stdexcept>
#include <Beam/Utilities/Endian.hpp>
#include <boost/throw_exception.hpp>
#include "HkexMarketDataFeedClient/HkexPacket.hpp"

namespace Nexus::MarketDataService {

  /** Stores a single HKEX message. */
  struct HkexMessage {

    /** Message code for a trade. */
    static constexpr auto TRADE = 50;

    /** Message code for an aggregate order book update. */
    static constexpr auto BOOK_UPDATE = 53;

    /** The packet carrying this message. */
    const HkexPacket* m_packet;

    /** The size of the message. */
    std::uint16_t m_size;

    /** The type of message. */
    std::uint16_t m_type;

    /** The message payload. */
    const char* m_payload;

    /**
     * Parses a single HkexMessage.
     * @param packet The packet carrying the message.
     * @param source A pointer to the first byte in the message.
     * @param size The size of the <i>source</i>.
     */
    static HkexMessage Parse(const HkexPacket* packet, const char* source,
      std::size_t size);
  };

  inline HkexMessage HkexMessage::Parse(const HkexPacket* packet,
      const char* source, std::size_t size) {
    auto message = HkexMessage();
    if(size < sizeof(std::uint16_t) + sizeof(std::uint16_t)) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Packet too short."));
    }
    message.m_packet = packet;
    message.m_size = Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint16_t*>(source));
    source += sizeof(std::uint16_t);
    message.m_type = Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint16_t*>(source));
    source += sizeof(std::uint16_t);
    message.m_payload = source;
    return message;
  }
}

#endif
