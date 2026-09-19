#ifndef ASX_TRADE_ITCH_MESSAGES_HPP
#define ASX_TRADE_ITCH_MESSAGES_HPP
#include <array>
#include <charconv>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchMessage.hpp"

namespace Nexus {
namespace AsxTradeItchDetails {
  constexpr auto SYMBOL_LENGTH = 32;
  constexpr auto LONG_NAME_LENGTH = 32;
  constexpr auto ISIN_LENGTH = 12;
  constexpr auto CURRENCY_LENGTH = 3;
  constexpr auto PARTICIPANT_ID_LENGTH = 7;
  constexpr auto NANOSECONDS_PER_SECOND = 1000000000;
}

  /**
   * A 96 bit match identifier stored as three words, most significant first.
   */
  using AsxTradeItchMatchId = std::array<std::uint32_t, 3>;

  /** Describes a leg of a combination order book. */
  struct AsxTradeItchCombinationLeg {

    /** The instrument series name of the leg. */
    std::string m_symbol;

    /** B for buy, C for sell, or ? for an unused leg. */
    char m_side;

    /** The relative number of contracts in the leg. */
    std::uint32_t m_ratio;
  };

  /** Stores an ASX Trade ITCH seconds message. */
  struct AsxTradeItchSeconds {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('T');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(5);

