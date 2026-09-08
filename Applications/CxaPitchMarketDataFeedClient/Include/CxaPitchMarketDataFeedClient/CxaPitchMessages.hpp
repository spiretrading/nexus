#ifndef CXA_PITCH_MESSAGES_HPP
#define CXA_PITCH_MESSAGES_HPP
#include <cstddef>
#include <cstdint>
#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/throw_exception.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchCursor.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Side.hpp"

namespace Nexus {
namespace Details {
  constexpr auto SYMBOL_LENGTH = 6;
  constexpr auto PID_LENGTH = 4;
}

  /** Stores a unit clear message. */
  struct CxaPitchUnitClear {

    /** The type of a unit clear message. */
    static constexpr auto TYPE = std::uint8_t(0x97);

    /** The smallest valid length of a unit clear message. */
    static constexpr auto LENGTH = std::size_t(6);

    /**
     * Parses a CxaPitchUnitClear.
     * @param message The message to parse.
     * @return The CxaPitchUnitClear represented by the <i>message</i>.
     */
    static CxaPitchUnitClear parse(const CxaPitchMessage& message);
  };

  /** Stores a trading status message. */
  struct CxaPitchTradingStatus {

    /** The type of a trading status message. */
    static constexpr auto TYPE = std::uint8_t(0x3B);

    /** The smallest valid length of a trading status message. */
    static constexpr auto LENGTH = std::size_t(22);

    /** The time that the trading status changed. */
    boost::posix_time::ptime m_timestamp;

    /** The symbol whose trading status changed. */
    std::string m_symbol;

    /** The trading status of the symbol. */
    char m_status;

    /** The market identifier code that the symbol belongs to. */
    std::string m_market_id_code;

    /**
     * Parses a CxaPitchTradingStatus.
     * @param message The message to parse.
     * @return The CxaPitchTradingStatus represented by the <i>message</i>.
     */
    static CxaPitchTradingStatus parse(const CxaPitchMessage& message);
  };

  /** Stores an add order message. */
  struct CxaPitchAddOrder {

    /** The type of an add order message. */
    static constexpr auto TYPE = std::uint8_t(0x37);

    /** The smallest valid length of an add order message. */
    static constexpr auto LENGTH = std::size_t(42);

    /** The time that the order was accepted. */
    boost::posix_time::ptime m_timestamp;

    /** The day specific identifier assigned to the order. */
    std::uint64_t m_order_id;

    /** The side that the order is on. */
    Side m_side;

    /** The number of shares added to the book, zero if undisclosed. */
    std::uint32_t m_quantity;

    /** The symbol that the order was placed on. */
    std::string m_symbol;

    /** The display price of the order. */
    Money m_price;

    /** The participant the order is attributed to, empty if unattributed. */
    std::string m_pid;

    /**
     * Parses a CxaPitchAddOrder.
     * @param message The message to parse.
     * @return The CxaPitchAddOrder represented by the <i>message</i>.
     */
    static CxaPitchAddOrder parse(const CxaPitchMessage& message);
  };

  /** Stores an order executed message. */
  struct CxaPitchOrderExecuted {

    /** The type of an order executed message. */
    static constexpr auto TYPE = std::uint8_t(0x38);

    /** The smallest valid length of an order executed message. */
    static constexpr auto LENGTH = std::size_t(43);

    /** The time that the order was executed. */
    boost::posix_time::ptime m_timestamp;

    /** The identifier of the order that was executed. */
    std::uint64_t m_order_id;

    /** The number of shares executed. */
    std::uint32_t m_executed_quantity;

    /** The day unique identifier of this execution. */
    std::uint64_t m_execution_id;

    /** The identifier of the contra order that matched with this order. */
    std::uint64_t m_contra_order_id;

    /** The contra participant, empty if unattributed. */
    std::string m_contra_pid;

    /**
     * Parses a CxaPitchOrderExecuted.
     * @param message The message to parse.
     * @return The CxaPitchOrderExecuted represented by the <i>message</i>.
     */
    static CxaPitchOrderExecuted parse(const CxaPitchMessage& message);
  };

  /** Stores an order executed at price message. */
  struct CxaPitchOrderExecutedAtPrice {

