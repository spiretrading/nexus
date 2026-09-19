#ifndef ASX_TRADE_ITCH_MESSAGE_HPP
#define ASX_TRADE_ITCH_MESSAGE_HPP
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchCursor.hpp"

namespace Nexus {

  /** A view of one ITCH payload, valid while its source buffer is unchanged. */
  struct AsxTradeItchMessage {

    /** The length of the message type field. */
    static constexpr auto HEADER_LENGTH = std::size_t(1);

    /** The length of the message, including its type field. */
    std::size_t m_length;

    /** The type of the message. */
    std::uint8_t m_type;

    /** The first byte of the message's fields. */
    const char* m_payload;

    /**
     * Parses a single ITCH message without its transport framing.
     * @param source The complete message to parse.
     * @return A view of the message.
     */
    static AsxTradeItchMessage parse(std::string_view source);

    /** Returns a cursor positioned at this message's first field. */
    AsxTradeItchCursor get_cursor() const;
  };

  inline AsxTradeItchMessage AsxTradeItchMessage::parse(
      std::string_view source) {
    if(source.size() < HEADER_LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("ITCH message too short."));
    }
    return AsxTradeItchMessage(source.size(),
      static_cast<std::uint8_t>(source.front()), source.data() + HEADER_LENGTH);
  }

  inline AsxTradeItchCursor AsxTradeItchMessage::get_cursor() const {
    if(m_length < HEADER_LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("ITCH message too short."));
    }
    return AsxTradeItchCursor(
      std::string_view(m_payload, m_length - HEADER_LENGTH));
  }
}

#endif
