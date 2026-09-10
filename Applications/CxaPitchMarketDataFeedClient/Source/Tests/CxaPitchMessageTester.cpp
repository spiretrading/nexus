#include <array>
#include <string_view>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchMessage.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("CxaPitchMessage") {
  TEST_CASE("parse_payload") {
    auto data = std::array<char, 6>{
      0x06, static_cast<char>(0x97), 0x20, 0x20, 0x20, 0x20};
    auto message =
      CxaPitchMessage::parse(std::string_view(data.data(), data.size()));
    REQUIRE(message.m_length == 6);
    REQUIRE(message.m_type == 0x97);
    REQUIRE(message.m_payload == data.data() + CxaPitchMessage::HEADER_LENGTH);
    REQUIRE(message.get_cursor().read_uint8() == 0x20);
  }

  TEST_CASE("parse_truncated_message") {
    auto data = std::array<char, 1>{0x06};
    REQUIRE_THROWS_AS(
      CxaPitchMessage::parse(std::string_view(data.data(), data.size())),
      CxaPitchParserException);
  }

  TEST_CASE("parse_length_out_of_range") {
    for(auto length : {0x01, 0x08}) {
      auto data = std::array<char, 6>{
        static_cast<char>(length), 0x2D, 0x00, 0x00, 0x00, 0x00};
      REQUIRE_THROWS_AS(
        CxaPitchMessage::parse(std::string_view(data.data(), data.size())),
        CxaPitchParserException);
    }
  }

  TEST_CASE("stream_unknown_message") {
    auto data = std::string_view("\x06\x7f\x00\x00\x00\x00", 6);
    REQUIRE(to_string(CxaPitchMessage::parse(data)) == "(unknown 0x7F 6)");
  }
}