    /** The type of an order executed at price message. */
    static constexpr auto TYPE = std::uint8_t(0x58);

    /** The smallest valid length of an order executed at price message. */
    static constexpr auto LENGTH = std::size_t(52);

    /** The time that the order was executed. */
    boost::posix_time::ptime m_timestamp;

    /** The identifier of the order that was executed. */
    std::uint64_t m_order_id;

    /** The number of shares executed. */
    std::uint32_t m_executed_quantity;

    /** The day unique identifier of this execution. */
    std::uint64_t m_execution_id;

    /** The identifier of the contra order that matched with this order. */
    std::uint64_t m_contra_order_id;

    /** The contra participant, empty if unattributed. */
    std::string m_contra_pid;

    /** The type of auction that the order was executed in. */
    char m_execution_type;

    /** The execution price of the order. */
    Money m_price;

    /**
     * Parses a CxaPitchOrderExecutedAtPrice.
     * @param message The message to parse.
     * @return The CxaPitchOrderExecutedAtPrice represented by the
     *         <i>message</i>.
     */
    static CxaPitchOrderExecutedAtPrice parse(const CxaPitchMessage& message);
  };

  /** Stores a reduce size message. */
  struct CxaPitchReduceSize {

    /** The type of a reduce size message. */
    static constexpr auto TYPE = std::uint8_t(0x39);

    /** The smallest valid length of a reduce size message. */
    static constexpr auto LENGTH = std::size_t(22);

    /** The time that the order was reduced. */
    boost::posix_time::ptime m_timestamp;

    /** The identifier of the order that was reduced. */
    std::uint64_t m_order_id;

    /** The number of shares cancelled. */
    std::uint32_t m_cancelled_quantity;

    /**
     * Parses a CxaPitchReduceSize.
     * @param message The message to parse.
     * @return The CxaPitchReduceSize represented by the <i>message</i>.
     */
    static CxaPitchReduceSize parse(const CxaPitchMessage& message);
  };

  /** Stores a modify order message. */
  struct CxaPitchModifyOrder {

    /** The type of a modify order message. */
    static constexpr auto TYPE = std::uint8_t(0x3A);

    /** The smallest valid length of a modify order message. */
    static constexpr auto LENGTH = std::size_t(31);

    /** The time that the order was modified. */
    boost::posix_time::ptime m_timestamp;

    /** The identifier of the order that was modified. */
    std::uint64_t m_order_id;

    /** The number of shares after the modification, zero if undisclosed. */
    std::uint32_t m_quantity;

    /** The price of the order after the modification. */
    Money m_price;

    /**
     * Parses a CxaPitchModifyOrder.
     * @param message The message to parse.
     * @return The CxaPitchModifyOrder represented by the <i>message</i>.
     */
    static CxaPitchModifyOrder parse(const CxaPitchMessage& message);
  };

  /** Stores a delete order message. */
  struct CxaPitchDeleteOrder {

    /** The type of a delete order message. */
    static constexpr auto TYPE = std::uint8_t(0x3C);

    /** The smallest valid length of a delete order message. */
    static constexpr auto LENGTH = std::size_t(18);

    /** The time that the order was removed from the book. */
    boost::posix_time::ptime m_timestamp;

    /** The identifier of the order that was removed from the book. */
    std::uint64_t m_order_id;

    /**
     * Parses a CxaPitchDeleteOrder.
     * @param message The message to parse.
     * @return The CxaPitchDeleteOrder represented by the <i>message</i>.
     */
    static CxaPitchDeleteOrder parse(const CxaPitchMessage& message);
  };

  /** Stores a trade message. */
  struct CxaPitchTrade {

    /** The type of a trade message. */
    static constexpr auto TYPE = std::uint8_t(0x3D);

    /** The smallest valid length of a trade message. */
    static constexpr auto LENGTH = std::size_t(72);

    /** The flag indicating a trade that resulted from a converted order. */
    static constexpr auto CONVERTED_ORDER_FLAG = std::uint8_t(0x02);

    /** The time that the trade was executed or reported. */
    boost::posix_time::ptime m_timestamp;

