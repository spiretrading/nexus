#include <string>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkMessages.hpp"

using namespace Nexus;

namespace {
  std::string make_security() {
    return std::string("\x01\x02\x03\x04", 4) + "NLST      " +
      std::string("\x00\x00\x01\x8c\x12\x34\x56\x78", 8) +
      std::string("\x02\x01\x10\x20\x30\x40\xa3\x0a", 8) + "FA";
  }

  std::string make_quote(std::uint8_t type) {
    auto payload = std::string("\x01\x02\x03\x04\x05\x06\x07\x08", 8) +
      std::string("\x02\x6e\x10\x20\x30\x40", 6) + "CDEL" +
      std::string("\x00\x00\x00\x01\x02\x03\x04\x05", 8);
    if(type == OtcLinkFractionalQuote::TYPE) {
      payload += std::string("\x00\x00\x00\x02", 4);
    }
    payload += std::string("\x10\x20\x30\x40\xe2", 5) +
      std::string("\x00\x00\x01\x8c\x12\x34\x56\x78", 8) +
      std::string("\x00\x00\x00\x01\x01\x02\x03\x04", 8);
    if(type == OtcLinkFractionalQuote::TYPE) {
      payload += std::string("\x00\x00\x00\x03", 4);
    }
    payload += std::string("\x50\x60\x70\x80\x1e", 5) +
      std::string("\x00\x00\x01\x8c\x11\x22\x33\x44", 8) +
      std::string("\xfd\xe7\x1f", 3);
    return payload;
  }

  std::string make_quote_update(std::uint8_t type) {
    auto payload = std::string("\x01\x02\x03\x04\x05\x06\x07\x08\x89", 9) +
      std::string("\x00\x00\x00\x01\x02\x03\x04\x05", 8);
    if(type == OtcLinkFractionalQuoteUpdate::TYPE) {
      payload += std::string("\x00\x00\x00\x05", 4);
    }
    payload += std::string("\x01\x02\x03\x04\xff", 5) +
      std::string("\x00\x00\x01\x8c\x12\x34\x56\x78", 8) +
      std::string("\xfd\xe7\x15", 3);
    return payload;
  }

  std::string make_inside(std::uint8_t type) {
    auto payload = std::string("\x01\x02\x03\x04\x05\x06\x07\x08", 8) +
      std::string("\x02\xda\x10\x20\x30\x40", 6) +
      std::string("\x00\x00\x00\x01\x02\x03\x04\x05", 8);
    if(type == OtcLinkFractionalInside::TYPE) {
      payload += std::string("\x00\x00\x00\x02", 4);
    }
    payload += std::string("\x10\x20\x30\x40", 4) +
      std::string("\x00\x00\x01\x8c\x12\x34\x56\x78", 8) +
      std::string("\x00\x00\x00\x01\x01\x02\x03\x04", 8);
    if(type == OtcLinkFractionalInside::TYPE) {
      payload += std::string("\x00\x00\x00\x03", 4);
    }
    payload += std::string("\x50\x60\x70\x80", 4) +
      std::string("\x00\x00\x01\x8c\x11\x22\x33\x44", 8) +
      std::string("\x07\xff", 2);
    return payload;
  }

  std::string make_inside_update(std::uint8_t type) {
    auto payload = std::string("\x01\x02\x03\x04\x05\x06\x07\x08\x99", 9) +
      std::string("\x00\x00\x00\x01\x02\x03\x04\x05", 8);
    if(type == OtcLinkFractionalInsideUpdate::TYPE) {
      payload += std::string("\x00\x00\x00\x05", 4);
    }
    payload += std::string("\x01\x02\x03\x04", 4) +
      std::string("\x00\x00\x01\x8c\x12\x34\x56\x78\x07", 9);
    return payload;
  }

  OtcLinkMessage make_message(std::uint8_t type, std::string_view payload) {
    return OtcLinkMessage(static_cast<std::uint16_t>(
      OtcLinkMessage::HEADER_LENGTH + payload.size()), type, payload);
  }
}

