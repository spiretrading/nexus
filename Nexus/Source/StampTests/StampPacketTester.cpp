#include <doctest/doctest.h>
#include "Nexus/Stamp/StampPacket.hpp"

using namespace Nexus;

namespace {
  auto make_packet_buffer(const std::string& length,
      const std::string& sequence, const std::string& service_id,
      char retransmission, char continuation, const std::string& message_type,
      const std::string& exchange_id, const std::string& message) {
    auto s = std::string();
    s += char(StampPacket::START_TOKEN);
    s += length;
    s += sequence;
    s += service_id;
    s += retransmission;
    s += continuation;
    s += message_type;
    s += exchange_id;
    s += message;
    s += char(StampPacket::END_TOKEN);
    return s;
  }
}

TEST_SUITE("StampPacket") {
  TEST_CASE("parse_valid_packet") {
    auto length = "0026";
    auto sequence = "000000001";
    auto service_id = "ABC";
    auto retransmission = '0';
    auto continuation = '0';
    auto message_type = "MT";
    auto exchange_id = "EX";
    auto message = "DATA";
    auto packet_str = make_packet_buffer(length, sequence, service_id,
      retransmission, continuation, message_type, exchange_id, message);
    auto packet = StampPacket::parse(packet_str.data(), packet_str.size());
    REQUIRE(packet.m_header.m_length == 26);
    REQUIRE(packet.m_header.m_sequence_number == 1);
    REQUIRE(packet.m_header.m_service_id == "ABC");
    REQUIRE(packet.m_header.m_retransmission_identifier ==
      RetransmissionIdentifier::NORMAL);
    REQUIRE(packet.m_header.m_continuation_indicator ==
      ContinuationIndicator::STAND_ALONE);
    REQUIRE(packet.m_header.m_message_type == "MT");
    REQUIRE(packet.m_header.m_exchange_identifier == "EX");
    REQUIRE(packet.m_message_size == 4);
    REQUIRE(std::memcmp(packet.m_message, "DATA", 4) == 0);
  }

  TEST_CASE("parse_valid_packet_empty_message") {
    auto length = "0022";
    auto sequence = "000000002";
    auto service_id = "DEF";
    auto retransmission = '1';
    auto continuation = '1';
    auto message_type = "XY";
    auto exchange_id = "YZ";
    auto message = "";
    auto packet_str = make_packet_buffer(length, sequence, service_id,
      retransmission, continuation, message_type, exchange_id, message);
    auto packet = StampPacket::parse(packet_str.data(), packet_str.size());
    REQUIRE(packet.m_header.m_length == 22);
    REQUIRE(packet.m_header.m_sequence_number == 2);
    REQUIRE(packet.m_header.m_service_id == "DEF");
    REQUIRE(packet.m_header.m_retransmission_identifier ==
      RetransmissionIdentifier::OUT_OF_ORDER);
    REQUIRE(packet.m_header.m_continuation_indicator ==
      ContinuationIndicator::SPANNING);
    REQUIRE(packet.m_header.m_message_type == "XY");
    REQUIRE(packet.m_header.m_exchange_identifier == "YZ");
    REQUIRE(packet.m_message_size == 0);
  }

  TEST_CASE("parse_invalid_start_token") {
    auto length = "0026";
    auto sequence = "000000003";
    auto service_id = "GHI";
    auto retransmission = '0';
    auto continuation = '0';
    auto message_type = "ZZ";
    auto exchange_id = "AA";
    auto message = "DATA";
    auto packet_str = make_packet_buffer(length, sequence, service_id,
      retransmission, continuation, message_type, exchange_id, message);
    auto corrupted = packet_str;
    corrupted[0] = 'X';
    REQUIRE_THROWS_AS(StampPacket::parse(
      corrupted.data(), corrupted.size()), StampParserException);
  }

  TEST_CASE("parse_invalid_end_token") {
    auto length = "0026";
    auto sequence = "000000004";
    auto service_id = "JKL";
    auto retransmission = '0';
    auto continuation = '0';
    auto message_type = "AA";
    auto exchange_id = "BB";
    auto message = "DATA";
    auto packet_str = make_packet_buffer(length, sequence, service_id,
      retransmission, continuation, message_type, exchange_id, message);
    auto corrupted = packet_str;
    corrupted[corrupted.size() - 1] = 'X';
    REQUIRE_THROWS_AS(StampPacket::parse(
      corrupted.data(), corrupted.size()), StampParserException);
  }

  TEST_CASE("parse_packet_too_short") {
    auto packet_str = std::string(1, char(StampPacket::START_TOKEN));
    REQUIRE_THROWS_AS(StampPacket::parse(
      packet_str.data(), packet_str.size()), StampParserException);
  }

  TEST_CASE("parse_packet_header_too_short") {
    auto packet_str = std::string();
    packet_str += char(StampPacket::START_TOKEN);
    packet_str += "0005";
    packet_str += std::string(StampHeader::HEADER_SIZE - 4, '0');
    packet_str += char(StampPacket::END_TOKEN);
    REQUIRE_THROWS_AS(StampPacket::parse(
      packet_str.data(), packet_str.size()), StampParserException);
  }
}
