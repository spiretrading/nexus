#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpMessages.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  std::string encode_message(std::string_view header, std::string_view fields) {
    auto source = std::string("\x01\x1e");
    source.append(header);
    source += "\x1c\x1e";
    source.append(fields);
    std::ranges::replace(source, ';', StampField::SEPARATOR);
    source += StampMessage::CONTROL_TRAILER;
    return source;
  }

  std::string encode_message(std::string_view fields) {
    return encode_message("56=20260920090000123456", fields);
  }
}

TEST_SUITE("TmxIpMessages") {
  TEST_CASE("cbbo_validation") {
    auto header = std::string("17=FFFFFFFF;50=1;54=0123abcd");
    auto fields = std::string(
      "6=Quote;5=Quote;55=ABX;196=12.34567;196.1=12.35;64=100;64.1=200");
    auto source = encode_message(header, fields);
    REQUIRE_NOTHROW(validate(StampMessage::parse(source)));
    source = encode_message(header, fields.substr(fields.find(';') + 1));
    REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
      TmxIpParserException);
    for(auto identifier : {"5", "55", "196", "196.1", "64", "64.1"}) {
      auto incomplete = fields;
      auto start = incomplete.find(std::string(identifier) + "=");
      auto end = incomplete.find(';', start);
      if(end == std::string::npos) {
        incomplete.erase(start - 1);
      } else {
        incomplete.erase(start, end - start + 1);
      }
      source = encode_message(header, incomplete);
      CAPTURE(identifier);
      REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
        StampParserException);
    }
    for(auto incomplete : {"50=1;54=0123abcd", "17=FFFFFFFF;54=0123abcd",
        "17=FFFFFFFF;50=1"}) {
      source = encode_message(incomplete, fields);
      CAPTURE(incomplete);
      REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
        StampParserException);
    }
    for(auto extra : {"196.0=12", "64.0=10", "55=XYZ", "55.1=XYZ"}) {
      source = encode_message(header, fields + ";" + extra);
      CAPTURE(extra);
      REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
        StampParserException);
    }
    for(auto invalid : {"5=Trade", "55=", "55=ABCDEFGHIJKLMNOPQR",
        "196=", "196=MKT", "196=OPG", "196=MBF", "196=-1",
        "196=1000000", "196=1.0000001", "196.1=1e2", "64=",
        "64=-1", "64=1.5", "64=10000000000", "64.1=bad"}) {
      auto replacement = std::string(invalid);
      auto identifier = replacement.substr(0, replacement.find('=') + 1);
      auto invalid_fields = fields;
      auto start = invalid_fields.find(identifier);
      auto end = invalid_fields.find(';', start);
      invalid_fields.replace(start, end - start, replacement);
      source = encode_message(header, invalid_fields);
      CAPTURE(invalid);
      REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
        TmxIpParserException);
    }
    for(auto extra : {"247=", "247.1=", "247=LONG", "196.2=12", "64.2=10",
        "247.2=TSE"}) {
      source = encode_message(header, fields + ";" + extra);
      CAPTURE(extra);
      REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
        TmxIpParserException);
    }
    for(auto identifier : {"501", "502", "514", "515"}) {
      for(auto invalid : {"", "20260920090000", "2026092009000012",
          "20260920090000123456", "20260230090000123",
          "20260920240000123", "20260920090000abc"}) {
        source = encode_message(
          header + ";" + identifier + "=" + invalid, fields);
        CAPTURE(identifier);
        CAPTURE(invalid);
        REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
          TmxIpParserException);
      }
    }
  }

  TEST_CASE("cbbo_quote") {
    auto source = encode_message(
      "514=20260920093000126;17=FFFFFFFF;50=999999999;54=0123abcd;"
      "501=20260920093000123;502=20260920093000124;"
      "515=20260920093000125;513=audit;9999=extension",
      "64.1=9999999999;196.1=999999.999999;247.1=ALX;55=ABX;"
      "5=Quote;64.0=123;196=12.34567;6=Quote;247=TSE;9999=extension");
    auto message = TmxIpCbboQuote::parse(StampMessage::parse(source));
    REQUIRE(message.m_symbol == "ABX");
    REQUIRE(message.m_source_address == "0123abcd");
    REQUIRE(message.m_destination_address == "FFFFFFFF");
    REQUIRE(message.m_sequence == 999999999);
    REQUIRE(message.m_publication_timestamp.value() ==
      time_from_string("2026-09-20 09:30:00.123"));
    REQUIRE(message.m_receipt_timestamp.value() ==
      time_from_string("2026-09-20 09:30:00.124"));
    REQUIRE(message.m_inbound_timestamp.value() ==
      time_from_string("2026-09-20 09:30:00.125"));
    REQUIRE(message.m_outbound_timestamp.value() ==
      time_from_string("2026-09-20 09:30:00.126"));
    REQUIRE(message.m_sides[0].m_price == parse_money("12.34567"));
    REQUIRE(message.m_sides[0].m_quantity == 123);
    REQUIRE(message.m_sides[0].m_exchange.value() == "TSE");
    REQUIRE(message.m_sides[1].m_price == parse_money("999999.999999"));
    REQUIRE(message.m_sides[1].m_quantity == 9999999999ULL);
    REQUIRE(message.m_sides[1].m_exchange.value() == "ALX");
    REQUIRE_NOTHROW(validate(StampMessage::parse(source)));
    auto header = std::string("17=FFFFFFFF;50=0;54=0123ABCD");
    auto fields = std::string(
      "6=Quote;5=Quote;55=ABX;196=0;196.1=0;64=0;64.1=0");
    source = encode_message(header, fields);
    message = TmxIpCbboQuote::parse(StampMessage::parse(source));
    REQUIRE(message.m_sequence == 0);
    REQUIRE(!message.m_publication_timestamp);
    REQUIRE(!message.m_receipt_timestamp);
    REQUIRE(!message.m_inbound_timestamp);
    REQUIRE(!message.m_outbound_timestamp);
    for(auto& side : message.m_sides) {
      REQUIRE(side.m_price == Money::ZERO);
      REQUIRE(side.m_quantity == 0);
      REQUIRE(!side.m_exchange);
    }
    for(auto index = 0; index != 2; ++index) {
      source = encode_message(
        header, fields + ";247." + std::to_string(index) + "=TSE");
      message = TmxIpCbboQuote::parse(StampMessage::parse(source));
      REQUIRE(message.m_sides[index].m_exchange.value() == "TSE");
      REQUIRE(!message.m_sides[1 - index].m_exchange);
    }
    for(auto identifier : {"501", "502", "515", "514"}) {
      source = encode_message(
        header + ";" + identifier + "=20240229093000123", fields);
      message = TmxIpCbboQuote::parse(StampMessage::parse(source));
      auto timestamps = {message.m_publication_timestamp,
        message.m_receipt_timestamp, message.m_inbound_timestamp,
        message.m_outbound_timestamp};
      REQUIRE(std::ranges::count_if(timestamps,
        [] (const auto& value) { return bool(value); }) == 1);
      for(auto& timestamp : timestamps) {
        if(timestamp) {
          REQUIRE(*timestamp == time_from_string("2024-02-29 09:30:00.123"));
        }
      }
    }
    source = encode_message(header,
      "6=TradeReport;5=Quote;55=ABX;196=0;196.1=0;64=0;64.1=0");
    REQUIRE_THROWS_AS(TmxIpCbboQuote::parse(StampMessage::parse(source)),
      TmxIpParserException);
  }

  TEST_CASE("cbbo_live_quote") {
    auto source = encode_message(
      "514=20260921190457364;515=20260921150457364;"
      "502=20260921150457364;501=20260921150457364;"
      "54=000000;17=000000;50=78823845",
      "55=HBA;196=29.9200;64=1100;247=TSE;196.1=30.0500;"
      "64.1=1900;247.1=CHI;6=Quote;5=Quote");
    auto message = TmxIpCbboQuote::parse(StampMessage::parse(source));
    REQUIRE(message.m_source_address == "000000");
    REQUIRE(message.m_destination_address == "000000");
    REQUIRE(message.m_symbol == "HBA");
    REQUIRE(message.m_sequence == 78823845);
    REQUIRE(message.m_sides[0].m_price == parse_money("29.92"));
    REQUIRE(message.m_sides[0].m_quantity == 1100);
    REQUIRE(message.m_sides[0].m_exchange.value() == "TSE");
    REQUIRE(message.m_sides[1].m_price == parse_money("30.05"));
    REQUIRE(message.m_sides[1].m_quantity == 1900);
    REQUIRE(message.m_sides[1].m_exchange.value() == "CHI");
    REQUIRE_NOTHROW(validate(StampMessage::parse(source)));
  }

  TEST_CASE("cbbo_control_fields") {
    auto fields = std::string(
      "6=Quote;5=Quote;55=ABX;196=1;196.1=2;64=100;64.1=200");
    for(auto sequence : {"", "-1", "+1", "1000000000", "1.5"}) {
      auto source = encode_message(
        std::string("17=FFFFFFFF;54=0123abcd;50=") + sequence, fields);
      CAPTURE(sequence);
      REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
        TmxIpParserException);
    }
    for(auto address : {"", "123abcd", "00123abcd", "0123abcg", "000000",
        "00000000", "FFFFFFFF", "0123abcd", "00000001"}) {
      for(auto name : {"17", "54"}) {
        auto control = std::string("17=FFFFFFFF;50=1;54=0123abcd");
        auto start = control.find(std::string(name) + '=') + 3;
        control.replace(start, 8, address);
        auto source = encode_message(control, fields);
        CAPTURE(address);
        CAPTURE(name);
        auto message = TmxIpCbboQuote::parse(StampMessage::parse(source));
        if(std::string_view(name) == "54") {
          REQUIRE(message.m_source_address == address);
        } else {
          REQUIRE(message.m_destination_address == address);
        }
      }
    }
    for(auto extra : {"17=FFFFFFFF", "17.1=FFFFFFFF", "50.0=2", "50.1=2",
        "54=0123abcd", "54.1=0123abcd", "501.1=20260920093000123",
        "502.1=20260920093000123", "514.1=20260920093000123",
        "515.1=20260920093000123", "501=20260920093000123;501.0=bad"}) {
      auto source = encode_message(
        std::string("17=FFFFFFFF;50=1;54=0123abcd;") + extra, fields);
      CAPTURE(extra);
      REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
        StampParserException);
    }
  }

  TEST_CASE("cbbo_visit") {
    auto source = encode_message("17=FFFFFFFF;50=1;54=0123abcd",
      "6=Quote;5=Quote;55=ABX;196=1;196.1=2;64=100;64.1=200");
    auto message = StampMessage::parse(source);
    auto result = visit(message,
      [] (const TmxIpTradeReport&) { return 1; },
      [] (const TmxIpCbboQuote& quote) {
        REQUIRE(quote.m_symbol == "ABX");
        return 2;
      }, [] (const StampMessage&) { return 3; });
    REQUIRE(result == 2);
    REQUIRE(visit(message, [] (const auto&) { return 4; }) == 4);
    REQUIRE(visit(message, [] (const StampMessage&) { return 5; },
      [] (const TmxIpCbboQuote&) { return 6; }) == 5);
    auto value = 7;
    auto& reference = visit(message,
      [&] (const TmxIpCbboQuote&) -> int& { return value; });
    REQUIRE(&reference == &value);
    auto called = false;
    visit(message, [&] (const TmxIpCbboQuote&) { called = true; });
    REQUIRE(called);
    called = false;
    source = encode_message("6=FutureMessage");
    message = StampMessage::parse(source);
    visit(message, [&] (const TmxIpCbboQuote&) { called = true; });
    REQUIRE(!called);
    REQUIRE_THROWS_AS(visit(message,
      [] (const TmxIpCbboQuote&) { return 1; }), TmxIpParserException);
    source = encode_message("17=FFFFFFFF;50=1;54=0123abcd", "6=Quote");
    message = StampMessage::parse(source);
    REQUIRE_THROWS_AS(visit(message,
      [] (const TmxIpCbboQuote&) {}), StampParserException);
    REQUIRE(visit(message, [] (const StampMessage&) { return 8; }) == 8);
  }

  TEST_CASE("price") {
    for(auto source : {"0", "12.34567", "999999.999999", "000012.30",
        "0.000001", "12.345678"}) {
      auto price = TmxIpPrice::parse(source);
      REQUIRE(price.m_type == TmxIpPrice::Type::LIMIT);
      REQUIRE(price.m_value == parse_money(source));
    }
    for(auto source : {"0.00000", "9.51000", "12.34567", "999999.999999"}) {
      for(auto padding : {"0", "0000000000"}) {
        auto price = TmxIpPrice::parse(std::string(source) + padding);
        REQUIRE(price.m_type == TmxIpPrice::Type::LIMIT);
        REQUIRE(price.m_value == parse_money(source));
      }
    }
    REQUIRE(TmxIpPrice::parse("").m_type == TmxIpPrice::Type::MARKET);
    REQUIRE(TmxIpPrice::parse("MKT").m_type == TmxIpPrice::Type::MARKET);
    REQUIRE(TmxIpPrice::parse("OPG").m_type == TmxIpPrice::Type::OPENING);
    REQUIRE(TmxIpPrice::parse("MBF").m_type ==
      TmxIpPrice::Type::MUST_BE_FILLED);
    for(auto source : {"-1", "+1", ".5", "1.", "1000000",
        "1.00000010", "1.0000001", "1.00000x0", "1.00000.0",
        "1e2", "NaN", " 1", "1 ", "MOC", "1.2.3"}) {
      REQUIRE_THROWS_AS(TmxIpPrice::parse(source), TmxIpParserException);
    }
  }

  TEST_CASE("volume") {
    auto fields = std::string();
    SUBCASE("order_cancel_report") {
      fields = "6=OrderCancelResp;5=Buy;55=ABX;16=Booked;196=12.30";
    }
    SUBCASE("order_book") {
      fields = "6=OrderInfo;5=OrderBook;55=ABX;40=ONE;70=1;197=Buy";
    }
    SUBCASE("trade_report") {
      fields = "6=TradeReport;5=Trade;55=ABX;41=12.30";
    }
    fields += ";57=20260920090000123";
    for(auto value : {"0", "123", "00000000000000000123", "10000000000",
        "20000000000000000000", "99999999999999999999"}) {
      CAPTURE(std::string(value));
      auto expected = Quantity(std::stod(value));
      auto source = encode_message(fields + ";64=" + value + ";68=" + value +
        ";31=" + value + ";74=" + value + ";150=" + value);
      visit(StampMessage::parse(source),
        [&] (const TmxIpOrderCancelReport& message) {
          REQUIRE(message.m_quantity == expected);
          REQUIRE(message.m_priority_quantity.value() == expected);
          REQUIRE(message.m_minimum_fill_quantity.value() == expected);
          REQUIRE(message.m_lots_of.value() == expected);
        }, [&] (const TmxIpOrderBook& message) {
          REQUIRE(message.m_orders.size() == 1);
          REQUIRE(message.m_orders.front().m_quantity == expected);
        }, [&] (const TmxIpTradeReport& message) {
          REQUIRE(message.m_quantity == expected);
          REQUIRE(message.m_sides[0].m_display_quantity.value() == expected);
        });
    }
    for(auto value : {"", "-1", "+1", "1.5", "1e2", "NaN", " 1", "1 ",
        "100000000000000000000"}) {
      auto source = encode_message(fields + ";64=" + value);
      CAPTURE(std::string(value));
      REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
        TmxIpParserException);
    }
  }

  TEST_CASE("settlement_terms") {
    auto fields = std::string();
    SUBCASE("order_cancel_report") {
      fields = "55=FCCB;636=AQL;6=OrderCancelResp;5=Sell;247=AQL;"
        "40=1942YMGVQ2H7;64=100;196=21.8400;70=80;16=Cancelled";
    }
    SUBCASE("order_book") {
      fields = "6=OrderInfo;5=OrderBook;55=FCCB;40=1942YMGVQ2H7;"
        "70=80;197=Sell;64=100;196=21.8400";
    }
    SUBCASE("stock_status") {
      fields = "6=StockStatus;55=FCCB";
    }
    SUBCASE("trade_report") {
      fields = "6=TradeReport;5=Trade;55=FCCB;41=21.8400;64=100";
    }
    fields += ";57=20260929170003215611000";
    for(auto terms : {"Cash", "CT", "MS", "NN", "Future", "ND", "20261002"}) {
      CAPTURE(std::string(terms));
      auto source = encode_message(fields + ";53=" + terms);
      auto message = StampMessage::parse(source);
      visit(message, [&] (const TmxIpOrderCancelReport& value) {
        REQUIRE(value.m_settlement_terms.value() == terms);
      }, [&] (const TmxIpOrderBook& value) {
        REQUIRE(value.m_orders.size() == 1);
        REQUIRE(value.m_orders.front().m_settlement_terms.value() == terms);
      }, [&] (const TmxIpStockStatus& value) {
        REQUIRE(value.m_settlement_terms.value() == terms);
      }, [&] (const TmxIpTradeReport& value) {
        REQUIRE(value.m_settlement_terms.value() == terms);
      });
      REQUIRE_NOTHROW(validate(message));
    }
  }

  TEST_CASE("order_cancel_report") {
    auto source = encode_message("55=ABX;64=9999999999;196=12.34567;"
      "6=OrderCancelResp;5=Buy;16=Booked;57=20260920085959123456789;"
      "40=ABC000000000000001;70=079;247=TSE;178=20260920085959123;"
      "11=OLD;68=100;31=25;74=5;503=Y;168=N;53=Cash;"
      "639=New;642=12.30;9999=ignored");
    auto message = TmxIpOrderCancelReport::parse(StampMessage::parse(source));
    REQUIRE(message.m_symbol == "ABX");
    REQUIRE(message.m_action == "Buy");
    REQUIRE(message.m_confirmation == "Booked");
    REQUIRE(message.m_order_id.value() == "ABC000000000000001");
    REQUIRE(message.m_previous_order_id.value() == "OLD");
    REQUIRE(message.m_broker.value() == 79);
    REQUIRE(message.m_public_price.m_type == TmxIpPrice::Type::LIMIT);
    REQUIRE(message.m_public_price.m_value == parse_money("12.34567"));
    REQUIRE(message.m_quantity == std::uint64_t(9999999999));
    REQUIRE(message.m_header.m_exchange.value() == "TSE");
    REQUIRE(message.m_header.m_timestamp ==
      time_from_string("2026-09-20 09:00:00.123456"));
    REQUIRE(message.m_header.m_trading_timestamp.value() ==
      time_from_string("2026-09-20 08:59:59.123456789"));
    REQUIRE(message.m_priority_timestamp.value() ==
      time_from_string("2026-09-20 08:59:59.123"));
    REQUIRE(message.m_priority_quantity.value() == 100);
    REQUIRE(message.m_minimum_fill_quantity.value() == 25);
    REQUIRE(message.m_lots_of.value() == 5);
    REQUIRE(message.m_is_bypass.value());
    REQUIRE(!message.m_is_nonresident.value());
    REQUIRE(message.m_settlement_terms.value() == "Cash");
    REQUIRE(message.m_previous_price->m_value == parse_money("12.30"));
    REQUIRE_NOTHROW(validate(StampMessage::parse(source)));
  }
  TEST_CASE("order_book") {
    auto source = encode_message("6=OrderInfo;5=OrderBook;"
      "57=2026092007000000;247=TSE;55=ABX;40=ONE;70=1;"
      "197=Buy;64=500;196=10.50;55.1=XYZ;40.1=TWO;70.1=2;"
      "197.1=Sell;64.1=1000;41.1=MKT;113=Y;111=2;112=2");
    auto message = TmxIpOrderBook::parse(StampMessage::parse(source));
    REQUIRE(message.m_orders.size() == 2);
    auto& first = message.m_orders[0];
    auto& second = message.m_orders[1];
    REQUIRE(first.m_symbol == "ABX");
    REQUIRE(first.m_order_id == "ONE");
    REQUIRE(first.m_broker == 1);
    REQUIRE(first.m_side == Side::BID);
    REQUIRE(first.m_quantity == 500);
    REQUIRE(first.m_public_price->m_value == parse_money("10.50"));
    REQUIRE(!first.m_price);
    REQUIRE(second.m_symbol == "XYZ");
    REQUIRE(second.m_order_id == "TWO");
    REQUIRE(second.m_broker == 2);
    REQUIRE(second.m_side == Side::ASK);
    REQUIRE(second.m_quantity == 1000);
    REQUIRE(!second.m_public_price);
    REQUIRE(second.m_price->m_type == TmxIpPrice::Type::MARKET);
    REQUIRE(message.m_is_last_message.value());
    REQUIRE(message.m_number_of_messages.value() == 2);
    REQUIRE(message.m_total_messages.value() == 2);
    REQUIRE_NOTHROW(validate(StampMessage::parse(source)));
    source = encode_message("6=ClearOrderInfo;5=ClearOrderBook;55=ABX;"
      "57=2026092007000000;247=AQL;636=AQL");
    auto clear = TmxIpClearOrderBook::parse(StampMessage::parse(source));
    REQUIRE(clear.m_symbol == "ABX");
    REQUIRE(clear.m_header.m_exchange.value() == "AQL");
    REQUIRE(clear.m_header.m_book_type.value() == "AQL");
  }

  TEST_CASE("trade_report") {
    for(auto action : {"Trade", "Cancelled", "AuctionTradeIndividual"}) {
      auto source = encode_message(std::string("6=TradeReport;5=") + action +
        ";55=ABX;41=23.125;64=9999999999;57=20260920093000123;247=TSE;"
        "40.1=SELL-ORDER;70.1=2;150.1=0;40.0=BUY-ORDER;70=1;150=200;"
        "220=000000000000001234;506=PREVIOUS|B;183=Y;76=N;503=Y;"
        "617=N;684=N;688=Y;494=N;574=R;264.1=2026092009295900;"
        "178=20260920092959000;114=23.10;390=Regular;53=ND;"
        "639=ignored;689=L;703=P");
      auto message = TmxIpTradeReport::parse(StampMessage::parse(source));
      REQUIRE(message.m_action == action);
      REQUIRE(message.m_price.m_value == parse_money("23.125"));
      REQUIRE(message.m_quantity == std::uint64_t(9999999999));
      REQUIRE(message.m_trade_id.value() == "000000000000001234");
      REQUIRE(message.m_original_trade_id.value() == "PREVIOUS|B");
      REQUIRE(message.m_is_correction.value());
      REQUIRE(!message.m_is_extended_hours.value());
      REQUIRE(message.m_is_bypass.value());
      REQUIRE(!message.m_is_dark.value());
      REQUIRE(!message.m_is_mid_only.value());
      REQUIRE(message.m_is_conditional.value());
      REQUIRE(!message.m_is_market_on_close.value());
      REQUIRE(message.m_opening_auction.value() == "R");
      REQUIRE(message.m_last_sale.value() == parse_money("23.10"));
      REQUIRE(message.m_sides[0].m_order_id.value() == "BUY-ORDER");
      REQUIRE(message.m_sides[0].m_broker.value() == 1);
      REQUIRE(message.m_sides[0].m_display_quantity.value() == 200);
      REQUIRE(message.m_sides[1].m_order_id.value() == "SELL-ORDER");
      REQUIRE(message.m_sides[1].m_broker.value() == 2);
      REQUIRE(message.m_sides[1].m_display_quantity.value() == 0);
      REQUIRE(!message.m_sides[0].m_trade_timestamp);
      REQUIRE(message.m_sides[1].m_trade_timestamp.value() ==
        time_from_string("2026-09-20 09:29:59"));
      REQUIRE_NOTHROW(validate(StampMessage::parse(source)));
    }
    auto source = encode_message("6=TradeReport;5=Trade;55=ABX;41=12;64=1;"
      "57=20260920093000123;40.1=PASSIVE");
    auto message = TmxIpTradeReport::parse(StampMessage::parse(source));
    REQUIRE(!message.m_sides[0].m_order_id);
    REQUIRE(message.m_sides[1].m_order_id.value() == "PASSIVE");
    REQUIRE(!message.m_trade_id);
    REQUIRE(!message.m_header.m_exchange);
  }

  TEST_CASE("cls_trade_report") {
    auto source = encode_message(
      "56=20260921150000000;17=FFFFFFFF;50=1;54=0123abcd;"
      "501=20260921150000100;502=20260921150000050",
      "6=TradeReport;5=Trade;55=ABX;41=12.34567;64=999999999;"
      "57=20260921100000123;247=AQL;636=AQD;53=20260924;"
      "264=20260921093000123;70=001;70.1=099;183=Y");
    auto message = TmxIpTradeReport::parse(StampMessage::parse(source));
    REQUIRE(message.m_header.m_exchange.value() == "AQL");
    REQUIRE(message.m_header.m_book_type.value() == "AQD");
    REQUIRE(message.m_settlement_terms.value() == "20260924");
    REQUIRE(message.m_quantity == 999999999);
    REQUIRE(message.m_price.m_value == parse_money("12.34567"));
    REQUIRE(message.m_header.m_trading_timestamp.value() ==
      time_from_string("2026-09-21 10:00:00.123"));
    REQUIRE(message.m_sides[0].m_trade_timestamp.value() ==
      time_from_string("2026-09-21 09:30:00.123"));
    REQUIRE(message.m_sides[0].m_broker.value() == 1);
    REQUIRE(message.m_sides[1].m_broker.value() == 99);
    REQUIRE_NOTHROW(validate(StampMessage::parse(source)));
  }

  TEST_CASE("cls_live_trade_report") {
    auto source = encode_message(
      "514=20260921002200000;515=20260921150622022;"
      "56=20260921150622021938;513=TRI:BC[6]:BA[8]:SEQ[104179];"
      "502=20260921150622022;501=20260921150622022;"
      "54=000000;17=000000;50=1055594",
      "55=ADBE;64=40;5=Trade;247=TCM;57=20260921150622021938;"
      "688=N;220=010004EMC;70=80;70.1=80;41=9.510000;6=TradeReport");
    auto message = TmxIpTradeReport::parse(StampMessage::parse(source));
    REQUIRE(message.m_symbol == "ADBE");
    REQUIRE(message.m_header.m_exchange.value() == "TCM");
    REQUIRE(message.m_price.m_type == TmxIpPrice::Type::LIMIT);
    REQUIRE(message.m_price.m_value == parse_money("9.51"));
    REQUIRE(message.m_quantity == 40);
    REQUIRE(message.m_sides[0].m_broker.value() == 80);
    REQUIRE(message.m_sides[1].m_broker.value() == 80);
    REQUIRE_NOTHROW(validate(StampMessage::parse(source)));
  }

  TEST_CASE("calculated_opening_price") {
    for(auto action : {"AssignCOP", "AssignLimit"}) {
      auto source = encode_message(std::string("6=MBXMessage;5=") + action +
        ";55=ABX;191=30.125;57=20260920090000123;247=TSE;194=1;195=2;"
        "192=001|ORDER1;41=OPG;192.1=002|ORDER2;"
        "159=Pre-open;492=BuySide;493=200;654=1000;698=800");
      auto message = TmxIpMbxMessage::parse(StampMessage::parse(source));
      REQUIRE(message.m_action == action);
      REQUIRE(message.m_symbol == "ABX");
      REQUIRE(message.m_calculated_opening_price.m_type ==
        TmxIpPrice::Type::LIMIT);
      REQUIRE(message.m_calculated_opening_price.m_value ==
        parse_money("30.125"));
      REQUIRE(message.m_header.m_exchange.value() == "TSE");
      REQUIRE(message.m_part_number.value() == 1);
      REQUIRE(message.m_total_parts.value() == 2);
      REQUIRE(message.m_orders.size() == 2);
      REQUIRE(message.m_orders[0].m_key.value() == "001|ORDER1");
      REQUIRE(message.m_orders[0].m_price->m_type == TmxIpPrice::Type::OPENING);
      REQUIRE(message.m_orders[1].m_key.value() == "002|ORDER2");
      REQUIRE(!message.m_orders[1].m_price);
      REQUIRE(message.m_market_state.value() == "Pre-open");
      REQUIRE(message.m_imbalance_side.value() == Side::BID);
      REQUIRE(message.m_imbalance_quantity.value() == 200);
      REQUIRE(message.m_theoretical_opening_quantity.value() == 1000);
      REQUIRE(message.m_paired_quantity.value() == 800);
      REQUIRE_NOTHROW(validate(StampMessage::parse(source)));
    }
    auto source = encode_message("6=MBXMessage;5=AssignCOP;55=ABX;191=0;"
      "57=20260920090000123");
    auto message = TmxIpMbxMessage::parse(StampMessage::parse(source));
    REQUIRE(message.m_orders.empty());
    REQUIRE(!message.m_part_number);
    REQUIRE(message.m_calculated_opening_price.m_type ==
      TmxIpPrice::Type::LIMIT);
    REQUIRE(message.m_calculated_opening_price.m_value == Money::ZERO);
  }

  TEST_CASE("moc_imbalance") {
    for(auto [text, side] : {std::pair("BuySide", Side::BID),
        {"SellSide", Side::ASK}, {"NA", Side::NONE},
        {"InsufficientOrders", Side::NONE}}) {
      auto source = encode_message(std::string("6=MocImbalanceStatus;") +
        "55=ABX;57=20260920155000123;247=TSE;636=AQL;631=30.125;492=" +
        text + ";493=999999999;698=800;691=100;692=BuySide;693=30.25");
      auto stamp = StampMessage::parse(source);
      auto message = TmxIpMocImbalance::parse(stamp);
      REQUIRE(message.m_symbol == "ABX");
      REQUIRE(message.m_header.m_trading_timestamp.value() ==
        time_from_string("2026-09-20 15:50:00.123"));
      REQUIRE(message.m_header.m_exchange.value() == "TSE");
      REQUIRE(message.m_header.m_book_type.value() == "AQL");
      REQUIRE(message.m_side == side);
      REQUIRE(message.m_quantity.value() == 999999999);
      REQUIRE(message.m_reference_price.m_type == TmxIpPrice::Type::LIMIT);
      REQUIRE(message.m_reference_price.m_value == parse_money("30.125"));
      REQUIRE(visit(stamp, [] (const TmxIpMocImbalance& message) {
        return message.m_side;
      }) == side);
      REQUIRE_NOTHROW(validate(stamp));
    }
    auto source = encode_message("6=MocImbalanceStatus;55=ABX;"
      "57=20260920155000123;631=0;492=NA");
    auto message = TmxIpMocImbalance::parse(StampMessage::parse(source));
    REQUIRE(!message.m_quantity);
    REQUIRE(message.m_reference_price.m_value == Money::ZERO);
    for(auto fields : {
        "6=MocImbalanceStatus;57=20260920155000123;631=30;492=BuySide",
        "6=MocImbalanceStatus;55=ABX;631=30;492=BuySide",
        "6=MocImbalanceStatus;55=ABX;57=20260920155000123;492=BuySide",
        "6=MocImbalanceStatus;55=ABX;57=20260920155000123;631=30",
        "6=MocImbalanceStatus;55=ABX;57=20260920155000123;631=30;492=BAD",
        "6=MocImbalanceStatus;55=ABX;57=20260920155000123;631=BAD;492=NA",
        "6=MocImbalanceStatus;55=ABX;57=20260920155000123;631=30;"
          "492=BuySide;493=-1"}) {
      source = encode_message(fields);
      REQUIRE_THROWS_AS(TmxIpMocImbalance::parse(StampMessage::parse(source)),
        std::exception);
      REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
        std::exception);
    }
  }

  TEST_CASE("opening_auction") {
    auto source = encode_message("6=OpeningAuction;5=PairedVolume;55=ABX;"
      "191=25.50;578=999999999;57=20260920092959123456;247=TSE");
    auto paired = TmxIpOpeningAuction::parse(StampMessage::parse(source));
    REQUIRE(paired.m_action == "PairedVolume");
    REQUIRE(paired.m_symbol.value() == "ABX");
    REQUIRE(paired.m_calculated_opening_price->m_value == parse_money("25.50"));
    REQUIRE(paired.m_paired_quantity.value() == 999999999);
    REQUIRE(!paired.m_imbalance_side);
    for(auto side : {"BuySide", "SellSide", "NA"}) {
      source = encode_message(std::string(
        "6=OpeningAuction;5=OddlotImbalance;573=0;572=") + side);
      auto imbalance = TmxIpOpeningAuction::parse(StampMessage::parse(source));
      REQUIRE(imbalance.m_action == "OddlotImbalance");
      REQUIRE(imbalance.m_imbalance_quantity.value() == 0);
      REQUIRE(!imbalance.m_calculated_opening_price);
      REQUIRE(!imbalance.m_symbol);
      REQUIRE(!imbalance.m_header.m_trading_timestamp);
      if(std::string_view(side) == "BuySide") {
        REQUIRE(imbalance.m_imbalance_side.value() == Side::BID);
      } else if(std::string_view(side) == "SellSide") {
        REQUIRE(imbalance.m_imbalance_side.value() == Side::ASK);
      } else {
        REQUIRE(imbalance.m_imbalance_side.value() == Side::NONE);
      }
    }
    source = encode_message("6=OpeningAuction;5=PairedVolume");
    auto empty = TmxIpOpeningAuction::parse(StampMessage::parse(source));
    REQUIRE(!empty.m_calculated_opening_price);
    REQUIRE(!empty.m_paired_quantity);
    REQUIRE_NOTHROW(validate(StampMessage::parse(source)));
  }

  TEST_CASE("symbol_name") {
    for(auto name : {
        "LONGPOINT ETF CORP MEGASHORT -3X CDN GOLD MINERS DAILY LEVERAGED "
          "ALTERNATIVE ETF SHS NEW",
        "LONGPOINT ETF CORP MEGASHORT -3X US SEMICONDUCTORS DAILY LEVERAGED "
          "ALTERNATIVE ETF SHS NEW",
        "TRANSALTA CORP MTN CUMULATIVE REDEEMABLE FLOATING RATE FIRST "
          "PREFERRED SHARES, SERIES D"}) {
      CAPTURE(name);
      auto source = encode_message(
        std::string("6=SymbolInfo;5=SymbolStatus;55=ABX;") +
          "57=20260923041202414702000;177=" + name);
      auto message = TmxIpSymbolStatus::parse(StampMessage::parse(source));
      REQUIRE(message.m_name.has_value());
      REQUIRE(*message.m_name == name);
    }
    auto source = encode_message("6=SymbolInfo;5=SymbolStatus;55=ABX;"
      "57=20260923041202414702000;177=");
    REQUIRE_THROWS_AS(TmxIpSymbolStatus::parse(StampMessage::parse(source)),
      TmxIpParserException);
    auto name = std::string(256, 'A');
    source = encode_message(std::string("6=SymbolInfo;5=SymbolStatus;55=ABX;") +
      "57=20260923041202414702000;177=" + name);
    auto message = TmxIpSymbolStatus::parse(StampMessage::parse(source));
    REQUIRE(message.m_name.has_value());
    REQUIRE(*message.m_name == name);
    source = encode_message(std::string("6=SymbolInfo;5=SymbolStatus;55=ABX;") +
      "57=20260923041202414702000;177=" + name + 'A');
    REQUIRE_THROWS_AS(TmxIpSymbolStatus::parse(StampMessage::parse(source)),
      TmxIpParserException);
  }

  TEST_CASE("trading_status") {
    auto source = std::string();
    SUBCASE("symbol") {
      source = encode_message("6=SymbolInfo;5=SymbolStatus;55=ABX;"
        "57=20260920070000123;247=CHI;554=T;58=CAD;115=100;282=1;"
        "161=PreOpen;177=Barrick;105=Equity;665=N;113=Y;111=1;112=1");
      auto message = TmxIpSymbolStatus::parse(StampMessage::parse(source));
      REQUIRE(message.m_symbol == "ABX");
      REQUIRE(message.m_header.m_exchange.value() == "CHI");
      REQUIRE(message.m_listing_market.value() == "T");
      REQUIRE(message.m_board_lot.value() == 100);
      REQUIRE(message.m_stock_state.value() == "PreOpen");
      REQUIRE(message.m_name.value() == "Barrick");
      REQUIRE(!message.m_is_test_symbol.value());
      REQUIRE(message.m_is_last_message.value());
    }
    SUBCASE("stock") {
      source = encode_message("6=StockStatus;55=ABX;57=20260920090000123;"
        "247=TSE;161=AuthorizedDelayed;361=Opening Delayed;282=1;554=T;"
        "58=CAD;173=Waiting;491=25.5;496=Y;110=N;605=Y;168=N");
      auto message = TmxIpStockStatus::parse(StampMessage::parse(source));
      REQUIRE(message.m_symbol.value() == "ABX");
      REQUIRE(message.m_header.m_exchange.value() == "TSE");
      REQUIRE(message.m_stock_state.value() == "AuthorizedDelayed");
      REQUIRE(message.m_sub_stock_state.value() == "Opening Delayed");
      REQUIRE(message.m_comment.value() == "Waiting");
      REQUIRE(message.m_calculated_closing_price->m_value ==
        parse_money("25.5"));
      REQUIRE(message.m_is_moc_eligible.value());
      REQUIRE(!message.m_accepts_anonymous.value());
      REQUIRE(message.m_accepts_undisplayed.value());
    }
    SUBCASE("market") {
      source = encode_message("6=MarketStateChange;57=20260920093000123;"
        "247=TSE;159=Open;282=1");
      auto message = TmxIpMarketStateChange::parse(StampMessage::parse(source));
      REQUIRE(message.m_market_state.value() == "Open");
      REQUIRE(message.m_stock_group.value() == 1);
      REQUIRE(message.m_header.m_exchange.value() == "TSE");
    }
    SUBCASE("optional_stock_fields") {
      source = encode_message("6=StockStatus;57=20260920090000123");
      auto message = TmxIpStockStatus::parse(StampMessage::parse(source));
      REQUIRE(!message.m_symbol);
      REQUIRE(!message.m_stock_state);
      REQUIRE(!message.m_listing_market);
      REQUIRE(!message.m_header.m_exchange);
    }
    SUBCASE("optional_market_fields") {
      source = encode_message("6=MarketStateChange;57=20260920090000123");
      auto message = TmxIpMarketStateChange::parse(StampMessage::parse(source));
      REQUIRE(!message.m_market_state);
      REQUIRE(!message.m_stock_group);
    }
    REQUIRE_NOTHROW(validate(StampMessage::parse(source)));
  }

  TEST_CASE("timestamp") {
    for(auto fraction : {"12", "123", "123456", "123456789"}) {
      auto source = encode_message(std::string(
        "6=StockStatus;57=20240229085959") + fraction);
      auto message = TmxIpStockStatus::parse(StampMessage::parse(source));
      REQUIRE(message.m_header.m_trading_timestamp.value() ==
        time_from_string(std::string("2024-02-29 08:59:59.") + fraction));
    }
    for(auto value : {"", "20260920090000", "202609200900001234",
        "202609200900001234567890", "20261320090000123",
        "20260230090000123", "20260920240000123", "20260920096000123",
        "20260920090060123", "20260920090000abc", "00000920090000123"}) {
      auto source = encode_message(std::string("6=StockStatus;57=") + value);
      REQUIRE_THROWS_AS(TmxIpStockStatus::parse(StampMessage::parse(source)),
        TmxIpParserException);
    }
    auto source = encode_message("6=StockStatus;57=20260920090000123");
    auto start = source.find("56=");
    auto end = source.find(StampMessage::BUSINESS_CONTENT);
    source.replace(start, end - start, "56=invalid");
    REQUIRE_THROWS_AS(TmxIpStockStatus::parse(StampMessage::parse(source)),
      TmxIpParserException);
  }

  TEST_CASE("invalid_fields") {
    for(auto fields : {
        "6=TradeReport;5=Trade;55=ABX;64=100",
        "6=TradeReport;5=Trade;55=ABX;41=10",
        "6=TradeReport;5=Trade;41=10;64=100",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=100;40=X;40.0=Y",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=100;55=XYZ",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=100;64.1=2",
        "6=OrderCancelResp;5=Buy;55=ABX;196=10;64=100",
        "6=OrderInfo;5=OrderBook;55.1=ABX;40.1=X;70.1=1;"
          "64.1=100;197.1=Buy",
        "6=MBXMessage;5=AssignCOP;55=ABX"}) {
      auto source = encode_message(std::string(fields) +
        ";57=20260920090000123");
      CAPTURE(std::string(fields));
      REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
        StampParserException);
    }
    for(auto fields : {
        "6=TradeReport;5=Wrong;55=ABX;41=10;64=100",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=-1",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=100000000000000000000",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=1.5",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=100;183=Maybe",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=100;40.2=BAD",
        "6=TradeReport;5=Trade;55=;41=10;64=100",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=100;70=1000",
        "6=OrderCancelResp;5=Buy;55=ABX;196=10;64=100;16=Unknown",
        "6=OrderCancelResp;5=Sideways;55=ABX;196=10;64=100;16=Booked",
        "6=OrderInfo;5=OrderBook;55=ABX;40=X;70=1;64=100;197=Neither",
        "6=MBXMessage;5=AssignCOP;55=ABX;191=1;192.1=001|ORDER",
        "6=OpeningAuction;5=OddlotImbalance;572=Buyside",
        "6=OpeningAuction;5=OddlotImbalance;572=Sellside",
        "6=OpeningAuction;5=PairedVolume;578=1000000000"}) {
      auto source = encode_message(std::string(fields) +
        ";57=20260920090000123");
      CAPTURE(std::string(fields));
      REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
        TmxIpParserException);
    }
    auto source = encode_message("6=StockStatus");
    REQUIRE_THROWS_AS(validate(StampMessage::parse(source)),
      TmxIpParserException);
    source = encode_message("6=OpeningAuction;5=PairedVolume");
    REQUIRE_THROWS_AS(TmxIpTradeReport::parse(StampMessage::parse(source)),
      TmxIpParserException);
  }

  TEST_CASE("last_sale") {
    auto source = encode_message("6=TradeReport;5=Trade;55=ABX;41=12.30;"
      "64=1;57=20260920090000123;114=MKT");
    REQUIRE_THROWS_AS(TmxIpTradeReport::parse(StampMessage::parse(source)),
      TmxIpParserException);
  }

  TEST_CASE("empty_defaults") {
    auto source = encode_message("6=TradeReport;5=Trade;55=ABX;41=;64=1;"
      "57=20260920090000123;617=;684=");
    auto trade = TmxIpTradeReport::parse(StampMessage::parse(source));
    REQUIRE(trade.m_price.m_type == TmxIpPrice::Type::MARKET);
    REQUIRE(!trade.m_is_dark.value());
    REQUIRE(!trade.m_is_mid_only.value());
    source = encode_message("6=SymbolInfo;5=SymbolStatus;55=ABX;665=;"
      "57=20260920090000123");
    auto symbol = TmxIpSymbolStatus::parse(StampMessage::parse(source));
    REQUIRE(!symbol.m_is_test_symbol.value());
  }

  TEST_CASE("visit") {
    auto source = encode_message("6=TradeReport;5=Trade;55=ABX;41=10;64=1;"
      "57=20260920090000123");
    auto message = StampMessage::parse(source);
    auto result = visit(message,
      [] (const TmxIpStockStatus&) { return 1; },
      [] (const TmxIpTradeReport& trade) {
        REQUIRE(trade.m_symbol == "ABX");
        return 2;
      }, [] (const StampMessage&) { return 3; });
    REQUIRE(result == 2);
    result = visit(message, [] (const auto&) { return 4; },
      [] (const TmxIpTradeReport&) { return 5; });
    REQUIRE(result == 4);
    auto value = 6;
    auto& reference = visit(message,
      [&] (const TmxIpTradeReport&) -> int& { return value; });
    REQUIRE(&reference == &value);
    REQUIRE_THROWS_AS(visit(message,
      [] (const TmxIpStockStatus&) { return 1; }), TmxIpParserException);
    auto called = false;
    visit(message, [&] (const TmxIpStockStatus&) { called = true; });
    REQUIRE(!called);
    source = encode_message("6=FutureMessage;5=FutureAction;9999=extension");
    message = StampMessage::parse(source);
    result = visit(message, [] (const TmxIpTradeReport&) { return 1; },
      [] (const StampMessage&) { return 2; });
    REQUIRE(result == 2);
    REQUIRE_NOTHROW(validate(message));
    source = encode_message("5=NoClass");
    REQUIRE_THROWS_AS(
      validate(StampMessage::parse(source)), TmxIpParserException);
  }
}
