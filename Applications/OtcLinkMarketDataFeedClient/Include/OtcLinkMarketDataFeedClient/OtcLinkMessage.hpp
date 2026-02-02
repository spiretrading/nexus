#ifndef OTC_LINK_MESSAGE_HPP
#define OTC_LINK_MESSAGE_HPP
#include <cstdint>
#include <string_view>
#include <boost/endian/conversion.hpp>
#include <boost/throw_exception.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkParserException.hpp"

namespace Nexus {

  /** Stores an OTC Link market data feed message. */
  struct OtcLinkMessage {

    /** Message type identifiers. */
    enum class Type : std::uint8_t {

      /** Quote add, delete, or spin message. */
      QUOTE = 1,

      /** Quote update message. */
      QUOTE_UPDATE = 2,

      /** Inside add, delete, or spin message. */
      INSIDE = 3,

      /** Inside update message. */
      INSIDE_UPDATE = 4,

      /** Reference price add, delete, or spin message. */
      REFERENCE_PRICE = 7,

      /** Reference price update message. */
      REFERENCE_PRICE_UPDATE = 8,

      /** Security reference message. */
      SECURITY_REFERENCE = 9,

      /** Start of spin message. */
      START_OF_SPIN = 11,

      /** End of spin message. */
      END_OF_SPIN = 12,

      /** Market open message. */
      MARKET_OPEN = 13,

      /** Market close message. */
      MARKET_CLOSE = 14,

      /** Extended security message. */
      EXTENDED_SECURITY = 15,

      /** Extended security without CUSIP message. */
      EXTENDED_SECURITY_NO_CUSIP = 16,

      /** Trade message. */
      TRADE = 17
    };

    /** The length of this message's header. */
    static const auto HEADER_LENGTH = std::size_t(3);

    /** The size of the message in bytes. */
    std::uint16_t m_size;

    /** The message type. */
    Type m_type;

    /** Pointer to message payload. */
    const char* m_payload;

    /**
     * Parses an OtcLinkMessage.
     * @param source The message to parse.
     * @return The OtcLinkMessage represented by the <i>source</i>.
     */
    static OtcLinkMessage parse(std::string_view source);
  };

  inline OtcLinkMessage OtcLinkMessage::parse(std::string_view source) {
    if(source.size() < HEADER_LENGTH) {
      boost::throw_with_location(OtcLinkParserException("Message too short."));
    }
    auto message = OtcLinkMessage();
    message.m_size = boost::endian::big_to_native(
      *reinterpret_cast<const std::uint16_t*>(source.data()));
    message.m_type = static_cast<Type>(
      *reinterpret_cast<const std::uint8_t*>(source.data() + 2));
    message.m_payload = source.data() + HEADER_LENGTH;
    return message;
  }
}

#endif
