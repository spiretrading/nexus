#ifndef NEXUS_BINARYSEQUENCEPROTOCOLPACKET_HPP
#define NEXUS_BINARYSEQUENCEPROTOCOLPACKET_HPP
#include <cstdint>
#include <Beam/Utilities/Endian.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocol.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolParserException.hpp"

namespace Nexus {
namespace BinarySequenceProtocol {

  /*! \struct BinarySequenceProtocolPacket
      \brief Stores a packet.
      \tparam SequenceType The type used to represent sequence numbers.
   */
  template<typename SequenceType>
  struct BinarySequenceProtocolPacket {

    //! The type used to represent sequence numbers.
    using Sequence = SequenceType;

    //! The length of this packet.
    static const auto PACKET_LENGTH = sizeof(Sequence) + sizeof(std::uint16_t);

    //! Sequence number of the first message in this packet.
    Sequence m_sequenceNumber;

    //! The number of messages contained in this packet.
    std::uint16_t m_count;

    //! One or more payload data messages.
    const char* m_payload;

    //! Parses a BinarySequenceProtocolPacket.
    /*!
      \param source The first byte in the packet to parse.
      \param size The remaining number of bytes to parse.
      \return The BinarySequenceProtocolPacket represented by the <i>source</i>.
    */
    static BinarySequenceProtocolPacket Parse(const char* source,
      std::size_t size);
  };

  template<typename SequenceType>
  BinarySequenceProtocolPacket<SequenceType>
      BinarySequenceProtocolPacket<SequenceType>::Parse(const char* source,
      std::size_t size) {
    static const auto SEQUENCE_LENGTH = sizeof(Sequence);
    static const auto COUNT_LENGTH = sizeof(std::uint16_t);
    static const auto HEADER_LENGTH = SEQUENCE_LENGTH + COUNT_LENGTH;
    if(size < HEADER_LENGTH) {
      BOOST_THROW_EXCEPTION(
        BinarySequenceProtocolParserException{"Packet too short."});
    }
    BinarySequenceProtocolPacket packet;
    packet.m_sequenceNumber = Beam::FromBigEndian(
      *reinterpret_cast<const Sequence*>(source));
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
