#include <Beam/Utilities/ToString.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchMessages.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("CxaPitchMessages") {
  static const auto TIMESTAMP = time_from_string("2021-02-10 14:45:48.641622");
  static const auto ORDER_ID = std::uint64_t(800891482924597253);
  static const auto CONTRA_ORDER_ID = std::uint64_t(800891482924597254);
  static const auto EXECUTION_ID = std::uint64_t(806921579316);
  static const auto PRICE = parse_money("12.3456789");
  static const auto TIME_TEXT = to_string(TIMESTAMP);
  static const auto PRICE_TEXT = to_string(PRICE);
  static const auto SIDE_POSITION = std::size_t(18);

  TEST_CASE("validate_message_length") {
    auto validate_length = [] (const auto& type, const auto& length) {
      auto message = std::string(length, char(0));
      message[0] = static_cast<char>(length);
      message[1] = static_cast<char>(type);
      if(type == CxaPitchAddOrder::TYPE) {
        message[SIDE_POSITION] = 'B';
      }
      REQUIRE_NOTHROW(validate(CxaPitchMessage::parse(message)));
      message.push_back(char(0));
      message[0] = static_cast<char>(message.size());
      REQUIRE_NOTHROW(validate(CxaPitchMessage::parse(message)));
      message.resize(length - 1);
      message[0] = static_cast<char>(message.size());
      REQUIRE_THROWS_AS(
        validate(CxaPitchMessage::parse(message)), CxaPitchParserException);
    };
    validate_length(CxaPitchUnitClear::TYPE, CxaPitchUnitClear::LENGTH);
    validate_length(CxaPitchTradingStatus::TYPE, CxaPitchTradingStatus::LENGTH);
    validate_length(CxaPitchAddOrder::TYPE, CxaPitchAddOrder::LENGTH);
    validate_length(CxaPitchOrderExecuted::TYPE, CxaPitchOrderExecuted::LENGTH);
    validate_length(
      CxaPitchOrderExecutedAtPrice::TYPE, CxaPitchOrderExecutedAtPrice::LENGTH);
    validate_length(CxaPitchReduceSize::TYPE, CxaPitchReduceSize::LENGTH);
    validate_length(CxaPitchModifyOrder::TYPE, CxaPitchModifyOrder::LENGTH);
    validate_length(CxaPitchDeleteOrder::TYPE, CxaPitchDeleteOrder::LENGTH);
    validate_length(CxaPitchTrade::TYPE, CxaPitchTrade::LENGTH);
    validate_length(CxaPitchTradeBreak::TYPE, CxaPitchTradeBreak::LENGTH);
    validate_length(
      CxaPitchCalculatedValue::TYPE, CxaPitchCalculatedValue::LENGTH);
    validate_length(CxaPitchEndOfSession::TYPE, CxaPitchEndOfSession::LENGTH);
    validate_length(CxaPitchAuctionUpdate::TYPE, CxaPitchAuctionUpdate::LENGTH);
    validate_length(CxaPitchAuctionSummary::TYPE,
      CxaPitchAuctionSummary::LENGTH);
    REQUIRE_NOTHROW(
      validate(CxaPitchMessage::parse(std::string_view("\x02\x7f", 2))));
  }

  TEST_CASE("parse_unit_clear") {
    auto source = std::string_view("\x06\x97\x20\x20\x20\x20", 6);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchUnitClear::TYPE);
    REQUIRE(to_string(CxaPitchUnitClear::parse(message)) == "(unit_clear)");
  }

  TEST_CASE("parse_trading_status") {
    auto source = std::string_view(
      "\x16\x3b"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "ZVZT  "
      "T"
      "AUS "
      "\x00", 22);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchTradingStatus::TYPE);
    auto status = CxaPitchTradingStatus::parse(message);
    REQUIRE(status.m_timestamp == TIMESTAMP);
    REQUIRE(status.m_symbol == "ZVZT");
    REQUIRE(status.m_status == 'T');
    REQUIRE(status.m_market_id_code == "AUS");
    REQUIRE(to_string(status) ==
      "(trading_status " + TIME_TEXT + " ZVZT T AUS)");
  }

  TEST_CASE("parse_add_order") {
    auto source = std::string_view(
      "\x2a\x37"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
      "B"
      "\xbc\x02\x00\x00"
      "ZVZT  "
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "1234"
      "\x00", 42);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchAddOrder::TYPE);
    auto add_order = CxaPitchAddOrder::parse(message);
    REQUIRE(add_order.m_timestamp == TIMESTAMP);
    REQUIRE(add_order.m_order_id == ORDER_ID);
    REQUIRE(add_order.m_side == Side::BID);
    REQUIRE(add_order.m_quantity == 700);
    REQUIRE(add_order.m_symbol == "ZVZT");
    REQUIRE(add_order.m_price == PRICE);
    REQUIRE(add_order.m_pid == "1234");
    REQUIRE(to_string(add_order) == "(add_order " + TIME_TEXT +
      " 800891482924597253 " + to_string(Side::BID) + " 700 ZVZT " +
      PRICE_TEXT + " 1234)");
  }

  TEST_CASE("parse_undisclosed_add_order") {
    auto source = std::string_view(
      "\x2a\x37"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
      "S"
      "\x00\x00\x00\x00"
      "ZVZT  "
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "    "
      "\x00", 42);
    auto add_order = CxaPitchAddOrder::parse(CxaPitchMessage::parse(source));
    REQUIRE(add_order.m_side == Side::ASK);
    REQUIRE(add_order.m_quantity == 0);
    REQUIRE(add_order.m_pid == "");
  }

  TEST_CASE("parse_sideless_add_order") {
    auto source = std::string_view(
      "\x2a\x37"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
      " "
      "\xbc\x02\x00\x00"
      "ZVZT  "
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "1234"
      "\x00", 42);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE_THROWS_AS(validate(message), CxaPitchParserException);
    REQUIRE_THROWS_AS(
      CxaPitchAddOrder::parse(message), CxaPitchParserException);
  }

  TEST_CASE("parse_grown_add_order") {
    auto source = std::string_view(
      "\x2b\x37"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
      "B"
      "\xbc\x02\x00\x00"
      "ZVZT  "
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "1234"
      "\x00\x00", 43);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_length == 43);
    auto add_order = CxaPitchAddOrder::parse(message);
    REQUIRE(add_order.m_order_id == ORDER_ID);
    REQUIRE(add_order.m_price == PRICE);
    REQUIRE(add_order.m_pid == "1234");
  }

  TEST_CASE("parse_truncated_add_order") {
    auto source = std::string_view(
      "\x29\x37"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
      "B"
      "\xbc\x02\x00\x00"
      "ZVZT  "
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "1234", 41);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE_THROWS_AS(
      CxaPitchAddOrder::parse(message), CxaPitchParserException);
  }

  TEST_CASE("parse_order_executed") {
    auto source = std::string_view(
      "\x2b\x38"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
      "\xbc\x02\x00\x00"
      "\x34\x2b\x46\xe0\xbb\x00\x00\x00"
      "\x06\x40\x5b\x77\x8f\x56\x1d\x0b"
      "5678"
      "\x00", 43);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchOrderExecuted::TYPE);
    auto executed = CxaPitchOrderExecuted::parse(message);
    REQUIRE(executed.m_timestamp == TIMESTAMP);
    REQUIRE(executed.m_order_id == ORDER_ID);
    REQUIRE(executed.m_executed_quantity == 700);
    REQUIRE(executed.m_execution_id == EXECUTION_ID);
    REQUIRE(executed.m_contra_order_id == CONTRA_ORDER_ID);
    REQUIRE(executed.m_contra_pid == "5678");
    REQUIRE(to_string(executed) == "(order_executed " + TIME_TEXT +
      " 800891482924597253 700 806921579316 800891482924597254 5678)");
  }

  TEST_CASE("parse_order_executed_at_price") {
    auto source = std::string_view(
      "\x34\x58"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
      "\xbc\x02\x00\x00"
      "\x34\x2b\x46\xe0\xbb\x00\x00\x00"
      "\x06\x40\x5b\x77\x8f\x56\x1d\x0b"
      "5678"
      "C"
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "\x00", 52);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchOrderExecutedAtPrice::TYPE);
    auto executed = CxaPitchOrderExecutedAtPrice::parse(message);
    REQUIRE(executed.m_timestamp == TIMESTAMP);
    REQUIRE(executed.m_order_id == ORDER_ID);
    REQUIRE(executed.m_executed_quantity == 700);
    REQUIRE(executed.m_execution_id == EXECUTION_ID);
    REQUIRE(executed.m_contra_order_id == CONTRA_ORDER_ID);
    REQUIRE(executed.m_contra_pid == "5678");
    REQUIRE(executed.m_execution_type == 'C');
    REQUIRE(executed.m_price == PRICE);
    REQUIRE(to_string(executed) == "(order_executed_at_price " + TIME_TEXT +
      " 800891482924597253 700 806921579316 800891482924597254 5678 C " +
      PRICE_TEXT + ")");
  }

  TEST_CASE("parse_reduce_size") {
    auto source = std::string_view(
      "\x16\x39"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
      "\xbc\x02\x00\x00", 22);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchReduceSize::TYPE);
    auto reduce_size = CxaPitchReduceSize::parse(message);
    REQUIRE(reduce_size.m_timestamp == TIMESTAMP);
    REQUIRE(reduce_size.m_order_id == ORDER_ID);
    REQUIRE(reduce_size.m_cancelled_quantity == 700);
    REQUIRE(to_string(reduce_size) ==
      "(reduce_size " + TIME_TEXT + " 800891482924597253 700)");
  }

  TEST_CASE("parse_modify_order") {
    auto source = std::string_view(
      "\x1f\x3a"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
      "\xbc\x02\x00\x00"
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "\x00", 31);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchModifyOrder::TYPE);
    auto modify_order = CxaPitchModifyOrder::parse(message);
    REQUIRE(modify_order.m_timestamp == TIMESTAMP);
    REQUIRE(modify_order.m_order_id == ORDER_ID);
    REQUIRE(modify_order.m_quantity == 700);
    REQUIRE(modify_order.m_price == PRICE);
    REQUIRE(to_string(modify_order) == "(modify_order " + TIME_TEXT +
      " 800891482924597253 700 " + PRICE_TEXT + ")");
  }

  TEST_CASE("parse_delete_order") {
    auto source = std::string_view(
      "\x12\x3c"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b", 18);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchDeleteOrder::TYPE);
    auto delete_order = CxaPitchDeleteOrder::parse(message);
    REQUIRE(delete_order.m_timestamp == TIMESTAMP);
    REQUIRE(delete_order.m_order_id == ORDER_ID);
    REQUIRE(to_string(delete_order) ==
      "(delete_order " + TIME_TEXT + " 800891482924597253)");
  }

  TEST_CASE("parse_on_exchange_trade") {
    auto source = std::string_view(
      "\x48\x3d"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "ZVZT  "
      "\xbc\x02\x00\x00"
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "\x34\x2b\x46\xe0\xbb\x00\x00\x00"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
      "\x06\x40\x5b\x77\x8f\x56\x1d\x0b"
      "1234"
      "5678"
      "N"
      "C"
      " "
      "\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00", 72);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchTrade::TYPE);
    auto trade = CxaPitchTrade::parse(message);
    REQUIRE(trade.m_timestamp == TIMESTAMP);
    REQUIRE(trade.m_symbol == "ZVZT");
    REQUIRE(trade.m_quantity == 700);
    REQUIRE(trade.m_price == PRICE);
    REQUIRE(trade.m_execution_id == EXECUTION_ID);
    REQUIRE(trade.m_order_id == ORDER_ID);
    REQUIRE(trade.m_contra_order_id == CONTRA_ORDER_ID);
    REQUIRE(trade.m_pid == "1234");
    REQUIRE(trade.m_contra_pid == "5678");
    REQUIRE(trade.m_trade_type == 'N');
    REQUIRE(trade.m_trade_designation == 'C');
    REQUIRE(trade.m_trade_report_type == ' ');
    REQUIRE(trade.m_transaction_time == from_time_t(0));
    REQUIRE(trade.m_flags == 0);
    REQUIRE(to_string(trade) == "(trade " + TIME_TEXT + " ZVZT 700 " +
      PRICE_TEXT + " 806921579316 800891482924597253 800891482924597254"
      " 1234 5678 N C   " + to_string(from_time_t(0)) + " 0)");
  }

  TEST_CASE("parse_off_exchange_trade") {
    auto source = std::string_view(
      "\x48\x3d"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "ZVZT  "
      "\xbc\x02\x00\x00"
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "\x34\x2b\x46\xe0\xbb\x00\x00\x00"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
      "\x06\x40\x5b\x77\x8f\x56\x1d\x0b"
      "1234"
      "    "
      " "
      " "
      "P"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x02", 72);
    auto trade = CxaPitchTrade::parse(CxaPitchMessage::parse(source));
    REQUIRE(trade.m_contra_pid == "");
    REQUIRE(trade.m_trade_type == ' ');
    REQUIRE(trade.m_trade_designation == ' ');
    REQUIRE(trade.m_trade_report_type == 'P');
    REQUIRE(trade.m_transaction_time == TIMESTAMP);
    REQUIRE((trade.m_flags & CxaPitchTrade::CONVERTED_ORDER_FLAG) != 0);
  }

  TEST_CASE("parse_trade_break") {
    auto source = std::string_view(
      "\x12\x3e"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x34\x2b\x46\xe0\xbb\x00\x00\x00", 18);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchTradeBreak::TYPE);
    auto trade_break = CxaPitchTradeBreak::parse(message);
    REQUIRE(trade_break.m_timestamp == TIMESTAMP);
    REQUIRE(trade_break.m_execution_id == EXECUTION_ID);
    REQUIRE(to_string(trade_break) ==
      "(trade_break " + TIME_TEXT + " 806921579316)");
  }

  TEST_CASE("parse_calculated_value") {
    auto source = std::string_view(
      "\x21\xe3"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "ZVZT  "
      "1"
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16", 33);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchCalculatedValue::TYPE);
    auto value = CxaPitchCalculatedValue::parse(message);
    REQUIRE(value.m_timestamp == TIMESTAMP);
    REQUIRE(value.m_symbol == "ZVZT");
    REQUIRE(value.m_category == '1');
    REQUIRE(value.m_value == PRICE);
    REQUIRE(value.m_value_timestamp == TIMESTAMP);
    REQUIRE(to_string(value) == "(calculated_value " + TIME_TEXT + " ZVZT 1 " +
      PRICE_TEXT + " " + TIME_TEXT + ")");
  }

  TEST_CASE("parse_end_of_session") {
    auto source = std::string_view("\x06\x2d\x00\x00\x00\x00", 6);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchEndOfSession::TYPE);
    REQUIRE(to_string(CxaPitchEndOfSession::parse(message)) ==
      "(end_of_session)");
  }

  TEST_CASE("parse_auction_update") {
    auto source = std::string_view(
      "\x22\x59"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "ZVZT  "
      "O"
      "\xbc\x02\x00\x00"
      "\x2c\x01\x00\x00"
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "\x00", 34);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchAuctionUpdate::TYPE);
    auto update = CxaPitchAuctionUpdate::parse(message);
    REQUIRE(update.m_timestamp == TIMESTAMP);
    REQUIRE(update.m_symbol == "ZVZT");
    REQUIRE(update.m_auction_type == 'O');
    REQUIRE(update.m_buy_shares == 700);
    REQUIRE(update.m_sell_shares == 300);
    REQUIRE(update.m_indicative_price == PRICE);
    REQUIRE(to_string(update) == "(auction_update " + TIME_TEXT +
      " ZVZT O 700 300 " + PRICE_TEXT + ")");
  }

  TEST_CASE("parse_auction_summary") {
    auto source = std::string_view(
      "\x1e\x5a"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "ZVZT  "
      "C"
      "\x15\xcd\x5b\x07\x00\x00\x00\x00"
      "\xbc\x02\x00\x00"
      "\x00", 30);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchAuctionSummary::TYPE);
    auto summary = CxaPitchAuctionSummary::parse(message);
    REQUIRE(summary.m_timestamp == TIMESTAMP);
    REQUIRE(summary.m_symbol == "ZVZT");
    REQUIRE(summary.m_auction_type == 'C');
    REQUIRE(summary.m_price == PRICE);
    REQUIRE(summary.m_shares == 700);
    REQUIRE(to_string(summary) == "(auction_summary " + TIME_TEXT +
      " ZVZT C " + PRICE_TEXT + " 700)");
  }

  TEST_CASE("get_timestamp") {
    auto source = std::string_view(
      "\x12\x3c"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b", 18);
    auto timestamp = get_timestamp(CxaPitchMessage::parse(source));
    REQUIRE(timestamp.has_value());
    REQUIRE(*timestamp == TIMESTAMP);
    auto clear = std::string_view("\x06\x97\x00\x00\x00\x00", 6);
    REQUIRE(!get_timestamp(CxaPitchMessage::parse(clear)));
    auto end_of_session = std::string_view("\x06\x2d\x00\x00\x00\x00", 6);
    REQUIRE(!get_timestamp(CxaPitchMessage::parse(end_of_session)));
    auto unknown = std::string_view("\x02\x7f", 2);
    REQUIRE(!get_timestamp(CxaPitchMessage::parse(unknown)));
  }

  TEST_CASE("get_block_timestamp") {
    auto source = std::string_view(
      "\x32\x00\x03\x01\x01\x00\x00\x00"
      "\x06\x97\x00\x00\x00\x00"
      "\x12\x3c"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
      "\x12\x3c"
      "\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x06\x40\x5b\x77\x8f\x56\x1d\x0b", 50);
    auto timestamp = get_timestamp(CxaPitchBlock::parse(source));
    REQUIRE(timestamp.has_value());
    REQUIRE(*timestamp == TIMESTAMP);
    auto untimestamped = std::string_view(
      "\x16\x00\x03\x01\x01\x00\x00\x00"
      "\x06\x97\x00\x00\x00\x00"
      "\x06\x2d\x00\x00\x00\x00"
      "\x02\x7f", 22);
    REQUIRE(!get_timestamp(CxaPitchBlock::parse(untimestamped)));
    auto heartbeat = std::string_view(
      "\x08\x00\x00\x01\x01\x00\x00\x00", 8);
    REQUIRE(!get_timestamp(CxaPitchBlock::parse(heartbeat)));
  }

  TEST_CASE("visit_known_message") {
    auto source = std::string_view(
      "\x12\x3c"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b", 18);
    auto text = visit(CxaPitchMessage::parse(source),
      [] (const auto& message) {
        return to_string(message);
      });
    REQUIRE(text == "(delete_order " + TIME_TEXT + " 800891482924597253)");
  }

  TEST_CASE("visit_first_matching_callable") {
    auto source = std::string_view(
      "\x12\x3c"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b", 18);
    auto text = visit(CxaPitchMessage::parse(source),
      [] (const CxaPitchAddOrder&) { return std::string("add_order"); },
      [] (const CxaPitchDeleteOrder& message) {
        return to_string(message);
      },
      [] (const CxaPitchMessage&) { return std::string("unknown"); });
    REQUIRE(text == "(delete_order " + TIME_TEXT + " 800891482924597253)");
  }

  TEST_CASE("visit_partial_callable") {
    struct Visitor {
      int operator ()(const CxaPitchUnitClear&) const {
        return 1;
      }

      int operator ()(const CxaPitchEndOfSession&) const {
        return 2;
      }
    };
    auto unit_clear = CxaPitchMessage::parse(
      std::string_view("\x06\x97\x00\x00\x00\x00", 6));
    auto end_of_session = CxaPitchMessage::parse(
      std::string_view("\x06\x2d\x00\x00\x00\x00", 6));
    auto unknown = CxaPitchMessage::parse(std::string_view("\x02\x7f", 2));
    auto fallback = [] (const auto&) { return -1; };
    SUBCASE("overloaded") {
      auto visitor = Visitor();
      REQUIRE(visit(unit_clear, visitor, fallback) == 1);
      REQUIRE(visit(end_of_session, visitor, fallback) == 2);
      REQUIRE(visit(unknown, visitor, fallback) == -1);
      REQUIRE(visit(unit_clear, fallback, visitor) == -1);
    }
    SUBCASE("constrained_generic") {
      auto visitor = []<typename T>(const T&) requires
          std::same_as<T, CxaPitchUnitClear> ||
            std::same_as<T, CxaPitchEndOfSession> {
        return T::TYPE;
      };
      auto fallback = [] (const auto&) { return std::uint8_t(0); };
      REQUIRE(visit(unit_clear, visitor, fallback) == CxaPitchUnitClear::TYPE);
      REQUIRE(visit(end_of_session, visitor, fallback) ==
        CxaPitchEndOfSession::TYPE);
      REQUIRE(visit(unknown, visitor, fallback) == 0);
      REQUIRE(visit(unit_clear, fallback, visitor) == 0);
    }
  }

  TEST_CASE("visit_rvalue_message_parameter") {
    auto source = std::string_view(
      "\x12\x3c"
      "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
      "\x05\x40\x5b\x77\x8f\x56\x1d\x0b", 18);
    auto message = CxaPitchMessage::parse(source);
    auto unknown = CxaPitchMessage::parse(std::string_view("\x02\x7f", 2));
    SUBCASE("value") {
      auto visitor = [] (CxaPitchDeleteOrder&& message) {
        return message.m_order_id;
      };
      REQUIRE(visit(message, visitor) == ORDER_ID);
      REQUIRE_THROWS_AS(visit(unknown, visitor), CxaPitchParserException);
    }
    SUBCASE("void") {
      auto order_id = std::uint64_t(0);
      auto visitor = [&] (CxaPitchDeleteOrder&& message) {
        order_id = message.m_order_id;
      };
      visit(message, visitor);
      REQUIRE(order_id == ORDER_ID);
      REQUIRE_NOTHROW(visit(unknown, visitor));
      REQUIRE(order_id == ORDER_ID);
    }
    SUBCASE("fallback") {
      auto visitor = [] (CxaPitchDeleteOrder&& message) {
        return message.m_order_id;
      };
      auto fallback = [] (const CxaPitchMessage&) {
        return std::uint64_t(0);
      };
      REQUIRE(visit(message, visitor, fallback) == ORDER_ID);
      REQUIRE(visit(unknown, visitor, fallback) == 0);
    }
  }

  TEST_CASE("visit_unmatched_message") {
    auto source = std::string_view("\x06\x7f\x00\x00\x00\x00", 6);
    auto text = visit(CxaPitchMessage::parse(source),
      [] (const CxaPitchAddOrder&) { return std::string("add_order"); },
      [] (const CxaPitchMessage& message) { return to_string(message); });
    REQUIRE(text == "(unknown 0x7F 6)");
  }

  TEST_CASE("visit_unhandled_void_message") {
    auto source = std::string_view("\x06\x7f\x00\x00\x00\x00", 6);
    auto count = 0;
    visit(CxaPitchMessage::parse(source),
      [&] (const CxaPitchAddOrder&) { ++count; });
    REQUIRE(count == 0);
  }

  TEST_CASE("visit_unhandled_value_message") {
    auto source = std::string_view("\x06\x7f\x00\x00\x00\x00", 6);
    REQUIRE_THROWS_AS(visit(CxaPitchMessage::parse(source),
      [] (const CxaPitchAddOrder&) { return std::string("add_order"); }),
      CxaPitchParserException);
  }

  TEST_CASE("visit_unknown_message") {
    auto source = std::string_view("\x06\x7f\x00\x00\x00\x00", 6);
    auto text = visit(CxaPitchMessage::parse(source),
      [] (const auto& message) {
        return to_string(message);
      });
    REQUIRE(text == "(unknown 0x7F 6)");
  }
}