    /** The symbol that was traded. */
    std::string m_symbol;

    /** The number of shares executed or reported. */
    std::uint32_t m_quantity;

    /** The price of the trade. */
    Money m_price;

    /** The day unique identifier of this execution. */
    std::uint64_t m_execution_id;

    /** The identifier of the executed order. */
    std::uint64_t m_order_id;

    /** The identifier of the contra order that matched with this order. */
    std::uint64_t m_contra_order_id;

    /** The participant the trade is attributed to, empty if unattributed. */
    std::string m_pid;

    /** The contra participant, empty if unattributed. */
    std::string m_contra_pid;

    /** The type of the trade. */
    char m_trade_type;

    /** The book that the trade was executed on. */
    char m_trade_designation;

    /** The type of the off-exchange trade report. */
    char m_trade_report_type;

    /** The time of the off-exchange trade report, the epoch if unreported. */
    boost::posix_time::ptime m_transaction_time;

    /** The flags that apply to this trade. */
    std::uint8_t m_flags;

    /**
     * Parses a CxaPitchTrade.
     * @param message The message to parse.
     * @return The CxaPitchTrade represented by the <i>message</i>.
     */
    static CxaPitchTrade parse(const CxaPitchMessage& message);
  };

  /** Stores a trade break message. */
  struct CxaPitchTradeBreak {

    /** The type of a trade break message. */
    static constexpr auto TYPE = std::uint8_t(0x3E);

    /** The smallest valid length of a trade break message. */
    static constexpr auto LENGTH = std::size_t(18);

    /** The time that the execution was broken. */
    boost::posix_time::ptime m_timestamp;

    /** The day unique identifier of the execution that was broken. */
    std::uint64_t m_execution_id;

    /**
     * Parses a CxaPitchTradeBreak.
     * @param message The message to parse.
     * @return The CxaPitchTradeBreak represented by the <i>message</i>.
     */
    static CxaPitchTradeBreak parse(const CxaPitchMessage& message);
  };

  /** Stores a calculated value message. */
  struct CxaPitchCalculatedValue {

    /** The type of a calculated value message. */
    static constexpr auto TYPE = std::uint8_t(0xE3);

    /** The smallest valid length of a calculated value message. */
    static constexpr auto LENGTH = std::size_t(33);

    /** The time that the value was disseminated. */
    boost::posix_time::ptime m_timestamp;

    /** The symbol that the value applies to. */
    std::string m_symbol;

    /** The category of the value. */
    char m_category;

    /** The calculated value. */
    Money m_value;

    /** The time that the value was generated. */
    boost::posix_time::ptime m_value_timestamp;

    /**
     * Parses a CxaPitchCalculatedValue.
     * @param message The message to parse.
     * @return The CxaPitchCalculatedValue represented by the <i>message</i>.
     */
    static CxaPitchCalculatedValue parse(const CxaPitchMessage& message);
  };

  /** Stores an end of session message. */
  struct CxaPitchEndOfSession {

    /** The type of an end of session message. */
    static constexpr auto TYPE = std::uint8_t(0x2D);

    /** The smallest valid length of an end of session message. */
    static constexpr auto LENGTH = std::size_t(6);

    /**
     * Parses a CxaPitchEndOfSession.
     * @param message The message to parse.
     * @return The CxaPitchEndOfSession represented by the <i>message</i>.
     */
    static CxaPitchEndOfSession parse(const CxaPitchMessage& message);
  };

  /** Stores an auction update message. */
  struct CxaPitchAuctionUpdate {

    /** The type of an auction update message. */
    static constexpr auto TYPE = std::uint8_t(0x59);

    /** The smallest valid length of an auction update message. */
    static constexpr auto LENGTH = std::size_t(34);

    /** The time that the auction was updated. */
    boost::posix_time::ptime m_timestamp;

    /** The symbol that the auction applies to. */
    std::string m_symbol;

    /** The type of the auction. */
    char m_auction_type;

    /** The number of shares on the buy side eligible to trade. */
    std::uint32_t m_buy_shares;

    /** The number of shares on the sell side eligible to trade. */
    std::uint32_t m_sell_shares;

