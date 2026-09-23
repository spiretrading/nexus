#include <limits>
#include <ostream>
#include <string>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkPacket.hpp"

using namespace Nexus;
using namespace std::literals;

namespace {
  std::string make_packet(std::string_view messages, std::uint8_t count,
      std::uint8_t flags) {
    auto length = boost::endian::native_to_big(
      static_cast<std::uint16_t>(OtcLinkHeader::LENGTH + messages.size()));
    auto source = std::string(
      reinterpret_cast<const char*>(&length), sizeof(length));
    source += "\x01\x23\x45\x67"sv;
    source += static_cast<char>(flags);
    source += static_cast<char>(count);
    source += "\x01\x02\x03\x04"sv;
    source += messages;
    return source;
  }
}

TEST_SUITE("OtcLinkPacket") {
  TEST_CASE("parse") {
    auto first = "\x00\x17\x0D\x00\x00\x00\x2A"
      "\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00"sv;
    auto second = "\x00\x13\x0E\x00\x00\x00\x2B"
      "\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00"sv;
    auto messages = std::string(first) + std::string(second);
    SUBCASE("messages") {
      auto source = make_packet(messages, 2, 0);
      auto packet = OtcLinkPacket::parse(source);
      REQUIRE(packet.get_header().m_length == source.size());
      REQUIRE(packet.get_header().m_count == 2);
      REQUIRE(packet.get_header().m_sequence == 0x01234567);
      REQUIRE(packet.get_payload() == messages);
      REQUIRE(packet.get_payload().data() ==
        source.data() + OtcLinkHeader::LENGTH);
      auto i = packet.begin();
      REQUIRE(i != packet.end());
      REQUIRE(i->m_type == 13);
      REQUIRE(i->m_length == first.size());
      REQUIRE(i->get_cursor().read_uint32() == 42);
      auto j = i++;
      REQUIRE(j->m_type == 13);
      REQUIRE(i->m_type == 14);
      REQUIRE((*i).m_length == second.size());
      REQUIRE(i->get_cursor().read_uint32() == 43);
      ++i;
      REQUIRE(i == packet.end());
      auto count = 0;
      for(auto& message : packet) {
        REQUIRE(message.m_length >= OtcLinkMessage::HEADER_LENGTH);
        ++count;
      }
      REQUIRE(count == packet.get_header().m_count);
    }
    SUBCASE("control_packets") {
      for(auto flags : {0, 1, 2, 3, 0x41, 0x82}) {
        auto source = make_packet({}, 0, static_cast<std::uint8_t>(flags));
        auto packet = OtcLinkPacket::parse(source);
        REQUIRE(packet.get_payload().empty());
        REQUIRE(packet.begin() == packet.end());
        REQUIRE(packet.get_header().m_flags == flags);
        REQUIRE(packet.get_header().m_sequence == 0x01234567);
      }
    }
    SUBCASE("control_payload") {
      for(auto flags : {1, 2, 3, 0x41, 0x82}) {
        for(auto count : {0, 1}) {
          auto source = make_packet(first, static_cast<std::uint8_t>(count),
            static_cast<std::uint8_t>(flags));
          REQUIRE_THROWS_AS(OtcLinkPacket::parse(source),
            OtcLinkParserException);
        }
      }
    }
    SUBCASE("unknown_type_and_extension") {
      auto source = make_packet("\x00\x09\xFF" "abcdef"sv, 1, 0xFC);
      auto packet = OtcLinkPacket::parse(source);
      REQUIRE(packet.get_header().m_flags == 0xFC);
      REQUIRE(packet.begin()->m_type == 0xFF);
      REQUIRE(packet.begin()->m_payload == "abcdef");
    }
    SUBCASE("truncation") {
      auto source = make_packet(messages, 2, 0);
      for(auto i = std::size_t(0); i < source.size(); ++i) {
        REQUIRE_THROWS_AS(OtcLinkPacket::parse(std::string_view(source).
          substr(0, i)), OtcLinkParserException);
      }
      source += '\0';
      REQUIRE_THROWS_AS(OtcLinkPacket::parse(source), OtcLinkParserException);
    }
    SUBCASE("message_count") {
      for(auto count : {0, 1, 3}) {
        auto source =
          make_packet(messages, static_cast<std::uint8_t>(count), 0);
        REQUIRE_THROWS_AS(OtcLinkPacket::parse(source), OtcLinkParserException);
      }
      auto source = make_packet({}, 1, 0);
      REQUIRE_THROWS_AS(OtcLinkPacket::parse(source), OtcLinkParserException);
      source = make_packet(messages + "x", 2, 0);
      REQUIRE_THROWS_AS(OtcLinkPacket::parse(source), OtcLinkParserException);
    }
    SUBCASE("malformed_second_message") {
      for(auto i = std::size_t(0); i < second.size(); ++i) {
        auto source = make_packet(std::string(first) +
          std::string(second.substr(0, i)), 2, 0);
        REQUIRE_THROWS_AS(OtcLinkPacket::parse(source), OtcLinkParserException);
      }
      for(auto length = std::size_t(0);
          length < OtcLinkMessage::HEADER_LENGTH; ++length) {
        auto malformed = std::string(OtcLinkMessage::HEADER_LENGTH, '\0');
        malformed[sizeof(std::uint8_t)] = static_cast<char>(length);
        auto source = make_packet(std::string(first) + malformed, 2, 0);
        REQUIRE_THROWS_AS(OtcLinkPacket::parse(source), OtcLinkParserException);
      }
    }
    SUBCASE("maximum_count") {
      auto messages = std::string();
      auto count = std::numeric_limits<std::uint8_t>::max();
      for(auto i = 0; i < count; ++i) {
        messages += first;
      }
      auto source = make_packet(messages, count, 0);
      auto packet = OtcLinkPacket::parse(source);
      auto actual = 0;
      for(auto& message : packet) {
        REQUIRE(message.m_type == 13);
        ++actual;
      }
      REQUIRE(actual == count);
    }
  }
}
