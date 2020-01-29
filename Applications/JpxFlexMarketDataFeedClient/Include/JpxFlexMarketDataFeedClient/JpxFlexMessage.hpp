#ifndef NEXUS_JPX_FLEX_MESSAGE_HPP
#define NEXUS_JPX_FLEX_MESSAGE_HPP
#include <cstdint>
#include <stdexcept>
#include <Beam/Utilities/Endian.hpp>
#include <boost/throw_exception.hpp>
#include "JpxFlexMarketDataFeedClient/JpxFlexPacket.hpp"

namespace Nexus::MarketDataService {

  /** Stores a single JPX Flex message. */
  struct JpxFlexMessage {

    /** The packet carrying this message. */
    const JpxFlexPacket* m_packet;

    /** The size of the message. */
    std::uint16_t m_size;

    /** The type of message. */
    std::uint16_t m_type;

    /** The message payload. */
    const char* m_payload;

    /**
     * Parses a single JpxFlexMessage.
     * @param packet The packet carrying the message.
     * @param source A pointer to the first byte in the message.
     * @param size The size of the <i>source</i>.
     */
    static JpxFlexMessage Parse(const JpxFlexPacket* packet, const char* source,
      std::size_t size);
  };

  inline JpxFlexMessage JpxFlexMessage::Parse(const JpxFlexPacket* packet,
      const char* source, std::size_t size) {
    auto message = JpxFlexMessage();
    message.m_packet = packet;
    message.m_size = static_cast<std::uint16_t>(size);
    message.m_type = 0;
    message.m_payload = packet->m_payload;
    return message;
  }
}

#endif
