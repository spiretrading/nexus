#ifndef NEXUS_BINARY_SEQUENCE_PROTOCOL_MESSAGE_HPP
#define NEXUS_BINARY_SEQUENCE_PROTOCOL_MESSAGE_HPP
#include <cstdint>
#include <Beam/Utilities/Endian.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocol.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolParserException.hpp"

namespace Nexus::BinarySequenceProtocol {

  /** Stores a message. */
  struct BinarySequenceProtocolMessage {

    /** The length of the message, ignoring this field. */
    std::uint16_t m_length;

    /** The data contents of this message. */
    const char* m_data;

    /**
     * Parses a BinarySequenceProtocolMessage.
     * @param source The first byte in the message to parse.
     * @param size The remaining number of bytes in the packet.
     * @return The BinarySequenceProtocolMessage represented by the
     *         <i>source</i>.
     */
    static BinarySequenceProtocolMessage Parse(const char* source,
      std::size_t size);
  };

  inline BinarySequenceProtocolMessage BinarySequenceProtocolMessage::Parse(
      const char* source, std::size_t size) {
    const auto LENGTH_LENGTH = 2;
    const auto HEADER_LENGTH = LENGTH_LENGTH;
    if(size < HEADER_LENGTH) {
      BOOST_THROW_EXCEPTION(
        BinarySequenceProtocolParserException("Packet too short."));
    }
    auto message = BinarySequenceProtocolMessage();
    message.m_length = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint16_t*>(source));
    if(size < message.m_length + sizeof(message.m_length)) {
      BOOST_THROW_EXCEPTION(
        BinarySequenceProtocolParserException("Packet too short."));
    }
    source += LENGTH_LENGTH;
    message.m_data = source;
    return message;
  }
}

#endif
