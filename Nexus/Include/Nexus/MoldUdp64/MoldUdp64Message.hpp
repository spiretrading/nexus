#ifndef NEXUS_MOLD_UDP_64_MESSAGE_HPP
#define NEXUS_MOLD_UDP_64_MESSAGE_HPP
#include <cstdint>
#include <cstring>
#include <string_view>
#include <boost/endian/conversion.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/MoldUdp64/MoldUdp64ParserException.hpp"

namespace Nexus {

  /** A message view, valid while its source buffer is unchanged. */
  struct MoldUdp64Message {

    /** The length of the message length field. */
    static constexpr auto HEADER_LENGTH = sizeof(std::uint16_t);

    /** The length of the payload, excluding the length field. */
    std::uint16_t m_length;

    /** The first byte of the payload. */
    const char* m_data;

    /**
     * Parses the first message in a buffer.
     * @param source The buffer to parse.
     * @return A view of the message.
     */
    static MoldUdp64Message parse(std::string_view source);

    /** Returns the complete application payload. */
    std::string_view get_payload() const;
  };

  inline MoldUdp64Message MoldUdp64Message::parse(std::string_view source) {
    if(source.size() < HEADER_LENGTH) {
      boost::throw_with_location(
        MoldUdp64ParserException("Message too short."));
    }
    auto length = std::uint16_t();
    std::memcpy(&length, source.data(), sizeof(length));
    length = boost::endian::big_to_native(length);
    if(source.size() - HEADER_LENGTH < length) {
      boost::throw_with_location(
        MoldUdp64ParserException("Message too short."));
    }
    return MoldUdp64Message(length, source.data() + HEADER_LENGTH);
  }

  inline std::string_view MoldUdp64Message::get_payload() const {
    return std::string_view(m_data, m_length);
  }
}

#endif
