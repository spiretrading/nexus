#ifndef NEXUS_MOLD_UDP_64_PACKET_HPP
#define NEXUS_MOLD_UDP_64_PACKET_HPP
#include <cstdint>
#include <string_view>
#include <Beam/Utilities/FixedString.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/MoldUdp64/MoldUdp64ParserException.hpp"

namespace Nexus {

  /** Stores a MoldUDP64 packet. */
  struct MoldUdp64Packet {

    /** The length of this packet. */
    static const auto PACKET_LENGTH = std::size_t(20);

    /** The length of a session field. */
    static const auto SESSION_FIELD_LENGTH = std::size_t(10);

    /** Identity of the session the payload relates to. */
    Beam::FixedString<SESSION_FIELD_LENGTH> m_session;

    /** Sequence number of the first message in this packet. */
    std::uint64_t m_sequence_number;

    /** The number of messages contained in this packet. */
    std::uint16_t m_count;

    /** One or more payload data messages. */
    const char* m_payload;

    /**
     * Parses an MoldUdp64Packet.
     * @param source The packet to parse.
     * @return The MoldUdp64Packet represented by the <i>source</i>.
     */
    static MoldUdp64Packet parse(std::string_view source);
  };

  inline MoldUdp64Packet MoldUdp64Packet::parse(std::string_view source) {
    static const auto SESSION_LENGTH = 10;
    static const auto SEQUENCE_LENGTH = 8;
    static const auto COUNT_LENGTH = 2;
    static const auto HEADER_LENGTH =
      SESSION_LENGTH + SEQUENCE_LENGTH + COUNT_LENGTH;
    if(source.size() < HEADER_LENGTH) {
      boost::throw_with_location(MoldUdp64ParserException("Packet too short."));
    }
    auto packet = MoldUdp64Packet();
    packet.m_session = source.data();
    packet.m_sequence_number = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint64_t*>(source.data() + SESSION_LENGTH));
    packet.m_count = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint16_t*>(
        source.data() + SESSION_LENGTH + SEQUENCE_LENGTH));
    packet.m_payload = source.data() + HEADER_LENGTH;
    return packet;
  }
}

#endif
