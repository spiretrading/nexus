#include <doctest/doctest.h>
#include "Nexus/Stamp/StampHeader.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  auto make_header_string(const std::string& length,
      const std::string& sequence, const std::string& service_id,
      char retransmission, char continuation, const std::string& message_type,
      const std::string& exchange_id) {
    auto s = std::string();
    s += length;
    s += sequence;
    s += service_id;
    s += retransmission;
    s += continuation;
    s += message_type;
    s += exchange_id;
    return s;
  }
}

TEST_SUITE("StampHeader") {
  TEST_CASE("parse_valid_header_normal") {
    auto header_source =
      make_header_string("0025", "000000123", "ABC", '0', '0', "MT", "EX");
    auto cursor = header_source.c_str();
    auto header = StampHeader::parse(out(cursor), header_source.size());
    REQUIRE(header.m_length == 25);
    REQUIRE(header.m_sequence_number == 123);
    REQUIRE(header.m_service_id == "ABC");
    REQUIRE(
      header.m_retransmission_identifier == RetransmissionIdentifier::NORMAL);
    REQUIRE(
      header.m_continuation_indicator == ContinuationIndicator::STAND_ALONE);
    REQUIRE(header.m_message_type == "MT");
    REQUIRE(header.m_exchange_identifier == "EX");
  }

  TEST_CASE("parse_valid_header_out_of_order_spanning") {
    auto header_source =
      make_header_string("0100", "000000001", "DEF", '1', '1', "XY", "YZ");
    auto cursor = header_source.c_str();
    auto header = StampHeader::parse(out(cursor), header_source.size());
    REQUIRE(header.m_length == 100);
    REQUIRE(header.m_sequence_number == 1);
    REQUIRE(header.m_service_id == "DEF");
    REQUIRE(header.m_retransmission_identifier ==
      RetransmissionIdentifier::OUT_OF_ORDER);
    REQUIRE(header.m_continuation_indicator == ContinuationIndicator::SPANNING);
    REQUIRE(header.m_message_type == "XY");
    REQUIRE(header.m_exchange_identifier == "YZ");
  }

  TEST_CASE("parse_valid_header_none_continuation") {
    auto header_source =
      make_header_string("0005", "         ", "GHI", ' ', '2', "ZZ", "AA");
    auto cursor = header_source.c_str();
    auto header = StampHeader::parse(out(cursor), header_source.size());
    REQUIRE(header.m_length == 5);
    REQUIRE(header.m_sequence_number == 0);
    REQUIRE(header.m_service_id == "GHI");
    REQUIRE(
      header.m_retransmission_identifier == RetransmissionIdentifier::NONE);
    REQUIRE(
      header.m_continuation_indicator == ContinuationIndicator::CONTINUATION);
    REQUIRE(header.m_message_type == "ZZ");
    REQUIRE(header.m_exchange_identifier == "AA");
  }

  TEST_CASE("parse_valid_header_spanning_continuation") {
    auto header_source =
      make_header_string("0010", "000000010", "JKL", '0', '3', "AA", "BB");
    auto cursor = header_source.c_str();
    auto header = StampHeader::parse(out(cursor), header_source.size());
    REQUIRE(header.m_length == 10);
    REQUIRE(header.m_sequence_number == 10);
    REQUIRE(header.m_service_id == "JKL");
    REQUIRE(
      header.m_retransmission_identifier == RetransmissionIdentifier::NORMAL);
    REQUIRE(header.m_continuation_indicator ==
      ContinuationIndicator::SPANNING_CONTINUATION);
    REQUIRE(header.m_message_type == "AA");
    REQUIRE(header.m_exchange_identifier == "BB");
  }

  TEST_CASE("parse_invalid_length_field") {
    auto header_source =
      make_header_string("0A25", "000000123", "ABC", '0', '0', "MT", "EX");
    auto cursor = header_source.c_str();
    REQUIRE_THROWS_AS(StampHeader::parse(
      out(cursor), header_source.size()), StampParserException);
  }

  TEST_CASE("parse_invalid_sequence_field") {
    auto header_source =
      make_header_string("0025", "00000A123", "ABC", '0', '0', "MT", "EX");
    auto cursor = header_source.c_str();
    REQUIRE_THROWS_AS(StampHeader::parse(
      out(cursor), header_source.size()), StampParserException);
  }

  TEST_CASE("parse_invalid_retransmission_field") {
    auto header_source =
      make_header_string("0025", "000000123", "ABC", 'X', '0', "MT", "EX");
    auto cursor = header_source.c_str();
    REQUIRE_THROWS_AS(StampHeader::parse(
      out(cursor), header_source.size()), StampParserException);
  }

  TEST_CASE("parse_invalid_continuation_field") {
    auto header_source =
      make_header_string("0025", "000000123", "ABC", '0', 'X', "MT", "EX");
    auto cursor = header_source.c_str();
    REQUIRE_THROWS_AS(StampHeader::parse(
      out(cursor), header_source.size()), StampParserException);
  }

  TEST_CASE("parse_header_too_short") {
    auto header_source =
      make_header_string("0025", "000000123", "ABC", '0', '0', "M", "E");
    auto cursor = header_source.c_str();
    REQUIRE_THROWS_AS(
      StampHeader::parse(out(cursor), 10), StampParserException);
  }
}
