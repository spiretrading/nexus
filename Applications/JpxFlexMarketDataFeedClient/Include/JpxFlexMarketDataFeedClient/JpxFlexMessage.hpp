#ifndef NEXUS_JPX_FLEX_MESSAGE_HPP
#define NEXUS_JPX_FLEX_MESSAGE_HPP
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <Beam/Utilities/Endian.hpp>
#include <boost/throw_exception.hpp>
#include "JpxFlexMarketDataFeedClient/JpxFlexPacket.hpp"

namespace Nexus::MarketDataService {

  /** Stores a single JPX Flex message. */
  struct JpxFlexMessage {

    /** Enumerates message types. */
    enum class Type : std::uint8_t {

      /** Unknown. */
      UNKNOWN,

      /** Control. */
      CONTROL,

      /** Update no. */
      UPDATE_NUMBER,

      /** Status information. */
      STATUS_INFORMATION,

      /** Open, high, low, and current price information. */
      OPEN_HIGH_LOW_PRICE,

      /** Quote information, ask and bid quotes with highest priority. */
      QUOTE_1,

      /** Quote information (1 quote each above and below quote 1. */
      QUOTE_2,

      /** Quote information (1 quote each above and below quote 2. */
      QUOTE_3,

      /** Quote information (1 quote each above and below quote 3. */
      QUOTE_4,

      /** Quote information (1 quote each above and below quote 4. */
      QUOTE_5,

      /** Quote information (1 quote each above and below quote 5. */
      QUOTE_6,

      /** Quote information (1 quote each above and below quote 6. */
      QUOTE_7,

      /** Quote information (1 quote each above and below quote 7. */
      QUOTE_8,

      /** Quote information (1 quote each above and below quote 8. */
      QUOTE_9,

      /** Quote information (1 quote each above and below quote 9. */
      QUOTE_10,

      /** Market order, (ask and bid quantity). */
      MARKET_ORDER,

      /** Quote information. */
      OVER_UNDER,

      /** Information concerning intraday cumulative trading volume. */
      TRADING_VOLUME,

      /** Information concerning intraday cumulative turnover. */
      TURNOVER,

      /** Information concerning volume weighted average price. */
      VWAP,

      /** Information concerning of parity and estimated price. */
      PARITY,

      /** Yield information. */
      YIELD,

      /** Current price. */
      CURRENT_PRICE,

      /** Ask quote. */
      ASK_QUOTE,

      /** Bid quote. */
      BID_QUOTE,

      /** Sell order effective only during the closing auction. */
      CLOSING_AUCTION_SELL_ORDER,

      /** Buy order effective only during the closing auction. */
      CLOSING_AUCTION_BUY_ORDER
    };

    /** The packet carrying this message. */
    const JpxFlexPacket* m_packet;

    /** The size of the message. */
    std::uint16_t m_size;

    /** The type of message. */
    Type m_type;

    /** The message payload. */
    const char* m_payload;

    /**
     * Parses the type of message.
     * @param source A pointer into the packet referring to the message type.
     */
    static Type ParseType(const char* source);

    /**
     * Parses a single JpxFlexMessage.
     * @param packet The packet carrying the message.
     * @param source A pointer to the first byte in the message.
     * @param size The size of the <i>source</i>.
     */
    static JpxFlexMessage Parse(const JpxFlexPacket* packet,
      Beam::Out<const char*> source, Beam::Out<std::size_t> remainingSize);
  };

  inline bool IsControl(char value) {
    return value >= 0x0 && value <= 0x8 || value == 0xB || value == 0xC ||
      value >= 0xE && value <= 0x1F;
  }

  inline JpxFlexMessage::Type JpxFlexMessage::ParseType(const char* source) {
    if(std::memcmp(source, "NO", 2) == 0) {
      return Type::UPDATE_NUMBER;
    } else if(std::memcmp(source, "ST", 2) == 0) {
      return Type::STATUS_INFORMATION;
    } else if(std::memcmp(source, "1P", 2) == 0) {
      return Type::CURRENT_PRICE;
    } else if(std::memcmp(source, "VL", 2) == 0) {
      return Type::TRADING_VOLUME;
    } else if(std::memcmp(source, "VA", 2) == 0) {
      return Type::TURNOVER;
    } else if(std::memcmp(source, "QS", 2) == 0) {
      return Type::ASK_QUOTE;
    } else if(std::memcmp(source, "QB", 2) == 0) {
      return Type::BID_QUOTE;
    } else if(std::memcmp(source, "SC", 2) == 0) {
      return Type::CLOSING_AUCTION_SELL_ORDER;
    } else if(std::memcmp(source, "BC", 2) == 0) {
      return Type::CLOSING_AUCTION_BUY_ORDER;
    } else if(std::memcmp(source, "LC", 2) == 0) {
      return Type::CONTROL;
    } else if(std::memcmp(source, "4P", 2) == 0) {
      return Type::OPEN_HIGH_LOW_PRICE;
    } else if(std::memcmp(source, "Q1", 2) == 0) {
      return Type::QUOTE_1;
    } else if(std::memcmp(source, "Q2", 2) == 0) {
      return Type::QUOTE_2;
    } else if(std::memcmp(source, "Q3", 2) == 0) {
      return Type::QUOTE_3;
    } else if(std::memcmp(source, "Q4", 2) == 0) {
      return Type::QUOTE_4;
    } else if(std::memcmp(source, "Q5", 2) == 0) {
      return Type::QUOTE_5;
    } else if(std::memcmp(source, "Q6", 2) == 0) {
      return Type::QUOTE_6;
    } else if(std::memcmp(source, "Q7", 2) == 0) {
      return Type::QUOTE_7;
    } else if(std::memcmp(source, "Q8", 2) == 0) {
      return Type::QUOTE_8;
    } else if(std::memcmp(source, "Q9", 2) == 0) {
      return Type::QUOTE_9;
    } else if(std::memcmp(source, "QA", 2) == 0) {
      return Type::QUOTE_10;
    } else if(std::memcmp(source, "QM", 2) == 0) {
      return Type::MARKET_ORDER;
    } else if(std::memcmp(source, "QO", 2) == 0) {
      return Type::OVER_UNDER;
    } else if(std::memcmp(source, "VW", 2) == 0) {
      return Type::VWAP;
    } else if(std::memcmp(source, "PA", 2) == 0) {
      return Type::PARITY;
    } else if(std::memcmp(source, "YI", 2) == 0) {
      return Type::YIELD;
    }
    return Type::UNKNOWN;
  }

  inline JpxFlexMessage JpxFlexMessage::Parse(const JpxFlexPacket* packet,
      Beam::Out<const char*> source, Beam::Out<std::size_t> remainingSize) {
    auto message = JpxFlexMessage();
    message.m_packet = packet;
    message.m_size = 0;
    message.m_type = ParseType(*source);
    *source += 2;
    *remainingSize -= 2;
    message.m_payload = *source;
    while(*remainingSize > 0) {
      --*remainingSize;
      if(!IsControl(**source)) {
        ++message.m_size;
        ++*source;
      } else {
        ++*source;
        break;
      }
    }
    return message;
  }
}

#endif
