#ifndef NEXUS_MOLDUDP64PACKET_HPP
#define NEXUS_MOLDUDP64PACKET_HPP
#include <cstdint>
#include <Beam/Utilities/Endian.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/MoldUdp64/MoldUdp64.hpp"
#include "Nexus/MoldUdp64/MoldUdp64ParserException.hpp"

namespace Nexus {
namespace MoldUdp64 {

  /*! \struct MoldUdp64Packet
      \brief Stores a MoldUDP64 packet.
   */
  struct MoldUdp64Packet {

    //! The length of this packet.
    static const auto PACKET_LENGTH = std::size_t{20};

    //! The length of a session field.
    static const auto SESSION_FIELD_LENGTH = std::size_t{10};

    //! Identity of the session the payload relates to.
    Beam::FixedString<SESSION_FIELD_LENGTH> m_session;

    //! Sequence number of the first message in this packet.
    std::uint64_t m_sequenceNumber;

    //! The number of messages contained in this packet.
    std::uint16_t m_count;

    //! One or more payload data messages.
    const char* m_payload;

    //! Parses an MoldUdp64Packet.
    /*!
      \param source The first byte in the packet to parse.
      \param size The remaining number of bytes to parse.
      \return The MoldUdp64Packet represented by the <i>source</i>.
    */
    static MoldUdp64Packet Parse(const char* source, std::size_t size);
  };

  inline MoldUdp64Packet MoldUdp64Packet::Parse(const char* source,
      std::size_t size) {
    static const auto SESSION_LENGTH = 10;
    static const auto SEQUENCE_LENGTH = 8;
    static const auto COUNT_LENGTH = 2;
    static const auto HEADER_LENGTH = SESSION_LENGTH + SEQUENCE_LENGTH +
      COUNT_LENGTH;
    if(size < HEADER_LENGTH) {
      BOOST_THROW_EXCEPTION(MoldUdp64ParserException("Packet too short."));
    }
    MoldUdp64Packet packet;
    packet.m_session = source;
    source += SESSION_LENGTH;
    packet.m_sequenceNumber = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint64_t*>(source));
    source += SEQUENCE_LENGTH;
    packet.m_count = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint16_t*>(source));
    source += COUNT_LENGTH;
    packet.m_payload = source;
    return packet;
  }
}
}

#endif
