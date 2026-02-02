#ifndef OTC_LINK_PACKET_HPP
#define OTC_LINK_PACKET_HPP
#include <cstdint>
#include <string_view>
#include <boost/endian/conversion.hpp>
#include <boost/throw_exception.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkParserException.hpp"

namespace Nexus {
  struct OtcLinkMessage;

  /** Stores an OTC Link market data feed packet. */
  struct OtcLinkPacket {

    /** Packet flag bits. */
    enum class Flag : std::uint8_t {

      /** No message in packet, heartbeat only. */
      HEARTBEAT = 0x01,

      /** Sequence numbers are being reset. */
      SEQUENCE_NUMBER_RESET = 0x02,

      /** Packet contains replay messages. */
      REPLAY = 0x40,

      /** Packet contains test messages. */
      TEST = 0x80
    };

    /** The length of this packet's header. */
    static const auto HEADER_LENGTH = std::size_t(12);

    /** Size of packet including header in bytes. */
    std::uint16_t m_size;

    /** Sequence number of packet. */
    std::uint32_t m_sequence_number;

    /** Packet flags. */
    Flag m_flag;

    /** Number of messages in packet. */
    std::uint8_t m_message_count;

    /** Milliseconds since local time midnight (EST/EDT). */
    std::uint32_t m_milliseconds;

    /** Pointer to payload data messages. */
    const char* m_payload;

    /**
     * Parses an OtcLinkPacket.
     * @param source The packet to parse.
     * @return The OtcLinkPacket represented by the <i>source</i>.
     */
    static OtcLinkPacket parse(std::string_view source);
  };

  inline OtcLinkPacket OtcLinkPacket::parse(std::string_view source) {
    if(source.size() < HEADER_LENGTH) {
      boost::throw_with_location(OtcLinkParserException("Packet too short."));
    }
    auto packet = OtcLinkPacket();
    packet.m_size = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint16_t*>(source.data()));
    packet.m_sequence_number = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint32_t*>(source.data() + 2));
    packet.m_flag = static_cast<Flag>(
      *reinterpret_cast<const std::uint8_t*>(source.data() + 6));
    packet.m_message_count =
      *reinterpret_cast<const std::uint8_t*>(source.data() + 7);
    packet.m_milliseconds = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint32_t*>(source.data() + 8));
    packet.m_payload = source.data() + HEADER_LENGTH;
    return packet;
  }
}

#endif
