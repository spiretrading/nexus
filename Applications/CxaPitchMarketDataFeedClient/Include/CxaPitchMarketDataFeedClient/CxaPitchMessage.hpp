#ifndef CXA_PITCH_MESSAGE_HPP
#define CXA_PITCH_MESSAGE_HPP
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string_view>
#include <boost/throw_exception.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchCursor.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"

namespace Nexus {

  /** Stores a single CXA PITCH message. */
  struct CxaPitchMessage {

    /** The length of a message's length and type fields. */
    static constexpr auto HEADER_LENGTH = std::size_t(2);

    /** The length of this message, including its length and type fields. */
    std::uint8_t m_length;

    /** The type of this message. */
    std::uint8_t m_type;

    /** The first byte of this message's fields. */
    const char* m_payload;

    /**
     * Parses a CxaPitchMessage.
     * @param source The buffer to parse.
     * @return The CxaPitchMessage represented by the <i>source</i>.
     */
    static CxaPitchMessage parse(std::string_view source);

    /** Returns a cursor positioned at this message's first field. */
    CxaPitchCursor get_cursor() const;
  };

  inline CxaPitchMessage CxaPitchMessage::parse(std::string_view source) {
    if(source.size() < HEADER_LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("PITCH message too short."));
    }
    auto cursor = CxaPitchCursor(source.data());
    auto message = CxaPitchMessage();
    message.m_length = cursor.read_uint8();
    message.m_type = cursor.read_uint8();
    if(message.m_length < HEADER_LENGTH || message.m_length > source.size()) {
      boost::throw_with_location(
        CxaPitchParserException("PITCH message length out of range."));
    }
    message.m_payload = source.data() + HEADER_LENGTH;
    return message;
  }

  inline CxaPitchCursor CxaPitchMessage::get_cursor() const {
    return CxaPitchCursor(m_payload);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const CxaPitchMessage& message) {
    static const auto DIGITS = std::string_view("0123456789ABCDEF");
    return out << "(unknown 0x" << DIGITS[message.m_type >> 4] <<
      DIGITS[message.m_type & 0x0F] << ' ' <<
      static_cast<int>(message.m_length) << ')';
  }
}

#endif