TEST_SUITE("OtcLinkMessages") {
  TEST_CASE("trade") {
    auto payload = std::string("\x01\x02\x03\x04\x05\x06\x07\x08", 8) +
      std::string("\x02\xfe\x10\x20\x30\x40\x81", 7) + "ATS     " +
      std::string("\x00\x00\x00\x01\x02\x03\x04\x05", 8) +
      std::string("\x10\x20\x30\x40", 4) +
      std::string("\x00\x00\x01\x8c\x12\x34\x56\x78", 8);
    auto message = make_message(OtcLinkTrade::TYPE, payload);
    auto trade = OtcLinkTrade::parse(message);
    REQUIRE(message.m_length == OtcLinkTrade::LENGTH);
    REQUIRE(trade.m_sequence == 0x01020304);
    REQUIRE(trade.m_trade == 0x05060708);
    REQUIRE(trade.m_flags == 0xfe);
    REQUIRE(trade.m_security == 0x10203040);
    REQUIRE(trade.m_status == 0x81);
    REQUIRE(trade.has_status(OtcLinkTrade::Status::IRREGULAR));
    REQUIRE(trade.m_venue == "ATS");
    REQUIRE(trade.m_price == 0x0000000102030405);
    REQUIRE(trade.m_size == 0x10203040);
    REQUIRE(trade.m_timestamp == 0x0000018c12345678);
    REQUIRE(visit(message, [] (const OtcLinkTrade& trade) {
      return trade.m_trade;
    }) == trade.m_trade);
    REQUIRE_NOTHROW(validate(message));
    for(auto i = std::size_t(0); i < payload.size(); ++i) {
      REQUIRE_THROWS_AS(OtcLinkTrade::parse(make_message(OtcLinkTrade::TYPE,
        std::string_view(payload).substr(0, i))), OtcLinkParserException);
    }
    message.m_type = OtcLinkQuote::TYPE;
    REQUIRE_THROWS_AS(OtcLinkTrade::parse(message), OtcLinkParserException);
    constexpr auto ACTION_OFFSET = 2 * sizeof(std::uint32_t);
    payload[ACTION_OFFSET] = 3;
    REQUIRE_THROWS_AS(validate(make_message(OtcLinkTrade::TYPE, payload)),
      OtcLinkParserException);
    payload[ACTION_OFFSET] = 2;
    constexpr auto STATUS_OFFSET =
      ACTION_OFFSET + 2 * sizeof(std::uint8_t) + sizeof(std::uint32_t);
    payload[STATUS_OFFSET] = 0x80;
    payload += "extension";
    trade = OtcLinkTrade::parse(make_message(OtcLinkTrade::TYPE, payload));
    REQUIRE_FALSE(trade.has_status(OtcLinkTrade::Status::IRREGULAR));
    REQUIRE(trade.m_timestamp == 0x0000018c12345678);
  }

  TEST_CASE_TEMPLATE(
      "inside_update", T, OtcLinkInsideUpdate, OtcLinkFractionalInsideUpdate) {
    auto payload = make_inside_update(T::TYPE);
    auto message = make_message(T::TYPE, payload);
    auto inside = T::parse(message);
    REQUIRE(message.m_length == T::LENGTH);
    REQUIRE(inside.m_sequence == 0x01020304);
    REQUIRE(inside.m_inside == 0x05060708);
    REQUIRE(inside.m_flags == 0x99);
    REQUIRE(inside.has_flag(T::Flag::UPDATE_ASK));
    REQUIRE(!inside.has_flag(T::Flag::OPEN));
    REQUIRE(inside.has_flag(T::Flag::ASK_PRICED));
    REQUIRE(!inside.has_flag(T::Flag::BID_PRICED));
    REQUIRE(inside.has_flag(T::Flag::ASK_SIZE_OVERFLOW));
    REQUIRE(inside.has_flag(T::Flag::BID_SIZE_OVERFLOW));
    REQUIRE(inside.m_price == 0x0000000102030405);
    if constexpr(std::same_as<T, OtcLinkInsideUpdate>) {
      REQUIRE(inside.m_size == 0x01020304);
    } else {
      REQUIRE(inside.m_size == 0x0000000501020304);
    }
    REQUIRE(inside.m_timestamp == 0x0000018c12345678);
    REQUIRE(inside.m_participants == 7);
    for(auto i = std::size_t(0); i < payload.size(); ++i) {
      REQUIRE_THROWS_AS(T::parse(make_message(T::TYPE,
        std::string_view(payload).substr(0, i))), OtcLinkParserException);
    }
    message.m_type = OtcLinkQuoteUpdate::TYPE;
    REQUIRE_THROWS_AS(T::parse(message), OtcLinkParserException);
    constexpr auto FLAGS_OFFSET = 2 * sizeof(std::uint32_t);
    payload[FLAGS_OFFSET] = 0x66;
    payload += "extension";
    inside = T::parse(make_message(T::TYPE, payload));
    REQUIRE(inside.m_flags == 0x66);
    REQUIRE(!inside.has_flag(T::Flag::UPDATE_ASK));
    REQUIRE(inside.has_flag(T::Flag::OPEN));
    REQUIRE(!inside.has_flag(T::Flag::ASK_PRICED));
    REQUIRE(inside.has_flag(T::Flag::BID_PRICED));
    REQUIRE(!inside.has_flag(T::Flag::ASK_SIZE_OVERFLOW));
    REQUIRE(!inside.has_flag(T::Flag::BID_SIZE_OVERFLOW));
    REQUIRE(inside.m_participants == 7);
    payload.assign(T::LENGTH - OtcLinkMessage::HEADER_LENGTH, '\0');
    inside = T::parse(make_message(T::TYPE, payload));
    REQUIRE(inside.m_flags == 0);
    REQUIRE(inside.m_price == 0);
    REQUIRE(inside.m_size == 0);
    REQUIRE(inside.m_timestamp == 0);
    REQUIRE(inside.m_participants == 0);
  }

  TEST_CASE_TEMPLATE("inside", T, OtcLinkInside, OtcLinkFractionalInside) {
    auto payload = make_inside(T::TYPE);
    auto message = make_message(T::TYPE, payload);
    auto inside = T::parse(message);
    REQUIRE(message.m_length == T::LENGTH);
    REQUIRE(inside.m_sequence == 0x01020304);
    REQUIRE(inside.m_inside == 0x05060708);
    REQUIRE(inside.m_action == OtcLinkInsideAction::ADD);
    REQUIRE(inside.m_security == 0x10203040);
    REQUIRE(inside.m_flags == 0xda);
    REQUIRE(inside.has_flag(T::Flag::OPEN));
    REQUIRE(inside.has_flag(T::Flag::ASK_PRICED));
    REQUIRE(inside.has_flag(T::Flag::BID_PRICED));
    REQUIRE(inside.has_flag(T::Flag::ASK_SIZE_OVERFLOW));
    REQUIRE(inside.has_flag(T::Flag::BID_SIZE_OVERFLOW));
    REQUIRE(inside.m_ask_price == 0x0000000102030405);
    REQUIRE(inside.m_bid_price == 0x0000000101020304);
    if constexpr(std::same_as<T, OtcLinkInside>) {
      REQUIRE(inside.m_ask_size == 0x10203040);
      REQUIRE(inside.m_bid_size == 0x50607080);
    } else {
      REQUIRE(inside.m_ask_size == 0x0000000210203040);
      REQUIRE(inside.m_bid_size == 0x0000000350607080);
    }
    REQUIRE(inside.m_ask_timestamp == 0x0000018c12345678);
    REQUIRE(inside.m_bid_timestamp == 0x0000018c11223344);
    REQUIRE(inside.m_ask_participants == 7);
    REQUIRE(inside.m_bid_participants == 255);
    for(auto i = std::size_t(0); i < payload.size(); ++i) {
      REQUIRE_THROWS_AS(T::parse(make_message(T::TYPE,
        std::string_view(payload).substr(0, i))), OtcLinkParserException);
    }
    message.m_type = OtcLinkQuote::TYPE;
    REQUIRE_THROWS_AS(T::parse(message), OtcLinkParserException);
    constexpr auto ACTION_OFFSET = 2 * sizeof(std::uint32_t);
    for(auto action :
        {OtcLinkInsideAction::DELETE, OtcLinkInsideAction::SPIN}) {
      payload[ACTION_OFFSET] = static_cast<char>(action);
      REQUIRE(T::parse(make_message(T::TYPE, payload)).m_action == action);
    }
    for(auto action : {0, 1, 5, 255}) {
      payload[ACTION_OFFSET] = static_cast<char>(action);
      REQUIRE_THROWS_AS(
        T::parse(make_message(T::TYPE, payload)), OtcLinkParserException);
    }
    payload.assign(T::LENGTH - OtcLinkMessage::HEADER_LENGTH, '\0');
    payload[ACTION_OFFSET] = static_cast<char>(OtcLinkInsideAction::SPIN);
    constexpr auto FLAGS_OFFSET = ACTION_OFFSET + sizeof(std::uint8_t);
    payload[FLAGS_OFFSET] = 0x24;
    payload += "extension";
    inside = T::parse(make_message(T::TYPE, payload));
    REQUIRE(inside.m_flags == 0x24);
    REQUIRE(!inside.has_flag(T::Flag::OPEN));
    REQUIRE(!inside.has_flag(T::Flag::ASK_PRICED));
    REQUIRE(!inside.has_flag(T::Flag::BID_PRICED));
    REQUIRE(!inside.has_flag(T::Flag::ASK_SIZE_OVERFLOW));
    REQUIRE(!inside.has_flag(T::Flag::BID_SIZE_OVERFLOW));
    REQUIRE(inside.m_ask_price == 0);
    REQUIRE(inside.m_bid_price == 0);
    REQUIRE(inside.m_ask_size == 0);
    REQUIRE(inside.m_bid_size == 0);
    REQUIRE(inside.m_ask_participants == 0);
    REQUIRE(inside.m_bid_participants == 0);
  }

  TEST_CASE_TEMPLATE(
      "quote_update", T, OtcLinkQuoteUpdate, OtcLinkFractionalQuoteUpdate) {
    auto payload = make_quote_update(T::TYPE);
    auto message = make_message(T::TYPE, payload);
    auto quote = T::parse(message);
    REQUIRE(message.m_length == T::LENGTH);
    REQUIRE(quote.m_sequence == 0x01020304);
    REQUIRE(quote.m_quote == 0x05060708);
    REQUIRE(quote.m_flags == 0x89);
    REQUIRE(quote.has_flag(T::Flag::UPDATE_ASK));
    REQUIRE(!quote.has_flag(T::Flag::OPEN));
    REQUIRE(quote.has_flag(T::Flag::ASK_PRICED));
    REQUIRE(quote.has_flag(T::Flag::BID_OFFER_WANTED));
    REQUIRE(quote.m_price == 0x0000000102030405);
    if constexpr(std::same_as<T, OtcLinkQuoteUpdate>) {
      REQUIRE(quote.m_size == 0x01020304);
    } else {
      REQUIRE(quote.m_size == 0x0000000501020304);
    }
    REQUIRE(quote.m_adjustment == -1);
    REQUIRE(quote.m_timestamp == 0x0000018c12345678);
    REQUIRE(quote.m_reference == 64999);
    REQUIRE(quote.m_extended_flags == 0x15);
    REQUIRE(quote.has_flag(T::ExtendedFlag::SATURATED));
    REQUIRE(quote.has_flag(T::ExtendedFlag::ASK_AUTO_EXECUTION));
    REQUIRE(!quote.has_flag(T::ExtendedFlag::BID_AUTO_EXECUTION));
    REQUIRE(!quote.has_flag(T::ExtendedFlag::NMS_CONDITIONAL));
    REQUIRE(quote.has_flag(T::ExtendedFlag::ACCEPTS_FRACTIONAL_TRADES));
    for(auto i = std::size_t(0); i < payload.size(); ++i) {
      REQUIRE_THROWS_AS(T::parse(make_message(T::TYPE,
        std::string_view(payload).substr(0, i))), OtcLinkParserException);
    }
    message.m_type = OtcLinkSecurity::TYPE;
    REQUIRE_THROWS_AS(T::parse(message), OtcLinkParserException);
    constexpr auto FLAGS_OFFSET = 2 * sizeof(std::uint32_t);
    payload[FLAGS_OFFSET] = 0x12;
    payload.back() = static_cast<char>(0xe0);
    payload += "extension";
    quote = T::parse(make_message(T::TYPE, payload));
    REQUIRE(!quote.has_flag(T::Flag::UPDATE_ASK));
    REQUIRE(quote.has_flag(T::Flag::OPEN));
    REQUIRE(quote.has_flag(T::Flag::ASK_BID_WANTED));
    REQUIRE(quote.m_extended_flags == 0xe0);
  }

  TEST_CASE_TEMPLATE("quote", T, OtcLinkQuote, OtcLinkFractionalQuote) {
    auto payload = make_quote(T::TYPE);
    auto message = make_message(T::TYPE, payload);
    auto quote = T::parse(message);
    REQUIRE(message.m_length == T::LENGTH);
    REQUIRE(quote.m_sequence == 0x01020304);
    REQUIRE(quote.m_quote == 0x05060708);
    REQUIRE(quote.m_action == OtcLinkQuoteAction::ADD);
    REQUIRE(quote.m_security == 0x10203040);
    REQUIRE(quote.m_mpid == "CDEL");
    REQUIRE(quote.m_flags == 0x6e);
    REQUIRE(quote.has_flag(T::Flag::OPEN));
    REQUIRE(quote.has_flag(T::Flag::ASK_PRICED));
    REQUIRE(quote.has_flag(T::Flag::BID_PRICED));
    REQUIRE(quote.m_ask_price == 0x0000000102030405);
    REQUIRE(quote.m_bid_price == 0x0000000101020304);
    if constexpr(std::same_as<T, OtcLinkQuote>) {
      REQUIRE(quote.m_ask_size == 0x10203040);
      REQUIRE(quote.m_bid_size == 0x50607080);
    } else {
      REQUIRE(quote.m_ask_size == 0x0000000210203040);
      REQUIRE(quote.m_bid_size == 0x0000000350607080);
    }
    REQUIRE(quote.m_ask_adjustment == -30);
    REQUIRE(quote.m_bid_adjustment == 30);
    REQUIRE(quote.m_ask_timestamp == 0x0000018c12345678);
    REQUIRE(quote.m_bid_timestamp == 0x0000018c11223344);
    REQUIRE(quote.m_reference == 64999);
    REQUIRE(quote.m_extended_flags == 0x1f);
    REQUIRE(quote.has_flag(T::ExtendedFlag::SATURATED));
    REQUIRE(quote.has_flag(T::ExtendedFlag::ACCEPTS_FRACTIONAL_TRADES));
    for(auto i = std::size_t(0); i < payload.size(); ++i) {
      REQUIRE_THROWS_AS(T::parse(make_message(T::TYPE,
        std::string_view(payload).substr(0, i))), OtcLinkParserException);
    }
    message.m_type = OtcLinkSecurity::TYPE;
    REQUIRE_THROWS_AS(T::parse(message), OtcLinkParserException);
    constexpr auto ACTION_OFFSET = 2 * sizeof(std::uint32_t);
    for(auto action : {OtcLinkQuoteAction::DELETE, OtcLinkQuoteAction::SPIN}) {
      payload[ACTION_OFFSET] = static_cast<char>(action);
      REQUIRE(T::parse(make_message(T::TYPE, payload)).m_action == action);
    }
    for(auto action : {0, 1, 5, 255}) {
      payload[ACTION_OFFSET] = static_cast<char>(action);
      REQUIRE_THROWS_AS(
        T::parse(make_message(T::TYPE, payload)), OtcLinkParserException);
    }
  }

  TEST_CASE("packet_validation") {
    auto append = [] (std::string& buffer, auto value) {
      value = boost::endian::native_to_big(value);
      buffer.append(reinterpret_cast<const char*>(&value), sizeof(value));
    };
    auto encode = [&] (std::uint8_t type, std::string_view payload) {
      auto buffer = std::string();
      append(buffer, static_cast<std::uint16_t>(
        OtcLinkMessage::HEADER_LENGTH + payload.size()));
      append(buffer, type);
      buffer += payload;
      return buffer;
    };
    auto make_packet = [&] (const std::string& messages) {
      auto buffer = std::string();
      append(buffer, static_cast<std::uint16_t>(
        OtcLinkHeader::LENGTH + messages.size()));
      append(buffer, std::uint32_t(1));
      append(buffer, std::uint8_t(0));
      append(buffer, std::uint8_t(2));
      append(buffer, std::uint32_t(0));
      buffer += messages;
      return buffer;
    };
    auto security = encode(OtcLinkSecurity::TYPE, make_security());
    auto buffer = make_packet(security + encode(0xff, {}));
    REQUIRE_NOTHROW(validate(OtcLinkPacket::parse(buffer)));
    buffer = make_packet(security + encode(OtcLinkSecurity::TYPE, {}));
    REQUIRE_THROWS_AS(validate(OtcLinkPacket::parse(buffer)),
      OtcLinkParserException);
  }

  TEST_CASE("visitor") {
    auto spin_start = std::string(17, '\0');
    auto spin_end = std::string(21, '\0');
    spin_start[sizeof(std::uint32_t)] =
      static_cast<char>(OtcLinkSpinType::REFERENCE);
    spin_end[sizeof(std::uint32_t)] =
      static_cast<char>(OtcLinkSpinType::REFERENCE);
    auto cases = std::vector<std::pair<std::uint8_t, std::string>>{
      {OtcLinkSecurity::TYPE, make_security()},
      {OtcLinkFractionalSecurity::TYPE, make_security() + std::string(8, '\0')},
      {OtcLinkMarketOpen::TYPE, std::string(20, '\0')},
      {OtcLinkMarketClose::TYPE, std::string(16, '\0')},
      {OtcLinkSpinStart::TYPE, spin_start},
      {OtcLinkSpinEnd::TYPE, spin_end},
      {OtcLinkQuote::TYPE, make_quote(OtcLinkQuote::TYPE)},
      {OtcLinkQuoteUpdate::TYPE, make_quote_update(OtcLinkQuoteUpdate::TYPE)},
      {OtcLinkFractionalQuote::TYPE, make_quote(OtcLinkFractionalQuote::TYPE)},
      {OtcLinkFractionalQuoteUpdate::TYPE,
        make_quote_update(OtcLinkFractionalQuoteUpdate::TYPE)},
      {OtcLinkInside::TYPE, make_inside(OtcLinkInside::TYPE)},
      {OtcLinkInsideUpdate::TYPE,
        make_inside_update(OtcLinkInsideUpdate::TYPE)},
      {OtcLinkFractionalInside::TYPE,
        make_inside(OtcLinkFractionalInside::TYPE)},
      {OtcLinkFractionalInsideUpdate::TYPE,
        make_inside_update(OtcLinkFractionalInsideUpdate::TYPE)}};
    auto type = [] (const auto& message) -> std::uint8_t {
      using Message = std::remove_cvref_t<decltype(message)>;
      if constexpr(std::same_as<Message, OtcLinkMessage>) {
        return 0;
      } else {
        return Message::TYPE;
      }
    };
    for(auto& [expected, payload] : cases) {
      auto message = make_message(expected, payload);
      REQUIRE(visit(message, type) == expected);
      REQUIRE_NOTHROW(validate(message));
      message.m_payload.remove_suffix(1);
      REQUIRE_THROWS_AS(visit(message, type), OtcLinkParserException);
      REQUIRE_THROWS_AS(validate(message), OtcLinkParserException);
    }
    auto inside_payload = make_inside(OtcLinkInside::TYPE);
    auto inside_message = make_message(OtcLinkInside::TYPE, inside_payload);
    REQUIRE(visit(inside_message, [] (const OtcLinkInside& inside) {
      return inside.m_inside;
    }) == 0x05060708);
    auto inside_calls = 0;
    visit(inside_message, [&] (const OtcLinkInside&) { ++inside_calls; });
    REQUIRE(inside_calls == 1);
    inside_message.m_type = 0xff;
    visit(inside_message, [&] (const OtcLinkInside&) { ++inside_calls; });
    REQUIRE(inside_calls == 1);
    auto payload = make_security();
    auto message = make_message(OtcLinkSecurity::TYPE, payload);
    auto calls = 0;
    visit(message, [&] (const OtcLinkSecurity&) { ++calls; },
      [&] (const auto&) { calls += 10; });
    REQUIRE(calls == 1);
    auto sequence = [] (const OtcLinkSecurity& security) {
      return security.m_sequence;
    };
    REQUIRE(visit(message, sequence) == 0x01020304);
    message.m_type = 0xff;
    REQUIRE(visit(message, type) == 0);
    REQUIRE_NOTHROW(validate(message));
    visit(message, [&] (const OtcLinkSecurity&) { ++calls; });
    REQUIRE(calls == 1);
    REQUIRE_THROWS_AS(visit(message, sequence), OtcLinkParserException);
    REQUIRE(visit(message, sequence, [] (const OtcLinkMessage& message) {
      return std::uint32_t(message.m_type);
    }) == 0xff);
  }

  TEST_CASE("market_state") {
    auto payload = std::string("\x01\x02\x03\x04", 4) +
      std::string("\x00\x00\x01\x8c\x12\x34\x56\x78", 8);
    auto opening_payload = payload +
      std::string("\x00\x00\x01\x8c\x13\x34\x56\x78", 8);
    auto message = make_message(OtcLinkMarketOpen::TYPE, opening_payload);
    auto opening = OtcLinkMarketOpen::parse(message);
    REQUIRE(message.m_length == OtcLinkMarketOpen::LENGTH);
    REQUIRE(opening.m_sequence == 0x01020304);
    REQUIRE(opening.m_timestamp == 0x0000018c12345678);
    REQUIRE(opening.m_close == 0x0000018c13345678);
    for(auto i = std::size_t(0); i < opening_payload.size(); ++i) {
      REQUIRE_THROWS_AS(OtcLinkMarketOpen::parse(make_message(
        OtcLinkMarketOpen::TYPE,
        std::string_view(opening_payload).substr(0, i))),
        OtcLinkParserException);
    }
    REQUIRE_THROWS_AS(OtcLinkMarketClose::parse(message),
      OtcLinkParserException);
    payload += std::string("\x00\x10\x20\x30", 4);
    message = make_message(OtcLinkMarketClose::TYPE, payload);
    auto closing = OtcLinkMarketClose::parse(message);
    REQUIRE(message.m_length == OtcLinkMarketClose::LENGTH);
    REQUIRE(closing.m_sequence == 0x01020304);
    REQUIRE(closing.m_timestamp == 0x0000018c12345678);
    REQUIRE(closing.m_count == 0x00102030);
    for(auto i = std::size_t(0); i < payload.size(); ++i) {
      REQUIRE_THROWS_AS(OtcLinkMarketClose::parse(make_message(
        OtcLinkMarketClose::TYPE, std::string_view(payload).substr(0, i))),
        OtcLinkParserException);
    }
    REQUIRE_THROWS_AS(OtcLinkMarketOpen::parse(message),
      OtcLinkParserException);
  }

  TEST_CASE("fractional_security") {
    auto payload = make_security() +
      std::string("\x00\x00\x01\x2c\x00\x00\x00\x05", 8);
    auto message = make_message(OtcLinkFractionalSecurity::TYPE, payload);
    auto security = OtcLinkFractionalSecurity::parse(message);
    REQUIRE(message.m_length == OtcLinkFractionalSecurity::LENGTH);
    REQUIRE(security.m_sequence == 0x01020304);
    REQUIRE(security.m_symbol == "NLST");
    REQUIRE(security.m_security == 0x10203040);
    REQUIRE(security.m_timestamp == 0x0000018c12345678);
    REQUIRE(security.m_action == OtcLinkSecurityAction::ADD);
    REQUIRE(security.m_asset_class == OtcLinkAssetClass::EQUITY);
    REQUIRE(security.m_tier == OtcLinkTier::OTCQB);
    REQUIRE(security.m_reporting_status == 'F');
    REQUIRE(security.m_status == 'A');
    REQUIRE(security.m_flags == 0xa3);
    REQUIRE(security.m_minimum_notional_value == 300);
    REQUIRE(security.m_minimum_quote_size == 5);
    for(auto i = std::size_t(0); i < payload.size(); ++i) {
      REQUIRE_THROWS_AS(OtcLinkFractionalSecurity::parse(make_message(
        OtcLinkFractionalSecurity::TYPE,
        std::string_view(payload).substr(0, i))), OtcLinkParserException);
    }
    message.m_type = OtcLinkSecurity::TYPE;
    REQUIRE_THROWS_AS(OtcLinkFractionalSecurity::parse(message),
      OtcLinkParserException);
    auto base_size =
      OtcLinkSecurity::LENGTH - OtcLinkMessage::HEADER_LENGTH;
    payload.replace(base_size, 2 * sizeof(std::uint32_t),
      2 * sizeof(std::uint32_t), '\0');
    security = OtcLinkFractionalSecurity::parse(
      make_message(OtcLinkFractionalSecurity::TYPE, payload));
    REQUIRE(security.m_minimum_notional_value == 0);
    REQUIRE(security.m_minimum_quote_size == 0);
  }

  TEST_CASE("security_attributes") {
    auto payload = make_security();
    constexpr auto SYMBOL_OFFSET = sizeof(std::uint32_t);
    constexpr auto SYMBOL_LENGTH = 10;
    constexpr auto ACTION_OFFSET = 22;
    constexpr auto ASSET_CLASS_OFFSET = 23;
    constexpr auto TIER_OFFSET = 29;
    constexpr auto REPORTING_STATUS_OFFSET = 30;
    constexpr auto STATUS_OFFSET = 31;
    payload.replace(SYMBOL_OFFSET, SYMBOL_LENGTH, SYMBOL_LENGTH, '\0');
    payload[ASSET_CLASS_OFFSET] =
      static_cast<char>(OtcLinkAssetClass::FIXED_INCOME);
    auto security = OtcLinkSecurity::parse(
      make_message(OtcLinkSecurity::TYPE, payload));
    REQUIRE(security.m_symbol.empty());
    REQUIRE(security.m_asset_class == OtcLinkAssetClass::FIXED_INCOME);
    payload.replace(SYMBOL_OFFSET, SYMBOL_LENGTH, "ABCDEFGHIJ");
    for(auto action : {OtcLinkSecurityAction::UPDATE,
        OtcLinkSecurityAction::ADD, OtcLinkSecurityAction::DELETE,
        OtcLinkSecurityAction::SPIN}) {
      payload[ACTION_OFFSET] = static_cast<char>(action);
      security = OtcLinkSecurity::parse(
        make_message(OtcLinkSecurity::TYPE, payload));
      REQUIRE(security.m_action == action);
      REQUIRE(security.m_symbol == "ABCDEFGHIJ");
    }
    payload[ASSET_CLASS_OFFSET] = 99;
    payload[TIER_OFFSET] = 99;
    payload[REPORTING_STATUS_OFFSET] = '?';
    payload[STATUS_OFFSET] = '?';
    payload += "extension";
    security = OtcLinkSecurity::parse(
      make_message(OtcLinkSecurity::TYPE, payload));
    REQUIRE(static_cast<std::uint8_t>(security.m_asset_class) == 99);
    REQUIRE(static_cast<std::uint8_t>(security.m_tier) == 99);
    REQUIRE(security.m_reporting_status == '?');
    REQUIRE(security.m_status == '?');
  }

  TEST_CASE("security") {
    auto payload = make_security();
    auto message = make_message(OtcLinkSecurity::TYPE, payload);
    auto security = OtcLinkSecurity::parse(message);
    REQUIRE(message.m_length == OtcLinkSecurity::LENGTH);
    REQUIRE(security.m_sequence == 0x01020304);
    REQUIRE(security.m_symbol == "NLST");
    REQUIRE(security.m_timestamp == 0x0000018c12345678);
    REQUIRE(security.m_action == OtcLinkSecurityAction::ADD);
    REQUIRE(security.m_asset_class == OtcLinkAssetClass::EQUITY);
    REQUIRE(security.m_security == 0x10203040);
    REQUIRE(security.m_flags == 0xa3);
    REQUIRE(security.has_flag(OtcLinkSecurity::Flag::CAVEAT_EMPTOR));
    REQUIRE(security.has_flag(OtcLinkSecurity::Flag::ECN_ELIGIBLE));
    REQUIRE(security.has_flag(OtcLinkSecurity::Flag::SATURATION_ELIGIBLE));
    REQUIRE(!security.has_flag(OtcLinkSecurity::Flag::MESSAGING_DISABLED));
    REQUIRE(security.m_tier == OtcLinkTier::OTCQB);
    REQUIRE(security.m_reporting_status == 'F');
    REQUIRE(security.m_status == 'A');
    for(auto i = std::size_t(0); i < payload.size(); ++i) {
      auto truncated = make_message(
        OtcLinkSecurity::TYPE, std::string_view(payload).substr(0, i));
      REQUIRE_THROWS_AS(
        OtcLinkSecurity::parse(truncated), OtcLinkParserException);
    }
    message.m_type = OtcLinkMarketOpen::TYPE;
    REQUIRE_THROWS_AS(OtcLinkSecurity::parse(message), OtcLinkParserException);
    constexpr auto ACTION_OFFSET = 22;
    for(auto action : {0, 5, 255}) {
      payload[ACTION_OFFSET] = static_cast<char>(action);
      REQUIRE_THROWS_AS(OtcLinkSecurity::parse(
        make_message(OtcLinkSecurity::TYPE, payload)), OtcLinkParserException);
    }
  }
}
