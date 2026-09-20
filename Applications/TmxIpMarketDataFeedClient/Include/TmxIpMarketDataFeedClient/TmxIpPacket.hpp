#ifndef TMX_IP_PACKET_HPP
#define TMX_IP_PACKET_HPP
#include "Nexus/Stamp/StampMessage.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpHeader.hpp"

namespace Nexus {

  /** A TMX IP packet view, valid while its source buffer is unchanged. */
  struct TmxIpPacket {

    /** The character introducing a packet. */
    static constexpr auto START = '\x02';

    /** The character terminating a packet. */
    static constexpr auto END = '\x03';

    /** The packet's transport header. */
    TmxIpHeader m_header;

    /** The business message, fragment, or heartbeat body. */
    std::string_view m_payload;

    /**
     * Parses one complete framed packet without interpreting its payload.
     * @param source The packet including STX and ETX.
     */
    static TmxIpPacket parse(std::string_view source);
  };

  /** Parses a standalone business packet's STAMP message. */
  inline StampMessage parse_message(const TmxIpPacket& packet) {
    if(!packet.m_header.m_sequence ||
        packet.m_header.m_continuation != TmxIpHeader::Continuation::NONE) {
      boost::throw_with_location(
        TmxIpParserException("Packet is not a complete STAMP message."));
    }
    return StampMessage::parse(packet.m_payload);
  }

  inline TmxIpPacket TmxIpPacket::parse(std::string_view source) {
    constexpr auto FRAMING_LENGTH = sizeof(START) + sizeof(END);
    if(source.size() < TmxIpHeader::LENGTH + FRAMING_LENGTH) {
      boost::throw_with_location(
        TmxIpParserException("TMX IP packet too short."));
    }
    if(source.front() != START || source.back() != END) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP packet framing."));
    }
    source.remove_prefix(sizeof(START));
    source.remove_suffix(sizeof(END));
    auto header = TmxIpHeader::parse(source);
    if(source.size() != header.m_length) {
      boost::throw_with_location(
        TmxIpParserException("TMX IP packet length does not match header."));
    }
    return TmxIpPacket(header, source.substr(TmxIpHeader::LENGTH));
  }
}

#endif
