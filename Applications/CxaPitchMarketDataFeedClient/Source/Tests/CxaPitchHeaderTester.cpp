#include <array>
#include <string_view>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchHeader.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"

using namespace Nexus;

TEST_SUITE("CxaPitchHeader") {
  TEST_CASE("parse_sequenced_header") {
    auto data = std::array<char, CxaPitchHeader::LENGTH>{
      0x2A, 0x00,
      0x03,
      0x01,
      0x3B, 0x10, 0x00, 0x00};
    auto header =
      CxaPitchHeader::parse(std::string_view(data.data(), data.size()));
    REQUIRE(header.m_length == 42);
    REQUIRE(header.m_count == 3);
    REQUIRE(header.m_unit == 1);
    REQUIRE(header.m_sequence == 4155);
  }

  TEST_CASE("parse_heartbeat") {
    auto data = std::array<char, CxaPitchHeader::LENGTH>{
      0x08, 0x00,
      0x00,
      0x02,
      0x3B, 0x10, 0x00, 0x00};
    auto header =
      CxaPitchHeader::parse(std::string_view(data.data(), data.size()));
    REQUIRE(header.m_length == CxaPitchHeader::LENGTH);
    REQUIRE(header.m_count == 0);
    REQUIRE(header.m_unit == 2);
    REQUIRE(header.m_sequence == 4155);
  }

  TEST_CASE("parse_header_too_short") {
    auto data = std::array<char, CxaPitchHeader::LENGTH - 1>();
    REQUIRE_THROWS_AS(
      CxaPitchHeader::parse(std::string_view(data.data(), data.size())),
      CxaPitchParserException);
  }
}