    /** The Unix time in seconds. */
    std::uint32_t m_seconds;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchSeconds parse(const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH order book directory message. */
  struct AsxTradeItchOrderBookDirectory {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('R');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(113);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The identifier of the order book. */
    std::uint32_t m_order_book_id;

    /** The instrument series name. */
    std::string m_symbol;

    /** The descriptive instrument name. */
    std::string m_long_name;

    /** The ISIN identifying the instrument. */
    std::string m_isin;

    /** The exchange financial product classification. */
    std::uint8_t m_financial_product;

    /** The trading currency code. */
    std::string m_currency;

    /** The number of decimal places in wire prices. */
    std::uint16_t m_price_decimals;

    /** The number of decimal places in the nominal value. */
    std::uint16_t m_nominal_value_decimals;

    /** The number of securities in an odd lot, zero if undefined. */
    std::uint32_t m_odd_lot_size;

    /** The number of securities in a round lot. */
    std::uint32_t m_round_lot_size;

    /** The number of securities in a block lot, zero if undefined. */
    std::uint32_t m_block_lot_size;

    /** The nominal value in its declared decimal scale. */
    std::uint64_t m_nominal_value;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchOrderBookDirectory parse(
      const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH combination order book directory message. */
  struct AsxTradeItchCombinationOrderBookDirectory {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('M');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(261);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The identifier of the order book. */
    std::uint32_t m_order_book_id;

    /** The instrument series name. */
    std::string m_symbol;

    /** The descriptive instrument name. */
    std::string m_long_name;

    /** The ISIN identifying the instrument. */
    std::string m_isin;

    /** The exchange financial product classification. */
    std::uint8_t m_financial_product;

    /** The trading currency code. */
    std::string m_currency;

    /** The number of decimal places in wire prices. */
    std::uint16_t m_price_decimals;

    /** The number of decimal places in the nominal value. */
    std::uint16_t m_nominal_value_decimals;

    /** The number of securities in an odd lot, zero if undefined. */
    std::uint32_t m_odd_lot_size;

    /** The number of securities in a round lot. */
    std::uint32_t m_round_lot_size;

    /** The number of securities in a block lot, zero if undefined. */
    std::uint32_t m_block_lot_size;

    /** The nominal value in its declared decimal scale. */
    std::uint64_t m_nominal_value;

    /** The four leg slots of the combination. */
    std::array<AsxTradeItchCombinationLeg, 4> m_legs;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchCombinationOrderBookDirectory parse(
      const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH tick size table entry message. */
  struct AsxTradeItchTickSize {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('L');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(25);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The identifier of the order book. */
    std::uint32_t m_order_book_id;

    /** The tick size in wire units. */
    std::uint64_t m_tick_size;

    /** The first price in the range, in wire units. */
    std::int32_t m_price_from;

    /** The last price in the range, in wire units. */
    std::int32_t m_price_to;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchTickSize parse(const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH system event message. */
  struct AsxTradeItchSystemEvent {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('S');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(6);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The system event code. */
    char m_event_code;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchSystemEvent parse(const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH order book state message. */
  struct AsxTradeItchOrderBookState {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('O');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(29);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The identifier of the order book. */
    std::uint32_t m_order_book_id;

    /** The name of the trading state. */
    std::string m_state;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchOrderBookState parse(const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH add order message. */
  struct AsxTradeItchAddOrder {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('A');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(37);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The order identifier, scoped to its order book and side. */
    std::uint64_t m_order_id;

    /** The identifier of the order book. */
    std::uint32_t m_order_book_id;

    /** The side of the order. */
    Side m_side;

    /** The rank of the order within its side of the book. */
    std::uint32_t m_order_book_position;

    /** The visible quantity, zero if undisclosed. */
    std::uint64_t m_quantity;

    /** The display price in wire units. */
    std::int32_t m_price;

    /** The bitmap of exchange order attributes. */
    std::uint16_t m_exchange_order_type;

    /** The lot classification of the order. */
    std::uint8_t m_lot_type;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchAddOrder parse(const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH add order with participant message. */
  struct AsxTradeItchAddOrderWithParticipant {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('F');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(44);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The order identifier, scoped to its order book and side. */
    std::uint64_t m_order_id;

    /** The identifier of the order book. */
    std::uint32_t m_order_book_id;

    /** The side of the order. */
    Side m_side;

    /** The rank of the order within its side of the book. */
    std::uint32_t m_order_book_position;

    /** The visible quantity, zero if undisclosed. */
    std::uint64_t m_quantity;

    /** The display price in wire units. */
    std::int32_t m_price;

    /** The bitmap of exchange order attributes. */
    std::uint16_t m_exchange_order_type;

    /** The lot classification of the order. */
    std::uint8_t m_lot_type;

    /** The participant entering the order. */
    std::string m_participant_id;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchAddOrderWithParticipant parse(
      const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH order executed message. */
  struct AsxTradeItchOrderExecuted {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('E');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(52);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The order identifier, scoped to its order book and side. */
    std::uint64_t m_order_id;

    /** The identifier of the order book. */
    std::uint32_t m_order_book_id;

    /** The side of the order. */
    Side m_side;

    /** The quantity executed in this match. */
    std::uint64_t m_executed_quantity;

    /** The identifier of the match. */
    AsxTradeItchMatchId m_match_id;

    /** The participant owning the order, empty if anonymous. */
    std::string m_owner;

    /** The counterparty participant, empty if anonymous. */
    std::string m_counterparty;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchOrderExecuted parse(const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH order executed at price message. */
  struct AsxTradeItchOrderExecutedAtPrice {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('C');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(58);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The order identifier, scoped to its order book and side. */
    std::uint64_t m_order_id;

    /** The identifier of the order book. */
    std::uint32_t m_order_book_id;

    /** The side of the order. */
    Side m_side;

    /** The quantity executed in this match. */
    std::uint64_t m_executed_quantity;

    /** The identifier of the match. */
    AsxTradeItchMatchId m_match_id;

    /** The participant owning the order, empty if anonymous. */
    std::string m_owner;

    /** The counterparty participant, empty if anonymous. */
    std::string m_counterparty;

    /** The execution price in wire units. */
    std::int32_t m_price;

    /** Whether the execution occurred in an auction (Y or N). */
    char m_occurred_at_cross;

    /** Whether the execution contributes to trade statistics (Y or N). */
    char m_printable;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchOrderExecutedAtPrice parse(
      const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH order replace message. */
  struct AsxTradeItchOrderReplace {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('U');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(36);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The order identifier, scoped to its order book and side. */
    std::uint64_t m_order_id;

    /** The identifier of the order book. */
    std::uint32_t m_order_book_id;

    /** The side of the order. */
    Side m_side;

    /** The rank of the order within its side of the book. */
    std::uint32_t m_order_book_position;

    /** The visible quantity, zero if undisclosed. */
    std::uint64_t m_quantity;

    /** The display price in wire units. */
    std::int32_t m_price;

    /** The bitmap of exchange order attributes. */
    std::uint16_t m_exchange_order_type;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchOrderReplace parse(const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH order delete message. */
  struct AsxTradeItchOrderDelete {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('D');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(18);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The order identifier, scoped to its order book and side. */
    std::uint64_t m_order_id;

    /** The identifier of the order book. */
    std::uint32_t m_order_book_id;

    /** The side of the order. */
    Side m_side;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchOrderDelete parse(const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH trade message. */
  struct AsxTradeItchTrade {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('P');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(50);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The identifier of the match. */
    AsxTradeItchMatchId m_match_id;

    /** The matched side, or NONE when unspecified. */
    Side m_side;

    /** The traded quantity. */
    std::uint64_t m_quantity;

    /** The identifier of the order book. */
    std::uint32_t m_order_book_id;

    /** The trade price in wire units. */
    std::int32_t m_price;

    /** The participant owning the order, empty if anonymous. */
    std::string m_owner;

    /** The counterparty participant, empty if anonymous. */
    std::string m_counterparty;

    /** Whether the execution contributes to trade statistics (Y or N). */
    char m_printable;

    /** Whether the execution occurred in an auction (Y or N). */
    char m_occurred_at_cross;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchTrade parse(const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH equilibrium price update message. */
  struct AsxTradeItchEquilibriumPriceUpdate {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('Z');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(53);

    /** The nanoseconds since the most recent seconds message. */
    std::uint32_t m_nanoseconds;

    /** The identifier of the order book. */
    std::uint32_t m_order_book_id;

    /** The bid quantity available for execution. */
    std::uint64_t m_bid_quantity;

    /** The ask quantity available for execution. */
    std::uint64_t m_ask_quantity;

    /** The auction price in wire units, or INT32_MIN if unavailable. */
    std::int32_t m_equilibrium_price;

    /** The best bid price in wire units, or INT32_MIN if unavailable. */
    std::int32_t m_best_bid_price;

    /** The best ask price in wire units, or INT32_MIN if unavailable. */
    std::int32_t m_best_ask_price;

    /** The quantity at the best bid price. */
    std::uint64_t m_best_bid_quantity;

    /** The quantity at the best ask price. */
    std::uint64_t m_best_ask_quantity;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchEquilibriumPriceUpdate parse(
      const AsxTradeItchMessage& message);
  };

  /** Stores an ASX Trade ITCH end of snapshot message. */
  struct AsxTradeItchEndOfSnapshot {

    /** The message type. */
    static constexpr auto TYPE = std::uint8_t('G');

    /** The smallest valid message length, including the type field. */
    static constexpr auto LENGTH = std::size_t(21);

    /** The first ITCH sequence to apply after this snapshot. */
    std::uint64_t m_sequence;

    /**
     * Parses the message's fields.
     * @param message The message to parse.
     * @return The decoded message.
     */
    static AsxTradeItchEndOfSnapshot parse(const AsxTradeItchMessage& message);
  };

  inline AsxTradeItchSeconds AsxTradeItchSeconds::parse(
      const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchSeconds();
    value.m_seconds = cursor.read_uint32();
    return value;
  }

  inline AsxTradeItchOrderBookDirectory AsxTradeItchOrderBookDirectory::parse(
      const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchOrderBookDirectory();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    value.m_order_book_id = cursor.read_uint32();
    value.m_symbol = cursor.read_text(AsxTradeItchDetails::SYMBOL_LENGTH);
    value.m_long_name = cursor.read_text(AsxTradeItchDetails::LONG_NAME_LENGTH);
    value.m_isin = cursor.read_text(AsxTradeItchDetails::ISIN_LENGTH);
    value.m_financial_product = cursor.read_uint8();
    value.m_currency = cursor.read_text(AsxTradeItchDetails::CURRENCY_LENGTH);
    value.m_price_decimals = cursor.read_uint16();
    value.m_nominal_value_decimals = cursor.read_uint16();
    value.m_odd_lot_size = cursor.read_uint32();
    value.m_round_lot_size = cursor.read_uint32();
    value.m_block_lot_size = cursor.read_uint32();
    value.m_nominal_value = cursor.read_uint64();
    return value;
  }

  inline AsxTradeItchCombinationOrderBookDirectory
      AsxTradeItchCombinationOrderBookDirectory::parse(
        const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchCombinationOrderBookDirectory();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    value.m_order_book_id = cursor.read_uint32();
    value.m_symbol = cursor.read_text(AsxTradeItchDetails::SYMBOL_LENGTH);
    value.m_long_name = cursor.read_text(AsxTradeItchDetails::LONG_NAME_LENGTH);
    value.m_isin = cursor.read_text(AsxTradeItchDetails::ISIN_LENGTH);
    value.m_financial_product = cursor.read_uint8();
    value.m_currency = cursor.read_text(AsxTradeItchDetails::CURRENCY_LENGTH);
    value.m_price_decimals = cursor.read_uint16();
    value.m_nominal_value_decimals = cursor.read_uint16();
    value.m_odd_lot_size = cursor.read_uint32();
    value.m_round_lot_size = cursor.read_uint32();
    value.m_block_lot_size = cursor.read_uint32();
    value.m_nominal_value = cursor.read_uint64();
    for(auto& leg : value.m_legs) {
      leg.m_symbol = cursor.read_text(AsxTradeItchDetails::SYMBOL_LENGTH);
      leg.m_side = cursor.read_char();
      leg.m_ratio = cursor.read_uint32();
      if(leg.m_side != 'B' && leg.m_side != 'C' && leg.m_side != '?') {
        boost::throw_with_location(
          AsxTradeItchParserException("Invalid combination leg side."));
      }
    }
    return value;
  }

  inline AsxTradeItchTickSize AsxTradeItchTickSize::parse(
      const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchTickSize();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    value.m_order_book_id = cursor.read_uint32();
    value.m_tick_size = cursor.read_uint64();
    value.m_price_from = cursor.read_price();
    value.m_price_to = cursor.read_price();
    return value;
  }

  inline AsxTradeItchSystemEvent AsxTradeItchSystemEvent::parse(
      const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchSystemEvent();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    value.m_event_code = cursor.read_char();
    return value;
  }

  inline AsxTradeItchOrderBookState AsxTradeItchOrderBookState::parse(
      const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchOrderBookState();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    value.m_order_book_id = cursor.read_uint32();
    static constexpr auto STATE_LENGTH = 20;
    value.m_state = cursor.read_text(STATE_LENGTH);
    return value;
  }

  inline AsxTradeItchAddOrder AsxTradeItchAddOrder::parse(
      const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchAddOrder();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    value.m_order_id = cursor.read_uint64();
    value.m_order_book_id = cursor.read_uint32();
    value.m_side = cursor.read_side();
    if(value.m_side == Side::NONE) {
      boost::throw_with_location(
        AsxTradeItchParserException("Missing order side."));
    }
    value.m_order_book_position = cursor.read_uint32();
    value.m_quantity = cursor.read_uint64();
    value.m_price = cursor.read_price();
    value.m_exchange_order_type = cursor.read_uint16();
    value.m_lot_type = cursor.read_uint8();
    return value;
  }

  inline AsxTradeItchAddOrderWithParticipant
      AsxTradeItchAddOrderWithParticipant::parse(
        const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchAddOrderWithParticipant();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    value.m_order_id = cursor.read_uint64();
    value.m_order_book_id = cursor.read_uint32();
    value.m_side = cursor.read_side();
    if(value.m_side == Side::NONE) {
      boost::throw_with_location(
        AsxTradeItchParserException("Missing order side."));
    }
    value.m_order_book_position = cursor.read_uint32();
    value.m_quantity = cursor.read_uint64();
    value.m_price = cursor.read_price();
    value.m_exchange_order_type = cursor.read_uint16();
    value.m_lot_type = cursor.read_uint8();
    value.m_participant_id =
      cursor.read_text(AsxTradeItchDetails::PARTICIPANT_ID_LENGTH);
    return value;
  }

  inline AsxTradeItchOrderExecuted AsxTradeItchOrderExecuted::parse(
      const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchOrderExecuted();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    value.m_order_id = cursor.read_uint64();
    value.m_order_book_id = cursor.read_uint32();
    value.m_side = cursor.read_side();
    if(value.m_side == Side::NONE) {
      boost::throw_with_location(
        AsxTradeItchParserException("Missing order side."));
    }
    value.m_executed_quantity = cursor.read_uint64();
    for(auto& word : value.m_match_id) {
      word = cursor.read_uint32();
    }
    value.m_owner =
      cursor.read_text(AsxTradeItchDetails::PARTICIPANT_ID_LENGTH);
    value.m_counterparty =
      cursor.read_text(AsxTradeItchDetails::PARTICIPANT_ID_LENGTH);
    return value;
  }

  inline AsxTradeItchOrderExecutedAtPrice
      AsxTradeItchOrderExecutedAtPrice::parse(
        const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchOrderExecutedAtPrice();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    value.m_order_id = cursor.read_uint64();
    value.m_order_book_id = cursor.read_uint32();
    value.m_side = cursor.read_side();
    if(value.m_side == Side::NONE) {
      boost::throw_with_location(
        AsxTradeItchParserException("Missing order side."));
    }
    value.m_executed_quantity = cursor.read_uint64();
    for(auto& word : value.m_match_id) {
      word = cursor.read_uint32();
    }
    value.m_owner =
      cursor.read_text(AsxTradeItchDetails::PARTICIPANT_ID_LENGTH);
    value.m_counterparty =
      cursor.read_text(AsxTradeItchDetails::PARTICIPANT_ID_LENGTH);
    value.m_price = cursor.read_price();
    value.m_occurred_at_cross = cursor.read_char();
    value.m_printable = cursor.read_char();
    return value;
  }

  inline AsxTradeItchOrderReplace AsxTradeItchOrderReplace::parse(
      const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchOrderReplace();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    value.m_order_id = cursor.read_uint64();
    value.m_order_book_id = cursor.read_uint32();
    value.m_side = cursor.read_side();
    if(value.m_side == Side::NONE) {
      boost::throw_with_location(
        AsxTradeItchParserException("Missing order side."));
    }
    value.m_order_book_position = cursor.read_uint32();
    value.m_quantity = cursor.read_uint64();
    value.m_price = cursor.read_price();
    value.m_exchange_order_type = cursor.read_uint16();
    return value;
  }

  inline AsxTradeItchOrderDelete AsxTradeItchOrderDelete::parse(
      const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchOrderDelete();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    value.m_order_id = cursor.read_uint64();
    value.m_order_book_id = cursor.read_uint32();
    value.m_side = cursor.read_side();
    if(value.m_side == Side::NONE) {
      boost::throw_with_location(
        AsxTradeItchParserException("Missing order side."));
    }
    return value;
  }

  inline AsxTradeItchTrade AsxTradeItchTrade::parse(
      const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchTrade();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    for(auto& word : value.m_match_id) {
      word = cursor.read_uint32();
    }
    value.m_side = cursor.read_side();
    value.m_quantity = cursor.read_uint64();
    value.m_order_book_id = cursor.read_uint32();
    value.m_price = cursor.read_price();
    value.m_owner =
      cursor.read_text(AsxTradeItchDetails::PARTICIPANT_ID_LENGTH);
    value.m_counterparty =
      cursor.read_text(AsxTradeItchDetails::PARTICIPANT_ID_LENGTH);
    value.m_printable = cursor.read_char();
    value.m_occurred_at_cross = cursor.read_char();
    return value;
  }

  inline AsxTradeItchEquilibriumPriceUpdate
      AsxTradeItchEquilibriumPriceUpdate::parse(
      const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchEquilibriumPriceUpdate();
    value.m_nanoseconds = cursor.read_uint32();
    if(value.m_nanoseconds >= AsxTradeItchDetails::NANOSECONDS_PER_SECOND) {
      boost::throw_with_location(
        AsxTradeItchParserException("Nanoseconds out of range."));
    }
    value.m_order_book_id = cursor.read_uint32();
    value.m_bid_quantity = cursor.read_uint64();
    value.m_ask_quantity = cursor.read_uint64();
    value.m_equilibrium_price = cursor.read_price();
    value.m_best_bid_price = cursor.read_price();
    value.m_best_ask_price = cursor.read_price();
    value.m_best_bid_quantity = cursor.read_uint64();
    value.m_best_ask_quantity = cursor.read_uint64();
    return value;
  }

  inline AsxTradeItchEndOfSnapshot AsxTradeItchEndOfSnapshot::parse(
      const AsxTradeItchMessage& message) {
    if(message.m_type != TYPE || message.m_length < LENGTH) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid ITCH message type or length."));
    }
    auto cursor = message.get_cursor();
    auto value = AsxTradeItchEndOfSnapshot();
    static constexpr auto SEQUENCE_LENGTH = 20;
    auto sequence = cursor.read_text(SEQUENCE_LENGTH);
    auto start = sequence.find_first_not_of(' ');
    if(start == std::string::npos) {
      boost::throw_with_location(
        AsxTradeItchParserException("Missing snapshot sequence."));
    }
    auto end = sequence.data() + sequence.size();
    auto result =
      std::from_chars(sequence.data() + start, end, value.m_sequence);
    if(result.ec != std::errc() || result.ptr != end) {
      boost::throw_with_location(
        AsxTradeItchParserException("Invalid snapshot sequence."));
    }
    return value;
  }
}

#endif
