#include <array>
#include <cstdint>
#include <string_view>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Out.hpp>
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

  TEST_CASE("encode_unsequenced_header") {
    auto header = CxaPitchHeader();
    header.m_length = static_cast<std::uint16_t>(CxaPitchHeader::LENGTH + 22);
    header.m_count = 1;
    header.m_unit = 0;
    header.m_sequence = 0;
    auto buffer = Beam::SharedBuffer();
    header.encode(Beam::out(buffer));
    REQUIRE(std::string_view(buffer.get_data(), buffer.get_size()) ==
      std::string_view("\x1e\x00" "\x01" "\x00" "\x00\x00\x00\x00", 8));
  }

  TEST_CASE("parse_header_too_short") {
    auto data = std::array<char, CxaPitchHeader::LENGTH - 1>();
    REQUIRE_THROWS_AS(
      CxaPitchHeader::parse(std::string_view(data.data(), data.size())),
      CxaPitchParserException);
  }
}
