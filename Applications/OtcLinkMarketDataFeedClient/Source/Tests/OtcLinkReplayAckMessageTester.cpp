#include <doctest/doctest.h>
#include <Beam/IO/SharedBuffer.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkReplayAckMessage.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("OtcLinkReplayAckMessage") {
  TEST_CASE("parse_success_ack") {
    auto source = std::string_view(
      "35=BX\x01"
      "59=TARGET\x01"
      "1346=REQ123\x01"
      "1348=0\x01"
      "1355=11\x01"
      "1182=100\x01"
      "1183=200\x01"
      "10=123\x01");
    auto message = OtcLinkReplayAckMessage::parse(source);
    REQUIRE(message.m_target_comp_id == "TARGET");
    REQUIRE(message.m_appl_req_id == "REQ123");
    REQUIRE(message.m_response_type ==
      OtcLinkReplayAckMessage::ResponseType::SUCCESS);
    REQUIRE(message.m_channel == OtcLinkChannelId::QUOTE_BOOK_REAL_TIME);
    REQUIRE(message.m_appl_beg_seq_no.value() == 100);
    REQUIRE(message.m_appl_end_seq_no.value() == 200);
    REQUIRE_FALSE(message.m_text.has_value());
  }

  TEST_CASE("parse_error_ack_with_text") {
    auto source = std::string_view(
      "35=BX\x01"
      "59=CLIENT\x01"
      "1346=REQ456\x01"
      "1348=1\x01"
      "58=Rate limit exceeded\x01"
      "1355=12\x01"
      "10=045\x01");
    auto message = OtcLinkReplayAckMessage::parse(source);
    REQUIRE(message.m_target_comp_id == "CLIENT");
    REQUIRE(message.m_appl_req_id == "REQ456");
    REQUIRE(message.m_response_type ==
      OtcLinkReplayAckMessage::ResponseType::LIMITS_EXCEEDED);
    REQUIRE(message.m_channel == OtcLinkChannelId::QUOTE_BOOK_SNAPSHOT);
    REQUIRE(message.m_text.value() == "Rate limit exceeded");
    REQUIRE_FALSE(message.m_appl_beg_seq_no.has_value());
    REQUIRE_FALSE(message.m_appl_end_seq_no.has_value());
  }

  TEST_CASE("parse_error_ack_without_text") {
    auto source = std::string_view(
      "35=BX\x01"
      "59=USER\x01"
      "1346=REQ789\x01"
      "1348=4\x01"
      "1355=14\x01"
      "10=000\x01");
    auto message = OtcLinkReplayAckMessage::parse(source);
    REQUIRE(message.m_target_comp_id == "USER");
    REQUIRE(message.m_appl_req_id == "REQ789");
    REQUIRE(message.m_response_type ==
      OtcLinkReplayAckMessage::ResponseType::BAD_REQUEST);
    REQUIRE(message.m_channel == OtcLinkChannelId::QUOTE_INSIDE_REAL_TIME);
    REQUIRE_FALSE(message.m_text.has_value());
  }

  TEST_CASE("make_success") {
    auto message = OtcLinkReplayAckMessage::make_success(
      "TARGET", "REQ1", OtcLinkChannelId::QUOTE_BOOK_REAL_TIME, 100, 200);
    REQUIRE(message.m_target_comp_id == "TARGET");
    REQUIRE(message.m_appl_req_id == "REQ1");
    REQUIRE(message.m_response_type ==
      OtcLinkReplayAckMessage::ResponseType::SUCCESS);
    REQUIRE(message.m_channel == OtcLinkChannelId::QUOTE_BOOK_REAL_TIME);
    REQUIRE(message.m_appl_beg_seq_no.value() == 100);
    REQUIRE(message.m_appl_end_seq_no.value() == 200);
    REQUIRE_FALSE(message.m_text.has_value());
  }

  TEST_CASE("make_error_with_text") {
    auto message = OtcLinkReplayAckMessage::make_error(
      "CLIENT", "REQ2", OtcLinkChannelId::TRADE_REAL_TIME,
      OtcLinkReplayAckMessage::ResponseType::NOT_ENTITLED,
      std::string("Not authorized"));
    REQUIRE(message.m_target_comp_id == "CLIENT");
    REQUIRE(message.m_appl_req_id == "REQ2");
    REQUIRE(message.m_response_type ==
      OtcLinkReplayAckMessage::ResponseType::NOT_ENTITLED);
    REQUIRE(message.m_channel == OtcLinkChannelId::TRADE_REAL_TIME);
    REQUIRE(message.m_text.value() == "Not authorized");
    REQUIRE_FALSE(message.m_appl_beg_seq_no.has_value());
    REQUIRE_FALSE(message.m_appl_end_seq_no.has_value());
  }

  TEST_CASE("make_error_without_text") {
    auto message = OtcLinkReplayAckMessage::make_error(
      "USER", "REQ3", OtcLinkChannelId::QUOTE_INSIDE_SNAPSHOT,
      OtcLinkReplayAckMessage::ResponseType::MESSAGES_NOT_AVAILABLE);
    REQUIRE(message.m_target_comp_id == "USER");
    REQUIRE(message.m_appl_req_id == "REQ3");
    REQUIRE(message.m_response_type ==
      OtcLinkReplayAckMessage::ResponseType::MESSAGES_NOT_AVAILABLE);
    REQUIRE(message.m_channel == OtcLinkChannelId::QUOTE_INSIDE_SNAPSHOT);
    REQUIRE_FALSE(message.m_text.has_value());
  }

  TEST_CASE("write_success_ack") {
    auto message = OtcLinkReplayAckMessage::make_success(
      "WRITER", "WRITE1", OtcLinkChannelId::TRADE_REAL_TIME, 500, 600);
    auto buffer = SharedBuffer();
    write(message, out(buffer));
    auto parsed = OtcLinkReplayAckMessage::parse(
      std::string_view(buffer.get_data(), buffer.get_size()));
    REQUIRE(parsed.m_target_comp_id == "WRITER");
    REQUIRE(parsed.m_appl_req_id == "WRITE1");
    REQUIRE(parsed.m_response_type ==
      OtcLinkReplayAckMessage::ResponseType::SUCCESS);
    REQUIRE(parsed.m_channel == OtcLinkChannelId::TRADE_REAL_TIME);
    REQUIRE(parsed.m_appl_beg_seq_no.value() == 500);
    REQUIRE(parsed.m_appl_end_seq_no.value() == 600);
  }

  TEST_CASE("write_error_ack_with_text") {
    auto message = OtcLinkReplayAckMessage::make_error(
      "ERRUSER", "ERR1", OtcLinkChannelId::QUOTE_BOOK_SNAPSHOT,
      OtcLinkReplayAckMessage::ResponseType::BAD_REQUEST,
      std::string("Invalid sequence range"));
    auto buffer = SharedBuffer();
    write(message, out(buffer));
    auto parsed = OtcLinkReplayAckMessage::parse(
      std::string_view(buffer.get_data(), buffer.get_size()));
    REQUIRE(parsed.m_target_comp_id == "ERRUSER");
    REQUIRE(parsed.m_appl_req_id == "ERR1");
    REQUIRE(parsed.m_response_type ==
      OtcLinkReplayAckMessage::ResponseType::BAD_REQUEST);
    REQUIRE(parsed.m_channel == OtcLinkChannelId::QUOTE_BOOK_SNAPSHOT);
    REQUIRE(parsed.m_text.value() == "Invalid sequence range");
  }

  TEST_CASE("write_error_ack_without_text") {
    auto message = OtcLinkReplayAckMessage::make_error(
      "NOTEXT", "ERR2", OtcLinkChannelId::QUOTE_INSIDE_REAL_TIME,
      OtcLinkReplayAckMessage::ResponseType::LIMITS_EXCEEDED);
    auto buffer = SharedBuffer();
    write(message, out(buffer));
    auto parsed = OtcLinkReplayAckMessage::parse(
      std::string_view(buffer.get_data(), buffer.get_size()));
    REQUIRE(parsed.m_target_comp_id == "NOTEXT");
    REQUIRE(parsed.m_appl_req_id == "ERR2");
    REQUIRE(parsed.m_response_type ==
      OtcLinkReplayAckMessage::ResponseType::LIMITS_EXCEEDED);
    REQUIRE(parsed.m_channel == OtcLinkChannelId::QUOTE_INSIDE_REAL_TIME);
    REQUIRE_FALSE(parsed.m_text.has_value());
  }

  TEST_CASE("checksum_is_valid") {
    auto message = OtcLinkReplayAckMessage::make_success(
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
