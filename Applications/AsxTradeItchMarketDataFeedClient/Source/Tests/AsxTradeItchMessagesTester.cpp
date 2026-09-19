#include <limits>
#include <doctest/doctest.h>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchMessages.hpp"

using namespace Nexus;
using namespace std::literals;

namespace {
  constexpr auto SECONDS =
    "\x54\x65\x53\xF1\x00"sv;
  constexpr auto ORDER_BOOK_DIRECTORY =
    "\x52\x36\x59\x2F\x64\x00\x01\x4E\x63\x43\x42\x41\x20\x20\x20\x20"
    "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
    "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x43\x57\x4C\x54\x48\x20\x42"
    "\x41\x4E\x4B\x20\x46\x50\x4F\x20\x5B\x43\x42\x41\x5D\x20\x20\x20"
    "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x41\x55\x30\x30\x30\x30\x30"
    "\x30\x43\x42\x41\x37\x05\x41\x55\x44\x00\x02\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00"sv;
  constexpr auto COMBINATION_ORDER_BOOK_DIRECTORY =
    "\x4D\x20\x94\xD9\x08\xFF\xFF\x4E\xFB\x54\x4D\x43\x5F\x4E\x41\x42"
    "\x5F\x44\x5F\x30\x30\x31\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
    "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
    "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
    "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
    "\x20\x20\x20\x20\x20\x0B\x41\x55\x44\x00\x02\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x4E\x41\x42\x31\x38\x4E\x4F\x56\x32\x39\x5F\x32\x38\x30\x30"
    "\x50\x2E\x42\x54\x38\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
    "\x20\x43\x00\x00\x00\x01\x4E\x41\x42\x31\x38\x4E\x4F\x56\x32\x39"
    "\x5F\x32\x39\x30\x30\x50\x2E\x42\x56\x38\x20\x20\x20\x20\x20\x20"
    "\x20\x20\x20\x20\x20\x20\x42\x00\x00\x00\x01\x20\x20\x20\x20\x20"
    "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
    "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x3F\x00\x00\x00\x00"
    "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
    "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
    "\x3F\x00\x00\x00\x00"sv;
  constexpr auto TICK_SIZE =
    "\x4C\x36\x59\x2F\x64\x00\x01\x4E\x63\x00\x00\x00\x00\x00\x00\x00"
    "\x0A\x00\x00\x00\x0A\x00\x00\x03\xDE"sv;
  constexpr auto SYSTEM_EVENT =
    "\x53\x36\x59\x2F\x64\x4F"sv;
  constexpr auto ORDER_BOOK_STATE =
    "\x4F\x36\x59\x2F\x64\x00\x01\x4E\x63\x43\x4C\x4F\x53\x45\x20\x20"
    "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"sv;
  constexpr auto ADD_ORDER =
    "\x41\x08\x8D\x07\x68\x62\x1F\x12\x82\x00\x00\xE5\xED\x00\x01\x4E"
    "\x63\x42\x00\x00\x00\x20\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00"
    "\x00\x64\x00\x00\x02"sv;
  constexpr auto ADD_ORDER_WITH_PARTICIPANT =
    "\x46\x19\xE6\xA4\xEC\x62\x1F\x12\x82\x00\x01\x21\x65\x00\x97\x51"
    "\xF2\x53\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x09"
    "\xEA\xAC\x20\x00\x02\x41\x55\x35\x35\x30\x20\x20"sv;
  constexpr auto ORDER_EXECUTED =
    "\x45\x02\xCD\x44\xB4\x63\xD0\x27\x41\x00\x01\x02\xA2\x00\x65\x51"
    "\xF2\x42\x00\x00\x00\x00\x00\x00\x03\xE8\x00\xFF\x85\x81\x00\x00"
    "\x00\x09\x00\x00\x00\x01\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
    "\x20\x20\x20\x20"sv;
  constexpr auto ORDER_EXECUTED_AT_PRICE =
    "\x43\x11\xFA\xDA\xDC\x62\x1F\x12\x82\x00\x01\x20\xAC\xFF\xFF\x51"
    "\xF2\x42\x00\x00\x00\x00\x00\x00\x00\x0A\x00\xFB\x30\xC2\x00\x00"
    "\x00\x36\x00\x00\x00\x01\x41\x55\x35\x35\x30\x20\x20\x41\x55\x35"
    "\x35\x31\x20\x20\x00\x00\x00\x32\x4E\x4E"sv;
  constexpr auto ORDER_REPLACE =
    "\x55\x1B\xC2\x1D\x3C\x62\x1F\x12\x81\x00\x01\x4A\x14\x00\x01\x14"
    "\x0D\x42\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x07\xD0\x00\x04"
    "\x1F\xDC\x00\x00"sv;
  constexpr auto ORDER_DELETE =
    "\x44\x28\x30\x6B\xF0\x62\x1F\x12\x82\x00\x01\x21\x65\x00\x97\x51"
    "\xF2\x53"sv;
  constexpr auto TRADE =
    "\x50\x1B\xC2\x1D\x3C\x00\xFB\x30\xC1\x00\x00\x00\x09\x00\x00\x00"
    "\x02\x20\x00\x00\x00\x00\x00\x00\xBB\x80\x00\x01\x14\x0D\x00\x04"
    "\x1F\xDC\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
    "\x59\x4E"sv;
  constexpr auto EQUILIBRIUM_PRICE_UPDATE =
    "\x5A\x18\x55\x85\xD0\x00\x01\x13\xEF\x00\x00\x00\x00\x00\x00\x00"
    "\x42\x00\x00\x00\x00\x00\x00\x00\x35\x00\x00\x65\xF4\x00\x00\x65"
    "\xF4\x00\x00\x65\xF4\x00\x00\x00\x00\x00\x00\x00\x42\x00\x00\x00"
    "\x00\x00\x00\x00\x35"sv;
  constexpr auto END_OF_SNAPSHOT =
    "\x47\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x31\x32\x33"
    "\x34\x35\x36\x37\x38"sv;
}

