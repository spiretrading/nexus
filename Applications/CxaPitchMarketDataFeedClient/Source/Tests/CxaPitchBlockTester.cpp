#include <algorithm>
#include <array>
#include <vector>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"

using namespace Nexus;

TEST_SUITE("CxaPitchBlock") {
  TEST_CASE("payload") {
    auto source = std::string_view();
    auto expected = std::string_view();
    SUBCASE("messages") {
      source = std::string_view(
        "\x0c\x00\x02\x01\x64\x00\x00\x00\x02\x7e\x02\x7f", 12);
      expected = std::string_view("\x02\x7e\x02\x7f", 4);
    }
    SUBCASE("heartbeat") {
      source = std::string_view("\x08\x00\x00\x01\x64\x00\x00\x00", 8);
    }
    auto payload = CxaPitchBlock::parse(source).get_payload();
    REQUIRE(payload == expected);
    REQUIRE(payload.data() == source.data() + CxaPitchHeader::LENGTH);
  }

  TEST_CASE("parse") {
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
    std::ranges::transform(block, std::back_inserter(types),
      [] (const auto& message) {
        REQUIRE(message.m_length == 6);
        return message.m_type;
      });
    REQUIRE(types == std::vector<std::uint8_t>{0x97, 0x2D});
  }

  TEST_CASE("parse_grown_message") {
    auto data = std::string_view(
      "\x16\x00\x02\x01\x64\x00\x00\x00"
      "\x08\x97\x20\x20\x20\x20\xab\xcd"
      "\x06\x2d\x00\x00\x00\x00", 22);
    auto block = CxaPitchBlock::parse(data);
    REQUIRE(std::ranges::distance(block) == 2);
    auto iterator = block.begin();
    auto first = iterator++;
    REQUIRE(first->m_type == 0x97);
    REQUIRE(first->m_length == 8);
    REQUIRE(std::string_view(first->m_payload, 6) ==
      std::string_view("\x20\x20\x20\x20\xab\xcd", 6));
    REQUIRE(iterator != block.end());
    REQUIRE(iterator->m_type == 0x2D);
    REQUIRE(iterator->m_length == 6);
    REQUIRE(std::string_view(iterator->m_payload, 4) ==
      std::string_view("\x00\x00\x00\x00", 4));
    ++iterator;
    REQUIRE(iterator == block.end());
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
    REQUIRE(types == std::vector<std::uint8_t>{0x97, 0x7F, 0x2D});
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
}
