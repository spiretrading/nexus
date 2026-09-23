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

  OtcLinkMessage make_message(std::uint8_t type, std::string_view payload) {
    return OtcLinkMessage(static_cast<std::uint16_t>(
      OtcLinkMessage::HEADER_LENGTH + payload.size()), type, payload);
  }
}

TEST_SUITE("OtcLinkMessages") {
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
      {OtcLinkSpinEnd::TYPE, spin_end}};
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
