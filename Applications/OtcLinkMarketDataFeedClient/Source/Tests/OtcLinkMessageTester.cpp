#include <array>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkMessage.hpp"

using namespace Nexus;

TEST_SUITE("OtcLinkMessage") {
  TEST_CASE("parse_valid_message") {
    auto data = std::array<char, 5>{
      0x00, 0x05,
      0x11,
      0x01, 0x02
    };
    auto message =
      OtcLinkMessage::parse(std::string_view(data.data(), data.size()));
    REQUIRE(message.m_size == 5);
    REQUIRE(message.m_type == OtcLinkMessage::Type::TRADE);
    REQUIRE(message.m_payload == data.data() + OtcLinkMessage::HEADER_LENGTH);
  }

  TEST_CASE("parse_message_too_short") {
    auto data = std::array<char, 2>();
    REQUIRE_THROWS_AS(
      OtcLinkMessage::parse(std::string_view(data.data(), data.size())),
      OtcLinkParserException);
  }
}
