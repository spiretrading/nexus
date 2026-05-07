#include <doctest/doctest.h>
#include <Beam/IO/SharedBuffer.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkReplayRequestMessage.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("OtcLinkReplayRequestMessage") {
  TEST_CASE("parse_gap_fill_request") {
    auto source = std::string_view(
      "35=BW\x01"
      "49=SENDER\x01"
      "1346=REQ123\x01"
      "1347=0\x01"
      "1355=11\x01"
      "1182=100\x01"
      "1183=200\x01"
      "10=123\x01");
    auto message = OtcLinkReplayRequestMessage::parse(source);
    REQUIRE(message.m_sender_comp_id == "SENDER");
    REQUIRE(message.m_appl_req_id == "REQ123");
    REQUIRE(
      message.m_appl_req_type == OtcLinkReplayRequestMessage::Type::GAP_FILL);
    REQUIRE(message.m_channel == OtcLinkChannelId::QUOTE_BOOK_REAL_TIME);
    REQUIRE(message.m_appl_beg_seq_no.value() == 100);
    REQUIRE(message.m_appl_end_seq_no.value() == 200);
  }

  TEST_CASE("parse_snapshot_request") {
    auto source = std::string_view(
      "35=BW\x01"
      "49=CLIENT\x01"
      "1346=SNAP456\x01"
      "1347=1\x01"
      "1355=12\x01"
      "10=045\x01");
    auto message = OtcLinkReplayRequestMessage::parse(source);
    REQUIRE(message.m_sender_comp_id == "CLIENT");
    REQUIRE(message.m_appl_req_id == "SNAP456");
    REQUIRE(
      message.m_appl_req_type == OtcLinkReplayRequestMessage::Type::SNAPSHOT);
    REQUIRE(message.m_channel == OtcLinkChannelId::QUOTE_BOOK_SNAPSHOT);
    REQUIRE_FALSE(message.m_appl_beg_seq_no.has_value());
    REQUIRE_FALSE(message.m_appl_end_seq_no.has_value());
  }

  TEST_CASE("parse_defaults_to_gap_fill") {
    auto source = std::string_view(
      "35=BW\x01"
      "49=TEST\x01"
      "1346=ID1\x01"
      "1355=14\x01"
      "10=000\x01");
    auto message = OtcLinkReplayRequestMessage::parse(source);
    REQUIRE(
      message.m_appl_req_type == OtcLinkReplayRequestMessage::Type::GAP_FILL);
  }

  TEST_CASE("make_gap_fill_request") {
    auto message = OtcLinkReplayRequestMessage::make_gap_fill_request(
      "SENDER", "REQ1", OtcLinkChannelId::QUOTE_BOOK_REAL_TIME, 100, 200);
    REQUIRE(message.m_sender_comp_id == "SENDER");
    REQUIRE(message.m_appl_req_id == "REQ1");
    REQUIRE(
      message.m_appl_req_type == OtcLinkReplayRequestMessage::Type::GAP_FILL);
    REQUIRE(message.m_channel == OtcLinkChannelId::QUOTE_BOOK_REAL_TIME);
    REQUIRE(message.m_appl_beg_seq_no.value() == 100);
    REQUIRE(message.m_appl_end_seq_no.value() == 200);
  }

  TEST_CASE("make_snapshot_request") {
    auto message = OtcLinkReplayRequestMessage::make_snapshot_request(
      "CLIENT", "SNAP1", OtcLinkChannelId::QUOTE_INSIDE_SNAPSHOT);
    REQUIRE(message.m_sender_comp_id == "CLIENT");
    REQUIRE(message.m_appl_req_id == "SNAP1");
    REQUIRE(
      message.m_appl_req_type == OtcLinkReplayRequestMessage::Type::SNAPSHOT);
    REQUIRE(message.m_channel == OtcLinkChannelId::QUOTE_INSIDE_SNAPSHOT);
    REQUIRE_FALSE(message.m_appl_beg_seq_no.has_value());
    REQUIRE_FALSE(message.m_appl_end_seq_no.has_value());
  }

  TEST_CASE("write_gap_fill_request") {
    auto message = OtcLinkReplayRequestMessage::make_gap_fill_request(
      "WRITER", "WRITE1", OtcLinkChannelId::TRADE_REAL_TIME, 500, 600);
    auto buffer = SharedBuffer();
    write(message, out(buffer));
    auto parsed = OtcLinkReplayRequestMessage::parse(
      std::string_view(buffer.get_data(), buffer.get_size()));
    REQUIRE(parsed.m_sender_comp_id == "WRITER");
    REQUIRE(parsed.m_appl_req_id == "WRITE1");
    REQUIRE(
      parsed.m_appl_req_type == OtcLinkReplayRequestMessage::Type::GAP_FILL);
    REQUIRE(parsed.m_channel == OtcLinkChannelId::TRADE_REAL_TIME);
    REQUIRE(parsed.m_appl_beg_seq_no.value() == 500);
    REQUIRE(parsed.m_appl_end_seq_no.value() == 600);
  }

  TEST_CASE("write_snapshot_request") {
    auto message = OtcLinkReplayRequestMessage::make_snapshot_request(
      "SNAPPER", "SNAP2", OtcLinkChannelId::QUOTE_BOOK_SNAPSHOT);
    auto buffer = SharedBuffer();
    write(message, out(buffer));
    auto parsed = OtcLinkReplayRequestMessage::parse(
      std::string_view(buffer.get_data(), buffer.get_size()));
    REQUIRE(parsed.m_sender_comp_id == "SNAPPER");
    REQUIRE(parsed.m_appl_req_id == "SNAP2");
    REQUIRE(
      parsed.m_appl_req_type == OtcLinkReplayRequestMessage::Type::SNAPSHOT);
    REQUIRE(parsed.m_channel == OtcLinkChannelId::QUOTE_BOOK_SNAPSHOT);
    REQUIRE_FALSE(parsed.m_appl_beg_seq_no.has_value());
    REQUIRE_FALSE(parsed.m_appl_end_seq_no.has_value());
  }

  TEST_CASE("checksum_is_valid") {
    auto message = OtcLinkReplayRequestMessage::make_gap_fill_request(
      "TEST", "ID", OtcLinkChannelId::QUOTE_BOOK_REAL_TIME, 1, 10);
    auto buffer = SharedBuffer();
    write(message, out(buffer));
    auto result = std::string_view(buffer.get_data(), buffer.get_size());
    auto checksum_position = result.find("10=");
    REQUIRE(checksum_position != std::string_view::npos);
    auto body = result.substr(0, checksum_position);
    auto expected_checksum = 0;
    for(auto c : body) {
      expected_checksum += static_cast<unsigned char>(c);
    }
    expected_checksum = expected_checksum % 256;
    auto checksum_str = result.substr(checksum_position + 3, 3);
    auto actual_checksum = std::stoi(std::string(checksum_str));
    REQUIRE(actual_checksum == expected_checksum);
  }
}
