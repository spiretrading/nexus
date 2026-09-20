#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpMessages.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  std::string encode_message(std::string_view fields) {
    auto source = std::string("\x01\x1e" "56=20260920090000123456\x1c\x1e");
    source.append(fields);
    std::ranges::replace(source, ';', StampField::SEPARATOR);
    source += StampMessage::CONTROL_TRAILER;
    return source;
  }
}

TEST_SUITE("TmxIpMessages") {
  TEST_CASE("price") {
    for(auto source : {"0", "12.34567", "999999.99999", "000012.30"}) {
      auto price = TmxIpPrice::parse(source);
      REQUIRE(price.m_type == TmxIpPrice::Type::LIMIT);
      REQUIRE(price.m_value == parse_money(source));
    }
    REQUIRE(TmxIpPrice::parse("").m_type == TmxIpPrice::Type::MARKET);
    REQUIRE(TmxIpPrice::parse("MKT").m_type == TmxIpPrice::Type::MARKET);
    REQUIRE(TmxIpPrice::parse("OPG").m_type == TmxIpPrice::Type::OPENING);
    REQUIRE(TmxIpPrice::parse("MBF").m_type ==
      TmxIpPrice::Type::MUST_BE_FILLED);
    for(auto source : {"-1", "+1", ".5", "1.", "1.000001", "1000000",
        "1e2", "NaN", " 1", "1 ", "MOC", "1.2.3"}) {
      REQUIRE_THROWS_AS(TmxIpPrice::parse(source), TmxIpParserException);
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
    REQUIRE(message.m_quantity == 9999999999ULL);
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
      REQUIRE(message.m_quantity == 9999999999ULL);
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
      REQUIRE(message.m_imbalance_side.value() == "BuySide");
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

  TEST_CASE("opening_auction") {
    auto source = encode_message("6=OpeningAuction;5=PairedVolume;55=ABX;"
      "191=25.50;578=999999999;57=20260920092959123456;247=TSE");
    auto paired = TmxIpOpeningAuction::parse(StampMessage::parse(source));
    REQUIRE(paired.m_action == "PairedVolume");
    REQUIRE(paired.m_symbol.value() == "ABX");
    REQUIRE(paired.m_calculated_opening_price->m_value == parse_money("25.50"));
    REQUIRE(paired.m_paired_quantity.value() == 999999999);
    REQUIRE(!paired.m_imbalance_side);
    for(auto side : {"Buyside", "Sellside", "NA"}) {
      source = encode_message(std::string(
        "6=OpeningAuction;5=OddlotImbalance;573=0;572=") + side);
      auto imbalance = TmxIpOpeningAuction::parse(StampMessage::parse(source));
      REQUIRE(imbalance.m_action == "OddlotImbalance");
      REQUIRE(imbalance.m_imbalance_quantity.value() == 0);
      REQUIRE(!imbalance.m_calculated_opening_price);
      REQUIRE(!imbalance.m_symbol);
      REQUIRE(!imbalance.m_header.m_trading_timestamp);
      if(std::string_view(side) == "Buyside") {
        REQUIRE(imbalance.m_imbalance_side.value() == Side::BID);
      } else if(std::string_view(side) == "Sellside") {
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
        "6=TradeReport;5=Trade;55=ABX;41=10;64=10000000000",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=1.5",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=100;183=Maybe",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=100;40.2=BAD",
        "6=TradeReport;5=Trade;55=;41=10;64=100",
        "6=TradeReport;5=Trade;55=ABX;41=10;64=100;70=1000",
        "6=OrderCancelResp;5=Buy;55=ABX;196=10;64=100;16=Unknown",
        "6=OrderCancelResp;5=Sideways;55=ABX;196=10;64=100;16=Booked",
        "6=OrderInfo;5=OrderBook;55=ABX;40=X;70=1;64=100;197=Neither",
        "6=MBXMessage;5=AssignCOP;55=ABX;191=1;192.1=001|ORDER",
        "6=OpeningAuction;5=OddlotImbalance;572=BuySide",
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
