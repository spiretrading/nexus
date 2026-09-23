#ifndef OTC_LINK_MESSAGE_HPP
#define OTC_LINK_MESSAGE_HPP
#include "OtcLinkMarketDataFeedClient/OtcLinkCursor.hpp"

namespace Nexus {

  /** An OTC Link message view, valid while its source is unchanged. */
  struct OtcLinkMessage {

    /** The length of a message's length and type fields. */
    static constexpr auto HEADER_LENGTH = std::size_t(3);

    /** The total message length, including the message header. */
    std::uint16_t m_length;

    /** The message's wire type identifier. */
    std::uint8_t m_type;

    /** The message fields following its header. */
    std::string_view m_payload;

    /**
     * Parses the first message in a buffer.
     * @param source The buffer beginning with the message.
     */
    static OtcLinkMessage parse(std::string_view source);

    /** Returns a cursor positioned at this message's first field. */
    OtcLinkCursor get_cursor() const;
  };

  inline OtcLinkMessage OtcLinkMessage::parse(std::string_view source) {
    auto cursor = OtcLinkCursor(source);
    auto message = OtcLinkMessage();
    message.m_length = cursor.read_uint16();
    message.m_type = cursor.read_uint8();
    if(message.m_length < HEADER_LENGTH) {
      boost::throw_with_location(
        OtcLinkParserException("OTC Link message length too short."));
    }
    message.m_payload = cursor.read_bytes(message.m_length - HEADER_LENGTH);
    return message;
  }

  inline OtcLinkCursor OtcLinkMessage::get_cursor() const {
    return OtcLinkCursor(m_payload);
  }
}

#endif
