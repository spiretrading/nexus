#ifndef NEXUS_MOLD_UDP_64_MESSAGE_HPP
#define NEXUS_MOLD_UDP_64_MESSAGE_HPP
#include <cstdint>
#include <string_view>
#include <boost/endian/conversion.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/MoldUdp64/MoldUdp64ParserException.hpp"

namespace Nexus {

  /** Stores a MoldUdp64 message. */
  struct MoldUdp64Message {

    /** The length of the message, ignoring this field. */
    std::uint16_t m_length;

    /** The code identifying this message type. */
    std::uint8_t m_message_type;

    /** The data contents of this message. */
    const char* m_data;

    /**
     * Parses a MoldUdp64Message.
     * @param source The message packet to parse.
     * @return The MoldUdp64Message represented by the <i>source</i>.
     */
    static MoldUdp64Message parse(std::string_view source);
  };

  inline MoldUdp64Message MoldUdp64Message::parse(std::string_view source) {
    static const auto LENGTH_LENGTH = 2;
    static const auto TYPE_LENGTH = 1;
    static const auto HEADER_LENGTH = LENGTH_LENGTH + TYPE_LENGTH;
    if(source.size() < HEADER_LENGTH) {
      boost::throw_with_location(MoldUdp64ParserException("Packet too short."));
    }
    auto message = MoldUdp64Message();
    message.m_length = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint16_t*>(source.data()));
    if(source.size() < message.m_length + sizeof(message.m_length)) {
      boost::throw_with_location(MoldUdp64ParserException("Packet too short."));
    }
    message.m_message_type = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint8_t*>(source.data() + LENGTH_LENGTH));
    message.m_data = source.data() + HEADER_LENGTH;
    return message;
  }
}

#endif