TEST_SUITE("AsxTradeItchMessages") {
  TEST_CASE("visitor_constraints") {
    auto accepts = []<typename... F> (F&&...) {
      return requires {
        visit(std::declval<const AsxTradeItchMessage&>(),
          std::declval<F>()...);
      };
    };
    auto seconds = [] (const AsxTradeItchSeconds&) {};
    auto snapshot = [] (AsxTradeItchEndOfSnapshot&&) {};
    auto fallback = [] (const AsxTradeItchMessage&) {};
    auto generic = [] (const auto&) {};
    auto unrelated = [] (int) {};
    REQUIRE(accepts(seconds));
    REQUIRE(accepts(snapshot));
    REQUIRE(accepts(fallback));
    REQUIRE(accepts(generic));
    REQUIRE(accepts(seconds, snapshot, fallback));
    REQUIRE(!accepts());
    REQUIRE(!accepts(0));
    REQUIRE(!accepts(unrelated));
    REQUIRE(!accepts([] {}));
    REQUIRE(!accepts(unrelated, seconds));
    REQUIRE(!accepts(seconds, unrelated));
    REQUIRE(!IsAsxTradeItchVisitor<decltype(&AsxTradeItchMessage::get_cursor)>);
    REQUIRE(!IsAsxTradeItchVisitor<decltype(&AsxTradeItchSeconds::m_seconds)>);
  }

  TEST_CASE("visit") {
    SUBCASE("message_types") {
      for(auto source : {SECONDS, ORDER_BOOK_DIRECTORY,
          COMBINATION_ORDER_BOOK_DIRECTORY, TICK_SIZE, SYSTEM_EVENT,
          ORDER_BOOK_STATE, ADD_ORDER, ADD_ORDER_WITH_PARTICIPANT,
          ORDER_EXECUTED, ORDER_EXECUTED_AT_PRICE, ORDER_REPLACE,
          ORDER_DELETE, TRADE, EQUILIBRIUM_PRICE_UPDATE, END_OF_SNAPSHOT}) {
        auto message = AsxTradeItchMessage::parse(source);
        auto type = visit(message, []<typename T> (const T&) {
          if constexpr(std::same_as<T, AsxTradeItchMessage>) {
            return std::uint8_t(0);
          } else {
            return T::TYPE;
          }
        });
        REQUIRE(type == message.m_type);
      }
    }
    SUBCASE("first_matching_callable") {
      auto message = AsxTradeItchMessage::parse(SECONDS);
      auto seconds = visit(message,
        [] (const AsxTradeItchAddOrder&) { return std::uint32_t(0); },
        [] (const AsxTradeItchSeconds& message) { return message.m_seconds; },
        [] (const auto&) { return std::uint32_t(1); });
      REQUIRE(seconds == 1700000000);
    }
    SUBCASE("unknown_message") {
      auto message = AsxTradeItchMessage::parse("?payload");
      auto type = visit(message,
        [] (const AsxTradeItchSeconds&) { return std::uint8_t(0); },
        [] (const AsxTradeItchMessage& message) { return message.m_type; });
      REQUIRE(type == '?');
    }
    SUBCASE("unhandled_void_message") {
      auto count = 0;
      visit(AsxTradeItchMessage::parse(SECONDS),
        [&] (const AsxTradeItchAddOrder&) { ++count; });
      REQUIRE(count == 0);
    }
    SUBCASE("unhandled_value_message") {
      REQUIRE_THROWS_AS(visit(AsxTradeItchMessage::parse(SECONDS),
        [] (const AsxTradeItchAddOrder&) { return 0; }),
        AsxTradeItchParserException);
    }
  }

  TEST_CASE("validate") {
    SUBCASE("message_lengths") {
      for(auto source : {SECONDS, ORDER_BOOK_DIRECTORY,
          COMBINATION_ORDER_BOOK_DIRECTORY, TICK_SIZE, SYSTEM_EVENT,
          ORDER_BOOK_STATE, ADD_ORDER, ADD_ORDER_WITH_PARTICIPANT,
          ORDER_EXECUTED, ORDER_EXECUTED_AT_PRICE, ORDER_REPLACE,
          ORDER_DELETE, TRADE, EQUILIBRIUM_PRICE_UPDATE, END_OF_SNAPSHOT}) {
        REQUIRE_NOTHROW(validate(AsxTradeItchMessage::parse(source)));
        for(auto size = std::size_t(1); size < source.size(); ++size) {
          REQUIRE_THROWS_AS(validate(AsxTradeItchMessage::parse(
            source.substr(0, size))), AsxTradeItchParserException);
        }
      }
    }
    SUBCASE("invalid_side") {
      constexpr auto SIDE_OFFSET = AsxTradeItchMessage::HEADER_LENGTH +
        sizeof(std::uint32_t) + sizeof(std::uint64_t) + sizeof(std::uint32_t);
      auto source = std::string(ADD_ORDER);
      source[SIDE_OFFSET] = '?';
      REQUIRE_THROWS_AS(validate(AsxTradeItchMessage::parse(source)),
        AsxTradeItchParserException);
    }
    SUBCASE("invalid_nanoseconds") {
      auto source = std::string(SYSTEM_EVENT);
      source.replace(AsxTradeItchMessage::HEADER_LENGTH,
        sizeof(std::uint32_t), "\x3B\x9A\xCA\x00"sv);
      REQUIRE_THROWS_AS(validate(AsxTradeItchMessage::parse(source)),
        AsxTradeItchParserException);
    }
    SUBCASE("invalid_snapshot_sequence") {
      auto source = std::string(END_OF_SNAPSHOT);
      source.back() = '?';
      REQUIRE_THROWS_AS(validate(AsxTradeItchMessage::parse(source)),
        AsxTradeItchParserException);
    }
    SUBCASE("unknown_message") {
      REQUIRE_NOTHROW(validate(AsxTradeItchMessage::parse("?")));
    }
  }

  TEST_CASE("seconds") {
    auto source = SECONDS;
    REQUIRE(source.size() == AsxTradeItchSeconds::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchSeconds::parse(message);
    REQUIRE(value.m_seconds == 1700000000);
  }

  TEST_CASE("order_book_directory") {
    auto source = ORDER_BOOK_DIRECTORY;
    REQUIRE(source.size() == AsxTradeItchOrderBookDirectory::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchOrderBookDirectory::parse(message);
    REQUIRE(value.m_nanoseconds == 911814500);
    REQUIRE(value.m_order_book_id == 85603);
    REQUIRE(value.m_symbol == "CBA");
    REQUIRE(value.m_long_name == "CWLTH BANK FPO [CBA]");
    REQUIRE(value.m_isin == "AU000000CBA7");
    REQUIRE(value.m_financial_product == 5);
    REQUIRE(value.m_currency == "AUD");
    REQUIRE(value.m_price_decimals == 2);
    REQUIRE(value.m_nominal_value_decimals == 0);
    REQUIRE(value.m_odd_lot_size == 0);
    REQUIRE(value.m_round_lot_size == 1);
    REQUIRE(value.m_block_lot_size == 0);
    REQUIRE(value.m_nominal_value == 0);
  }

  TEST_CASE("combination_order_book_directory") {
    auto source = COMBINATION_ORDER_BOOK_DIRECTORY;
    REQUIRE(source.size() == AsxTradeItchCombinationOrderBookDirectory::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchCombinationOrderBookDirectory::parse(message);
    REQUIRE(value.m_nanoseconds == 546625800);
    REQUIRE(value.m_order_book_id == 4294921979U);
    REQUIRE(value.m_symbol == "TMC_NAB_D_001");
    REQUIRE(value.m_long_name.empty());
    REQUIRE(value.m_isin.empty());
    REQUIRE(value.m_financial_product == 11);
    REQUIRE(value.m_currency == "AUD");
    REQUIRE(value.m_price_decimals == 2);
    REQUIRE(value.m_nominal_value_decimals == 0);
    REQUIRE(value.m_odd_lot_size == 0);
    REQUIRE(value.m_round_lot_size == 1);
    REQUIRE(value.m_block_lot_size == 0);
    REQUIRE(value.m_nominal_value == 0);
    REQUIRE(value.m_legs[0].m_symbol == "NAB18NOV29_2800P.BT8");
    REQUIRE(value.m_legs[0].m_side == 'C');
    REQUIRE(value.m_legs[0].m_ratio == 1);
    REQUIRE(value.m_legs[1].m_symbol == "NAB18NOV29_2900P.BV8");
    REQUIRE(value.m_legs[1].m_side == 'B');
    REQUIRE(value.m_legs[1].m_ratio == 1);
    REQUIRE(value.m_legs[2].m_symbol.empty());
    REQUIRE(value.m_legs[2].m_side == '?');
    REQUIRE(value.m_legs[2].m_ratio == 0);
    REQUIRE(value.m_legs[3].m_symbol.empty());
    REQUIRE(value.m_legs[3].m_side == '?');
    REQUIRE(value.m_legs[3].m_ratio == 0);
  }

  TEST_CASE("tick_size") {
    auto source = TICK_SIZE;
    REQUIRE(source.size() == AsxTradeItchTickSize::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchTickSize::parse(message);
    REQUIRE(value.m_nanoseconds == 911814500);
    REQUIRE(value.m_order_book_id == 85603);
    REQUIRE(value.m_tick_size == 10);
    REQUIRE(value.m_price_from == 10);
    REQUIRE(value.m_price_to == 990);
  }

  TEST_CASE("system_event") {
    auto source = SYSTEM_EVENT;
    REQUIRE(source.size() == AsxTradeItchSystemEvent::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchSystemEvent::parse(message);
    REQUIRE(value.m_nanoseconds == 911814500);
    REQUIRE(value.m_event_code == 'O');
  }

  TEST_CASE("order_book_state") {
    auto source = ORDER_BOOK_STATE;
    REQUIRE(source.size() == AsxTradeItchOrderBookState::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchOrderBookState::parse(message);
    REQUIRE(value.m_nanoseconds == 911814500);
    REQUIRE(value.m_order_book_id == 85603);
    REQUIRE(value.m_state == "CLOSE");
  }

  TEST_CASE("add_order") {
    auto source = ADD_ORDER;
    REQUIRE(source.size() == AsxTradeItchAddOrder::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchAddOrder::parse(message);
    REQUIRE(value.m_nanoseconds == 143460200);
    REQUIRE(value.m_order_id == 0x621F12820000E5EDULL);
    REQUIRE(value.m_order_book_id == 85603);
    REQUIRE(value.m_side == Side::BID);
    REQUIRE(value.m_order_book_position == 32);
    REQUIRE(value.m_quantity == 1);
    REQUIRE(value.m_price == 100);
    REQUIRE(value.m_exchange_order_type == 0);
    REQUIRE(value.m_lot_type == 2);
  }

  TEST_CASE("add_order_with_participant") {
    auto source = ADD_ORDER_WITH_PARTICIPANT;
    REQUIRE(source.size() == AsxTradeItchAddOrderWithParticipant::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchAddOrderWithParticipant::parse(message);
    REQUIRE(value.m_nanoseconds == 434545900);
    REQUIRE(value.m_order_id == 0x621F128200012165ULL);
    REQUIRE(value.m_order_book_id == 9916914);
    REQUIRE(value.m_side == Side::ASK);
    REQUIRE(value.m_order_book_position == 1);
    REQUIRE(value.m_quantity == 2);
    REQUIRE(value.m_price == 649900);
    REQUIRE(value.m_exchange_order_type == 8192);
    REQUIRE(value.m_lot_type == 2);
    REQUIRE(value.m_participant_id == "AU550");
  }

  TEST_CASE("order_executed") {
    auto source = ORDER_EXECUTED;
    REQUIRE(source.size() == AsxTradeItchOrderExecuted::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchOrderExecuted::parse(message);
    REQUIRE(value.m_nanoseconds == 47006900);
    REQUIRE(value.m_order_id == 0x63D02741000102A2ULL);
    REQUIRE(value.m_order_book_id == 6640114);
    REQUIRE(value.m_side == Side::BID);
    REQUIRE(value.m_executed_quantity == 1000);
    REQUIRE(value.m_match_id[0] == 0x00FF8581);
    REQUIRE(value.m_match_id[1] == 9);
    REQUIRE(value.m_match_id[2] == 1);
    REQUIRE(value.m_owner.empty());
    REQUIRE(value.m_counterparty.empty());
  }

  TEST_CASE("order_executed_at_price") {
    auto source = ORDER_EXECUTED_AT_PRICE;
    REQUIRE(source.size() == AsxTradeItchOrderExecutedAtPrice::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchOrderExecutedAtPrice::parse(message);
    REQUIRE(value.m_nanoseconds == 301652700);
    REQUIRE(value.m_order_id == 0x621F1282000120ACULL);
    REQUIRE(value.m_order_book_id == 4294922738U);
    REQUIRE(value.m_side == Side::BID);
    REQUIRE(value.m_executed_quantity == 10);
    REQUIRE(value.m_match_id[0] == 0x00FB30C2);
    REQUIRE(value.m_match_id[1] == 0x36);
    REQUIRE(value.m_match_id[2] == 1);
    REQUIRE(value.m_owner == "AU550");
    REQUIRE(value.m_counterparty == "AU551");
    REQUIRE(value.m_price == 50);
    REQUIRE(value.m_occurred_at_cross == 'N');
    REQUIRE(value.m_printable == 'N');
  }

  TEST_CASE("order_replace") {
    auto source = ORDER_REPLACE;
    REQUIRE(source.size() == AsxTradeItchOrderReplace::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchOrderReplace::parse(message);
    REQUIRE(value.m_nanoseconds == 465706300);
    REQUIRE(value.m_order_id == 0x621F128100014A14ULL);
    REQUIRE(value.m_order_book_id == 70669);
    REQUIRE(value.m_side == Side::BID);
    REQUIRE(value.m_order_book_position == 1);
    REQUIRE(value.m_quantity == 2000);
    REQUIRE(value.m_price == 270300);
    REQUIRE(value.m_exchange_order_type == 0);
  }

  TEST_CASE("order_delete") {
    auto source = ORDER_DELETE;
    REQUIRE(source.size() == AsxTradeItchOrderDelete::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchOrderDelete::parse(message);
    REQUIRE(value.m_nanoseconds == 674262000);
    REQUIRE(value.m_order_id == 0x621F128200012165ULL);
    REQUIRE(value.m_order_book_id == 9916914);
    REQUIRE(value.m_side == Side::ASK);
  }

  TEST_CASE("trade") {
    auto source = TRADE;
    REQUIRE(source.size() == AsxTradeItchTrade::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchTrade::parse(message);
    REQUIRE(value.m_nanoseconds == 465706300);
    REQUIRE(value.m_match_id[0] == 0x00FB30C1);
    REQUIRE(value.m_match_id[1] == 9);
    REQUIRE(value.m_match_id[2] == 2);
    REQUIRE(value.m_side == Side::NONE);
    REQUIRE(value.m_quantity == 48000);
    REQUIRE(value.m_order_book_id == 70669);
    REQUIRE(value.m_price == 270300);
    REQUIRE(value.m_owner.empty());
    REQUIRE(value.m_counterparty.empty());
    REQUIRE(value.m_printable == 'Y');
    REQUIRE(value.m_occurred_at_cross == 'N');
  }

  TEST_CASE("equilibrium_price_update") {
    SUBCASE("specification") {
      auto source = EQUILIBRIUM_PRICE_UPDATE;
      REQUIRE(source.size() == AsxTradeItchEquilibriumPriceUpdate::LENGTH);
      auto message = AsxTradeItchMessage::parse(source);
      auto value = AsxTradeItchEquilibriumPriceUpdate::parse(message);
      REQUIRE(value.m_nanoseconds == 408258000);
      REQUIRE(value.m_order_book_id == 70639);
      REQUIRE(value.m_bid_quantity == 66);
      REQUIRE(value.m_ask_quantity == 53);
      REQUIRE(value.m_equilibrium_price == 26100);
      REQUIRE(value.m_best_bid_price == 26100);
      REQUIRE(value.m_best_ask_price == 26100);
      REQUIRE(value.m_best_bid_quantity == 66);
      REQUIRE(value.m_best_ask_quantity == 53);
    }
    SUBCASE("distinct_fields") {
      auto source =
        "Z\x00\x00\x00\x01\x00\x00\x00\x02"
        "\x00\x00\x00\x00\x00\x00\x03\xE8"
        "\x00\x00\x00\x00\x00\x00\x03\x84"
        "\x00\x00\x30\x39\x00\x00\x30\x34\x00\x00\x30\x3E"
        "\x00\x00\x00\x00\x00\x00\x00\x64"
        "\x00\x00\x00\x00\x00\x00\x00\x32"sv;
      REQUIRE(source.size() == AsxTradeItchEquilibriumPriceUpdate::LENGTH);
      auto value = AsxTradeItchEquilibriumPriceUpdate::parse(
        AsxTradeItchMessage::parse(source));
      REQUIRE(value.m_nanoseconds == 1);
      REQUIRE(value.m_order_book_id == 2);
      REQUIRE(value.m_bid_quantity == 1000);
      REQUIRE(value.m_ask_quantity == 900);
      REQUIRE(value.m_equilibrium_price == 12345);
      REQUIRE(value.m_best_bid_price == 12340);
      REQUIRE(value.m_best_ask_price == 12350);
      REQUIRE(value.m_best_bid_quantity == 100);
      REQUIRE(value.m_best_ask_quantity == 50);
    }
  }

  TEST_CASE("end_of_snapshot") {
    auto source = END_OF_SNAPSHOT;
    REQUIRE(source.size() == AsxTradeItchEndOfSnapshot::LENGTH);
    auto message = AsxTradeItchMessage::parse(source);
    auto value = AsxTradeItchEndOfSnapshot::parse(message);
    REQUIRE(value.m_sequence == 12345678);
  }

  TEST_CASE("message_boundaries") {
    auto check = [] (std::string_view source, auto parse) {
      for(auto size = std::size_t(0); size != source.size(); ++size) {
        REQUIRE_THROWS_AS(
          parse(AsxTradeItchMessage::parse(source.substr(0, size))),
          AsxTradeItchParserException);
      }
      auto invalid = std::string(source);
      invalid.front() = '?';
      REQUIRE_THROWS_AS(parse(AsxTradeItchMessage::parse(invalid)),
        AsxTradeItchParserException);
      auto extended = std::string(source);
      extended.append("extra");
      REQUIRE_NOTHROW(parse(AsxTradeItchMessage::parse(extended)));
    };
    check(SECONDS, &AsxTradeItchSeconds::parse);
    check(ORDER_BOOK_DIRECTORY, &AsxTradeItchOrderBookDirectory::parse);
    check(COMBINATION_ORDER_BOOK_DIRECTORY,
      &AsxTradeItchCombinationOrderBookDirectory::parse);
    check(TICK_SIZE, &AsxTradeItchTickSize::parse);
    check(SYSTEM_EVENT, &AsxTradeItchSystemEvent::parse);
    check(ORDER_BOOK_STATE, &AsxTradeItchOrderBookState::parse);
    check(ADD_ORDER, &AsxTradeItchAddOrder::parse);
    check(ADD_ORDER_WITH_PARTICIPANT,
      &AsxTradeItchAddOrderWithParticipant::parse);
    check(ORDER_EXECUTED, &AsxTradeItchOrderExecuted::parse);
    check(ORDER_EXECUTED_AT_PRICE, &AsxTradeItchOrderExecutedAtPrice::parse);
    check(ORDER_REPLACE, &AsxTradeItchOrderReplace::parse);
    check(ORDER_DELETE, &AsxTradeItchOrderDelete::parse);
    check(TRADE, &AsxTradeItchTrade::parse);
    check(EQUILIBRIUM_PRICE_UPDATE, &AsxTradeItchEquilibriumPriceUpdate::parse);
    check(END_OF_SNAPSHOT, &AsxTradeItchEndOfSnapshot::parse);
  }

  TEST_CASE("nanoseconds") {
    auto check = [] (std::string_view source, auto parse) {
      static constexpr auto OFFSET = AsxTradeItchMessage::HEADER_LENGTH;
      auto data = std::string(source);
      data.replace(OFFSET, sizeof(std::uint32_t), "\x3B\x9A\xC9\xFF"sv);
      REQUIRE(parse(AsxTradeItchMessage::parse(data)).m_nanoseconds ==
        999999999);
      data.replace(OFFSET, sizeof(std::uint32_t), "\x3B\x9A\xCA\x00"sv);
      REQUIRE_THROWS_AS(parse(AsxTradeItchMessage::parse(data)),
        AsxTradeItchParserException);
    };
    check(ORDER_BOOK_DIRECTORY, &AsxTradeItchOrderBookDirectory::parse);
    check(COMBINATION_ORDER_BOOK_DIRECTORY,
      &AsxTradeItchCombinationOrderBookDirectory::parse);
    check(TICK_SIZE, &AsxTradeItchTickSize::parse);
    check(SYSTEM_EVENT, &AsxTradeItchSystemEvent::parse);
    check(ORDER_BOOK_STATE, &AsxTradeItchOrderBookState::parse);
    check(ADD_ORDER, &AsxTradeItchAddOrder::parse);
    check(ADD_ORDER_WITH_PARTICIPANT,
      &AsxTradeItchAddOrderWithParticipant::parse);
    check(ORDER_EXECUTED, &AsxTradeItchOrderExecuted::parse);
    check(ORDER_EXECUTED_AT_PRICE, &AsxTradeItchOrderExecutedAtPrice::parse);
    check(ORDER_REPLACE, &AsxTradeItchOrderReplace::parse);
    check(ORDER_DELETE, &AsxTradeItchOrderDelete::parse);
    check(TRADE, &AsxTradeItchTrade::parse);
    check(EQUILIBRIUM_PRICE_UPDATE, &AsxTradeItchEquilibriumPriceUpdate::parse);
  }

  TEST_CASE("sides") {
    SUBCASE("orders") {
      static constexpr auto SIDE_OFFSET = AsxTradeItchMessage::HEADER_LENGTH +
        sizeof(std::uint32_t) + sizeof(std::uint64_t) + sizeof(std::uint32_t);
      auto check = [] (std::string_view source, auto parse) {
        auto data = std::string(source);
        data[SIDE_OFFSET] = 'B';
        REQUIRE(parse(AsxTradeItchMessage::parse(data)).m_side == Side::BID);
        data[SIDE_OFFSET] = 'S';
        REQUIRE(parse(AsxTradeItchMessage::parse(data)).m_side == Side::ASK);
        for(auto side : {' ', 'X'}) {
          data[SIDE_OFFSET] = side;
          REQUIRE_THROWS_AS(parse(AsxTradeItchMessage::parse(data)),
            AsxTradeItchParserException);
        }
      };
      check(ADD_ORDER, &AsxTradeItchAddOrder::parse);
      check(ADD_ORDER_WITH_PARTICIPANT,
        &AsxTradeItchAddOrderWithParticipant::parse);
      check(ORDER_EXECUTED, &AsxTradeItchOrderExecuted::parse);
      check(ORDER_EXECUTED_AT_PRICE, &AsxTradeItchOrderExecutedAtPrice::parse);
      check(ORDER_REPLACE, &AsxTradeItchOrderReplace::parse);
      check(ORDER_DELETE, &AsxTradeItchOrderDelete::parse);
    }
    SUBCASE("trade") {
      static constexpr auto SIDE_OFFSET = AsxTradeItchMessage::HEADER_LENGTH +
        sizeof(std::uint32_t) + 3 * sizeof(std::uint32_t);
      auto data = std::string(TRADE);
      data[SIDE_OFFSET] = 'B';
      REQUIRE(AsxTradeItchTrade::parse(
        AsxTradeItchMessage::parse(data)).m_side == Side::BID);
      data[SIDE_OFFSET] = 'S';
      REQUIRE(AsxTradeItchTrade::parse(
        AsxTradeItchMessage::parse(data)).m_side == Side::ASK);
      data[SIDE_OFFSET] = 'X';
      REQUIRE_THROWS_AS(AsxTradeItchTrade::parse(
        AsxTradeItchMessage::parse(data)), AsxTradeItchParserException);
    }
    SUBCASE("combination_leg") {
      static constexpr auto SYMBOL_LENGTH = 32;
      static constexpr auto SIDE_OFFSET =
        AsxTradeItchOrderBookDirectory::LENGTH + SYMBOL_LENGTH;
      auto data = std::string(COMBINATION_ORDER_BOOK_DIRECTORY);
      data[SIDE_OFFSET] = 'S';
      REQUIRE_THROWS_AS(AsxTradeItchCombinationOrderBookDirectory::parse(
        AsxTradeItchMessage::parse(data)), AsxTradeItchParserException);
    }
  }

  TEST_CASE("snapshot_sequence") {
    SUBCASE("maximum") {
      auto message =
        AsxTradeItchMessage::parse("G18446744073709551615"sv);
      REQUIRE(AsxTradeItchEndOfSnapshot::parse(message).m_sequence ==
        std::numeric_limits<std::uint64_t>::max());
    }
    SUBCASE("padding") {
      for(auto field : {"                  42"sv, "42                  "sv}) {
        auto data = "G" + std::string(field);
        REQUIRE(AsxTradeItchEndOfSnapshot::parse(
          AsxTradeItchMessage::parse(data)).m_sequence == 42);
      }
    }
    SUBCASE("invalid") {
      for(auto field : {"18446744073709551616"sv, ""sv, "-1"sv, "+1"sv,
          "12X"sv, "1 2"sv, "\t1"sv}) {
        auto data = "G" + std::string(field);
        data.resize(AsxTradeItchEndOfSnapshot::LENGTH, ' ');
        REQUIRE_THROWS_AS(AsxTradeItchEndOfSnapshot::parse(
          AsxTradeItchMessage::parse(data)), AsxTradeItchParserException);
      }
    }
  }

  TEST_CASE("field_values") {
    SUBCASE("auction_without_price") {
      auto source =
        "\x5A\x01\xD0\x81\x48\x00\x01\x13\xEF\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00"
        "\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00"sv;
      auto value = AsxTradeItchEquilibriumPriceUpdate::parse(
        AsxTradeItchMessage::parse(source));
      REQUIRE(value.m_nanoseconds == 30441800);
      REQUIRE(value.m_equilibrium_price ==
        std::numeric_limits<std::int32_t>::min());
      REQUIRE(value.m_best_bid_price ==
        std::numeric_limits<std::int32_t>::min());
      REQUIRE(value.m_best_ask_price ==
        std::numeric_limits<std::int32_t>::min());
      REQUIRE(value.m_bid_quantity == 0);
      REQUIRE(value.m_ask_quantity == 0);
      REQUIRE(value.m_best_bid_quantity == 0);
      REQUIRE(value.m_best_ask_quantity == 0);
    }
    SUBCASE("negative_price_and_large_quantity") {
      auto source =
        "\x41\x08\x8D\x07\x68\x62\x1F\x12\x82\x00\x00\xE5\xED\x00\x01\x4E"
        "\x63\x42\x00\x00\x00\x20\xFE\xDC\xBA\x98\x76\x54\x32\x10\xFF\xFF"
        "\xFF\x9C\x20\x20\x02"sv;
      auto value =
        AsxTradeItchAddOrder::parse(AsxTradeItchMessage::parse(source));
      REQUIRE(value.m_quantity == 0xFEDCBA9876543210ULL);
      REQUIRE(value.m_price == -100);
      REQUIRE(value.m_exchange_order_type == 8224);
    }
    SUBCASE("execution_flags") {
      auto data = std::string(ORDER_EXECUTED_AT_PRICE);
      data[data.size() - 2] = 'Y';
      data.back() = 'N';
      auto value = AsxTradeItchOrderExecutedAtPrice::parse(
        AsxTradeItchMessage::parse(data));
      REQUIRE(value.m_occurred_at_cross == 'Y');
      REQUIRE(value.m_printable == 'N');
    }
    SUBCASE("trade_flags") {
      auto data = std::string(TRADE);
      data[data.size() - 2] = 'N';
      data.back() = 'Y';
      auto value =
        AsxTradeItchTrade::parse(AsxTradeItchMessage::parse(data));
      REQUIRE(value.m_printable == 'N');
      REQUIRE(value.m_occurred_at_cross == 'Y');
    }
    SUBCASE("directory_scales_and_lots") {
      auto source =
        "\x52\x36\x59\x2F\x64\x00\x01\x4E\x63\x43\x42\x41\x20\x20\x20\x20"
        "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
        "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x43\x57\x4C\x54\x48\x20\x42"
        "\x41\x4E\x4B\x20\x46\x50\x4F\x20\x5B\x43\x42\x41\x5D\x20\x20\x20"
        "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x41\x55\x30\x30\x30\x30\x30"
        "\x30\x43\x42\x41\x37\x05\x41\x55\x44\x00\x04\x00\x03\x12\x34\x56"
        "\x78\x90\xAB\xCD\xEF\xFE\xDC\xBA\x98\xFE\xDC\xBA\x98\x76\x54\x32"
        "\x10"sv;
      auto check = [] (const auto& value) {
        REQUIRE(value.m_price_decimals == 4);
        REQUIRE(value.m_nominal_value_decimals == 3);
        REQUIRE(value.m_odd_lot_size == 0x12345678);
        REQUIRE(value.m_round_lot_size == 0x90ABCDEFU);
        REQUIRE(value.m_block_lot_size == 0xFEDCBA98U);
        REQUIRE(value.m_nominal_value == 0xFEDCBA9876543210ULL);
      };
      check(AsxTradeItchOrderBookDirectory::parse(
        AsxTradeItchMessage::parse(source)));
      auto combination = std::string(source);
      combination.front() = 'M';
      combination.append(COMBINATION_ORDER_BOOK_DIRECTORY.substr(
        AsxTradeItchOrderBookDirectory::LENGTH));
      check(AsxTradeItchCombinationOrderBookDirectory::parse(
        AsxTradeItchMessage::parse(combination)));
    }
  }
}
