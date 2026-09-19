#include <array>
#include <cstring>
#include <string>
#include <doctest/doctest.h>
#include "Nexus/MoldUdp64/MoldUdp64Packet.hpp"

using namespace Nexus;

TEST_SUITE("MoldUdp64Packet") {
  TEST_CASE("parse") {
    auto source = std::string_view("ABCDEFGHIJ"
      "\x01\x02\x03\x04\x05\x06\x07\x08\x00\x02"
      "\x00\x03X\x00Y\x00\x00", 27);
    SUBCASE("fields") {
      auto packet = MoldUdp64Packet::parse(source);
      REQUIRE(packet.m_session == "ABCDEFGHIJ");
      REQUIRE(packet.m_sequence_number == 0x0102030405060708ULL);
      REQUIRE(packet.m_count == 2);
      REQUIRE(packet.m_payload ==
        source.substr(MoldUdp64Packet::PACKET_LENGTH));
      REQUIRE_FALSE(packet.is_heartbeat());
      REQUIRE_FALSE(packet.is_end_of_session());
    }
    SUBCASE("unaligned") {
      alignas(std::uint64_t) auto buffer = std::array<char, 28>();
      std::memcpy(buffer.data() + 1, source.data(), source.size());
      auto packet = MoldUdp64Packet::parse(
        std::string_view(buffer.data() + 1, source.size()));
      REQUIRE(packet.m_session == "ABCDEFGHIJ");
      REQUIRE(packet.m_sequence_number == 0x0102030405060708ULL);
      REQUIRE(packet.m_count == 2);
    }
    SUBCASE("truncated") {
      for(auto size = std::size_t(0); size != source.size(); ++size) {
        REQUIRE_THROWS_AS(MoldUdp64Packet::parse(source.substr(0, size)),
          MoldUdp64ParserException);
      }
    }
    SUBCASE("count") {
      auto buffer = std::string(source);
      SUBCASE("too_small") {
        buffer[MoldUdp64Packet::PACKET_LENGTH - 1] = 1;
      }
      SUBCASE("too_large") {
        buffer[MoldUdp64Packet::PACKET_LENGTH - 1] = 3;
      }
      SUBCASE("heartbeat_payload") {
        buffer[MoldUdp64Packet::PACKET_LENGTH - 1] = 0;
      }
      SUBCASE("end_of_session_payload") {
        buffer[MoldUdp64Packet::PACKET_LENGTH - 2] = '\xFF';
        buffer[MoldUdp64Packet::PACKET_LENGTH - 1] = '\xFF';
      }
      REQUIRE_THROWS_AS(
        MoldUdp64Packet::parse(buffer), MoldUdp64ParserException);
    }
    SUBCASE("trailing_bytes") {
      auto buffer = std::string(source);
      buffer.push_back('X');
      REQUIRE_THROWS_AS(
        MoldUdp64Packet::parse(buffer), MoldUdp64ParserException);
    }
  }

  TEST_CASE("iteration") {
    SUBCASE("messages") {
      auto source = std::string_view("ABCDEFGHIJ"
        "\x00\x00\x00\x00\x00\x00\x00\x2A\x00\x03"
        "\x00\x03X\x00Y\x00\x00\x00\x01Z", 30);
      auto packet = MoldUdp64Packet::parse(source);
      auto expected = std::array{
        std::string_view("X\x00Y", 3), std::string_view(),
        std::string_view("Z")};
      auto index = std::size_t(0);
      for(auto& message : packet) {
        REQUIRE(index < expected.size());
        REQUIRE(message.get_payload() == expected[index]);
        ++index;
      }
      REQUIRE(index == expected.size());
      auto iterator = packet.begin();
      REQUIRE((iterator++)->get_payload() == expected.front());
      REQUIRE(iterator->get_payload().empty());
      ++iterator;
      REQUIRE((*iterator).get_payload() == expected.back());
      ++iterator;
      REQUIRE(iterator == packet.end());
    }
    SUBCASE("heartbeat") {
      auto source = std::string_view("ABCDEFGHIJ"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00", 20);
      auto packet = MoldUdp64Packet::parse(source);
      REQUIRE(packet.is_heartbeat());
      REQUIRE_FALSE(packet.is_end_of_session());
      REQUIRE(packet.m_sequence_number == 0xFFFFFFFFFFFFFFFFULL);
      REQUIRE(packet.m_payload.empty());
      REQUIRE(packet.begin() == packet.end());
    }
    SUBCASE("end_of_session") {
      auto source = std::string_view("ABCDEFGHIJ"
        "\x00\x00\x00\x00\x00\x00\x00\x2A\xFF\xFF", 20);
      auto packet = MoldUdp64Packet::parse(source);
      REQUIRE_FALSE(packet.is_heartbeat());
      REQUIRE(packet.is_end_of_session());
      REQUIRE(packet.m_sequence_number == 42);
      REQUIRE(packet.m_payload.empty());
      REQUIRE(packet.begin() == packet.end());
    }
  }
}
