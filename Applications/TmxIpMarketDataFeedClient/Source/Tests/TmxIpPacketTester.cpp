#include <ostream>
#include <string>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpPacket.hpp"

using namespace Nexus;
using namespace std::literals;

TEST_SUITE("TmxIpPacket") {
  TEST_CASE("parse") {
    auto source = "\x02" "0036000000001CDF00  T "
      "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d\x03"sv;
    auto packet = TmxIpPacket::parse(source);
    REQUIRE(packet.m_header.m_length == source.size() -
      sizeof(TmxIpPacket::START) - sizeof(TmxIpPacket::END));
    REQUIRE(packet.m_header.m_sequence == std::uint32_t(1));
    REQUIRE(packet.m_header.m_service == "CDF");
    REQUIRE(packet.m_header.m_exchange == 'T');
    REQUIRE(packet.m_payload == "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d");
    REQUIRE(packet.m_payload.data() ==
      source.data() + sizeof(TmxIpPacket::START) + TmxIpHeader::LENGTH);
    auto message = parse_message(packet);
    auto field = message.m_business_content.find(55);
    REQUIRE(field.has_value());
    REQUIRE(field->m_value == "ABX");
    REQUIRE(field->m_value.data() == source.data() + source.find("ABX"));
    field = message.m_control_header.find(1);
    REQUIRE(field.has_value());
    REQUIRE(field->m_value == "H");
  }

  TEST_CASE("heartbeat") {
    auto source = "\x02" "0207         CDF00V T "
      "[HEARTBEAT 2012-10-10 03:25:02-001349853902.844623]"
      "[LAST SENT 000001345-03:05:03-001349852703.441869]"
      "[LAST HB   000001345-03:24:02-001349853842.845443]"
      "OCSA-CDF-1           ATDOTDR  00.1\x03"sv;
    auto packet = TmxIpPacket::parse(source);
    REQUIRE(is_heartbeat(packet.m_header));
    REQUIRE(!packet.m_header.m_sequence.has_value());
    constexpr auto HEARTBEAT_LENGTH = 185;
    REQUIRE(packet.m_payload.size() == HEARTBEAT_LENGTH);
    REQUIRE(packet.m_payload.starts_with("[HEARTBEAT "));
    REQUIRE_THROWS_AS(parse_message(packet), TmxIpParserException);
  }

  TEST_CASE("fragments") {
    auto first = TmxIpPacket::parse(
      "\x02" "0027000000001CDF01  T \x01\x1e" "1=H\x03");
    auto middle = TmxIpPacket::parse(
      "\x02" "0029000000002CDF03  T \x1c\x1e" "55=AB\x03");
    auto last = TmxIpPacket::parse(
      "\x02" "0024000000003CDF02  T X\x1d\x03");
    REQUIRE(first.m_header.m_continuation == TmxIpHeader::Continuation::FIRST);
    REQUIRE(
      middle.m_header.m_continuation == TmxIpHeader::Continuation::MIDDLE);
    REQUIRE(last.m_header.m_continuation == TmxIpHeader::Continuation::LAST);
    REQUIRE(first.m_header.m_sequence == std::uint32_t(1));
    REQUIRE(middle.m_header.m_sequence == std::uint32_t(2));
    REQUIRE(last.m_header.m_sequence == std::uint32_t(3));
    auto source = std::string();
    for(auto& packet : {first, middle, last}) {
      REQUIRE_THROWS_AS(parse_message(packet), TmxIpParserException);
      source.append(packet.m_payload);
    }
    auto message = StampMessage::parse(source);
    auto field = message.m_business_content.find(55);
    REQUIRE(field.has_value());
    REQUIRE(field->m_value == "ABX");
  }

  TEST_CASE("malformed_packet") {
    auto source = "\x02" "0036000000001CDF00  T "
      "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d\x03"sv;
    for(auto size = std::size_t(0); size < source.size(); ++size) {
      REQUIRE_THROWS_AS(
        TmxIpPacket::parse(source.substr(0, size)), TmxIpParserException);
    }
    auto invalid = std::string(source);
    SUBCASE("start") {
      invalid.front() = '\x01';
    }
    SUBCASE("end") {
      invalid.back() = '\x1d';
    }
    SUBCASE("length_too_small") {
      invalid.replace(sizeof(TmxIpPacket::START), std::size("0036") - 1,
        "0035");
    }
    SUBCASE("length_too_large") {
      invalid.replace(sizeof(TmxIpPacket::START), std::size("0036") - 1,
        "0037");
    }
    SUBCASE("trailing_bytes") {
      invalid += "EXTRA\x03";
    }
    SUBCASE("multiple_packets") {
      invalid += source;
    }
    REQUIRE_THROWS_AS(TmxIpPacket::parse(invalid), TmxIpParserException);
  }

  TEST_CASE("malformed_message") {
    for(auto source : {
        "\x02" "0022000000001CDF00  T \x03",
        "\x02" "0029000000001CDF00  T invalid\x03",
        "\x02" "0038000000001CDF00  T "
          "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1e" "56\x03"}) {
      auto packet = TmxIpPacket::parse(source);
      REQUIRE_THROWS_AS(parse_message(packet), StampParserException);
    }
  }
}
