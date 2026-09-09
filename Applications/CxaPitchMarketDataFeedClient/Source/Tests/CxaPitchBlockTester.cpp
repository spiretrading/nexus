#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"

using namespace Nexus;

TEST_SUITE("CxaPitchBlock") {
  TEST_CASE("parse_block") {
    auto data = std::array<char, 20>{
      0x14, 0x00,
      0x02,
      0x01,
      0x64, 0x00, 0x00, 0x00,
      0x06, static_cast<char>(0x97), 0x20, 0x20, 0x20, 0x20,
      0x06, 0x2D, 0x00, 0x00, 0x00, 0x00};
    auto block =
      CxaPitchBlock::parse(std::string_view(data.data(), data.size()));
    REQUIRE(block.get_header().m_length == 20);
    REQUIRE(block.get_header().m_count == 2);
    REQUIRE(block.get_header().m_unit == 1);
    REQUIRE(block.get_header().m_sequence == 100);
    auto types = std::vector<std::uint8_t>();
    for(auto& message : block) {
      REQUIRE(message.m_length == 6);
      types.push_back(message.m_type);
    }
    REQUIRE(types == std::vector<std::uint8_t>({0x97, 0x2D}));
  }

  TEST_CASE("parse_heartbeat") {
    auto data = std::array<char, 8>{
      0x08, 0x00,
      0x00,
      0x01,
      static_cast<char>(0xF4), 0x01, 0x00, 0x00};
    auto block =
      CxaPitchBlock::parse(std::string_view(data.data(), data.size()));
    REQUIRE(block.get_header().m_count == 0);
    REQUIRE(block.get_header().m_sequence == 500);
    REQUIRE(block.begin() == block.end());
  }

  TEST_CASE("parse_unknown_message") {
    auto data = std::array<char, 30>{
      0x1E, 0x00,
      0x03,
      0x01,
      0x01, 0x00, 0x00, 0x00,
      0x06, static_cast<char>(0x97), 0x20, 0x20, 0x20, 0x20,
      0x0A, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x06, 0x2D, 0x00, 0x00, 0x00, 0x00};
    auto block =
      CxaPitchBlock::parse(std::string_view(data.data(), data.size()));
    auto types = std::vector<std::uint8_t>();
    for(auto& message : block) {
      types.push_back(message.m_type);
    }
    REQUIRE(types == std::vector<std::uint8_t>({0x97, 0x7F, 0x2D}));
  }

  TEST_CASE("parse_block_longer_than_source") {
    auto data = std::array<char, 10>{
      0x14, 0x00,
      0x01,
      0x01,
      0x01, 0x00, 0x00, 0x00,
      0x06, 0x2D};
    REQUIRE_THROWS_AS(
      CxaPitchBlock::parse(std::string_view(data.data(), data.size())),
      CxaPitchParserException);
  }

  TEST_CASE("parse_message_longer_than_block") {
    auto data = std::array<char, 14>{
      0x0E, 0x00,
      0x01,
      0x01,
      0x01, 0x00, 0x00, 0x00,
      0x2A, 0x37, 0x00, 0x00, 0x00, 0x00};
    REQUIRE_THROWS_AS(
      CxaPitchBlock::parse(std::string_view(data.data(), data.size())),
      CxaPitchParserException);
  }

  TEST_CASE("parse_malformed_second_message") {
    auto data = std::string_view(
      "\x10\x00\x02\x01\x01\x00\x00\x00"
      "\x06\x97\x00\x00\x00\x00\xff\x37", 16);
    REQUIRE_THROWS_AS(CxaPitchBlock::parse(data), CxaPitchParserException);
  }

  TEST_CASE("parse_message_count_mismatch") {
    auto data = std::string(
      "\x0e\x00\x01\x01\x01\x00\x00\x00"
      "\x06\x97\x00\x00\x00\x00", 14);
    for(auto count : {0, 2}) {
      data[2] = static_cast<char>(count);
      REQUIRE_THROWS_AS(CxaPitchBlock::parse(data), CxaPitchParserException);
    }
    data[2] = 1;
    data += char(0);
    REQUIRE_THROWS_AS(CxaPitchBlock::parse(data), CxaPitchParserException);
  }

  TEST_CASE("parse_message_too_short") {
    auto data = std::array<char, 1>{0x06};
    REQUIRE_THROWS_AS(
      CxaPitchMessage::parse(std::string_view(data.data(), data.size())),
      CxaPitchParserException);
  }

  TEST_CASE("parse_message_payload") {
    auto data = std::array<char, 6>{
      0x06, static_cast<char>(0x97), 0x20, 0x20, 0x20, 0x20};
    auto message =
      CxaPitchMessage::parse(std::string_view(data.data(), data.size()));
    REQUIRE(message.m_length == 6);
    REQUIRE(message.m_type == 0x97);
    REQUIRE(message.m_payload == data.data() + CxaPitchMessage::HEADER_LENGTH);
  }
}