    /** The price at which the continuous book would match. */
    Money m_indicative_price;

    /**
     * Parses a CxaPitchAuctionUpdate.
     * @param message The message to parse.
     * @return The CxaPitchAuctionUpdate represented by the <i>message</i>.
     */
    static CxaPitchAuctionUpdate parse(const CxaPitchMessage& message);
  };

  /** Stores an auction summary message. */
  struct CxaPitchAuctionSummary {

    /** The type of an auction summary message. */
    static constexpr auto TYPE = std::uint8_t(0x5A);

    /** The smallest valid length of an auction summary message. */
    static constexpr auto LENGTH = std::size_t(30);

    /** The time that the auction completed. */
    boost::posix_time::ptime m_timestamp;

    /** The symbol that the auction applies to. */
    std::string m_symbol;

    /** The type of the auction. */
    char m_auction_type;

    /** The price that the auction executed at. */
    Money m_price;

    /** The number of shares executed during the auction. */
    std::uint32_t m_shares;

    /**
     * Parses a CxaPitchAuctionSummary.
     * @param message The message to parse.
     * @return The CxaPitchAuctionSummary represented by the <i>message</i>.
     */
    static CxaPitchAuctionSummary parse(const CxaPitchMessage& message);
  };

  inline CxaPitchUnitClear CxaPitchUnitClear::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Unit clear message too short."));
    }
    return CxaPitchUnitClear();
  }

  inline CxaPitchTradingStatus CxaPitchTradingStatus::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Trading status message too short."));
    }
    auto cursor = message.get_cursor();
    auto status = CxaPitchTradingStatus();
    status.m_timestamp = cursor.read_timestamp();
    status.m_symbol = cursor.read_text(Details::SYMBOL_LENGTH);
    status.m_status = cursor.read_char();
    status.m_market_id_code = cursor.read_text(4);
    return status;
  }

  inline CxaPitchAddOrder CxaPitchAddOrder::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Add order message too short."));
    }
    auto cursor = message.get_cursor();
    auto add_order = CxaPitchAddOrder();
    add_order.m_timestamp = cursor.read_timestamp();
    add_order.m_order_id = cursor.read_uint64();
    add_order.m_side = cursor.read_side();
    add_order.m_quantity = cursor.read_uint32();
    add_order.m_symbol = cursor.read_text(Details::SYMBOL_LENGTH);
    add_order.m_price = cursor.read_price();
    add_order.m_pid = cursor.read_text(Details::PID_LENGTH);
    return add_order;
  }

  inline CxaPitchOrderExecuted CxaPitchOrderExecuted::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Order executed message too short."));
    }
    auto cursor = message.get_cursor();
    auto executed = CxaPitchOrderExecuted();
    executed.m_timestamp = cursor.read_timestamp();
    executed.m_order_id = cursor.read_uint64();
    executed.m_executed_quantity = cursor.read_uint32();
    executed.m_execution_id = cursor.read_uint64();
    executed.m_contra_order_id = cursor.read_uint64();
    executed.m_contra_pid = cursor.read_text(Details::PID_LENGTH);
    return executed;
  }

  inline CxaPitchOrderExecutedAtPrice CxaPitchOrderExecutedAtPrice::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(CxaPitchParserException(
        "Order executed at price message too short."));
    }
    auto cursor = message.get_cursor();
    auto executed = CxaPitchOrderExecutedAtPrice();
    executed.m_timestamp = cursor.read_timestamp();
    executed.m_order_id = cursor.read_uint64();
    executed.m_executed_quantity = cursor.read_uint32();
    executed.m_execution_id = cursor.read_uint64();
    executed.m_contra_order_id = cursor.read_uint64();
    executed.m_contra_pid = cursor.read_text(Details::PID_LENGTH);
    executed.m_execution_type = cursor.read_char();
    executed.m_price = cursor.read_price();
    return executed;
  }

  inline CxaPitchReduceSize CxaPitchReduceSize::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Reduce size message too short."));
    }
    auto cursor = message.get_cursor();
    auto reduce_size = CxaPitchReduceSize();
    reduce_size.m_timestamp = cursor.read_timestamp();
    reduce_size.m_order_id = cursor.read_uint64();
    reduce_size.m_cancelled_quantity = cursor.read_uint32();
    return reduce_size;
  }

  inline CxaPitchModifyOrder CxaPitchModifyOrder::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Modify order message too short."));
    }
    auto cursor = message.get_cursor();
    auto modify_order = CxaPitchModifyOrder();
    modify_order.m_timestamp = cursor.read_timestamp();
    modify_order.m_order_id = cursor.read_uint64();
    modify_order.m_quantity = cursor.read_uint32();
    modify_order.m_price = cursor.read_price();
    return modify_order;
  }

  inline CxaPitchDeleteOrder CxaPitchDeleteOrder::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Delete order message too short."));
    }
    auto cursor = message.get_cursor();
    auto delete_order = CxaPitchDeleteOrder();
    delete_order.m_timestamp = cursor.read_timestamp();
    delete_order.m_order_id = cursor.read_uint64();
    return delete_order;
  }

  inline CxaPitchTrade CxaPitchTrade::parse(const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Trade message too short."));
    }
    auto cursor = message.get_cursor();
    auto trade = CxaPitchTrade();
    trade.m_timestamp = cursor.read_timestamp();
    trade.m_symbol = cursor.read_text(Details::SYMBOL_LENGTH);
    trade.m_quantity = cursor.read_uint32();
    trade.m_price = cursor.read_price();
    trade.m_execution_id = cursor.read_uint64();
    trade.m_order_id = cursor.read_uint64();
    trade.m_contra_order_id = cursor.read_uint64();
    trade.m_pid = cursor.read_text(Details::PID_LENGTH);
    trade.m_contra_pid = cursor.read_text(Details::PID_LENGTH);
    trade.m_trade_type = cursor.read_char();
    trade.m_trade_designation = cursor.read_char();
    trade.m_trade_report_type = cursor.read_char();
    trade.m_transaction_time = cursor.read_timestamp();
    trade.m_flags = cursor.read_uint8();
    return trade;
  }

  inline CxaPitchTradeBreak CxaPitchTradeBreak::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Trade break message too short."));
    }
    auto cursor = message.get_cursor();
    auto trade_break = CxaPitchTradeBreak();
    trade_break.m_timestamp = cursor.read_timestamp();
    trade_break.m_execution_id = cursor.read_uint64();
    return trade_break;
  }

  inline CxaPitchCalculatedValue CxaPitchCalculatedValue::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Calculated value message too short."));
    }
    auto cursor = message.get_cursor();
    auto value = CxaPitchCalculatedValue();
    value.m_timestamp = cursor.read_timestamp();
    value.m_symbol = cursor.read_text(Details::SYMBOL_LENGTH);
    value.m_category = cursor.read_char();
    value.m_value = cursor.read_price();
    value.m_value_timestamp = cursor.read_timestamp();
    return value;
  }

  inline CxaPitchEndOfSession CxaPitchEndOfSession::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("End of session message too short."));
    }
    return CxaPitchEndOfSession();
  }

  inline CxaPitchAuctionUpdate CxaPitchAuctionUpdate::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Auction update message too short."));
    }
    auto cursor = message.get_cursor();
    auto update = CxaPitchAuctionUpdate();
    update.m_timestamp = cursor.read_timestamp();
    update.m_symbol = cursor.read_text(Details::SYMBOL_LENGTH);
    update.m_auction_type = cursor.read_char();
    update.m_buy_shares = cursor.read_uint32();
    update.m_sell_shares = cursor.read_uint32();
    update.m_indicative_price = cursor.read_price();
    return update;
  }

  inline CxaPitchAuctionSummary CxaPitchAuctionSummary::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Auction summary message too short."));
    }
    auto cursor = message.get_cursor();
    auto summary = CxaPitchAuctionSummary();
    summary.m_timestamp = cursor.read_timestamp();
    summary.m_symbol = cursor.read_text(Details::SYMBOL_LENGTH);
    summary.m_auction_type = cursor.read_char();
    summary.m_price = cursor.read_price();
    summary.m_shares = cursor.read_uint32();
    return summary;
  }
}

#endif
