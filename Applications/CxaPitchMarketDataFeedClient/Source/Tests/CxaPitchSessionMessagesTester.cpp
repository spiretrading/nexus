#include <string_view>
#include <Beam/IO/SharedBuffer.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSessionMessages.hpp"

using namespace Nexus;

TEST_SUITE("CxaPitchSessionMessages") {
  TEST_CASE("session_message_concept") {
    REQUIRE(IsCxaPitchSessionMessage<CxaPitchLogin>);
    REQUIRE(IsCxaPitchSessionMessage<CxaPitchLoginResponse>);
    REQUIRE(IsCxaPitchSessionMessage<CxaPitchGapRequest>);
    REQUIRE(IsCxaPitchSessionMessage<CxaPitchGapResponse>);
    REQUIRE(IsCxaPitchSessionMessage<CxaPitchSpinImageAvailable>);
    REQUIRE(IsCxaPitchSessionMessage<CxaPitchSpinRequest>);
    REQUIRE(IsCxaPitchSessionMessage<CxaPitchSpinResponse>);
    REQUIRE(IsCxaPitchSessionMessage<CxaPitchSpinFinished>);
    REQUIRE(IsCxaPitchSessionMessage<const CxaPitchLogin&>);
    REQUIRE(IsCxaPitchSessionMessage<CxaPitchGapRequest&&>);
    REQUIRE(!IsCxaPitchSessionMessage<CxaPitchMessage>);
    REQUIRE(!IsCxaPitchSessionMessage<int>);
    struct DerivedLogin : CxaPitchLogin {};
    REQUIRE(!IsCxaPitchSessionMessage<DerivedLogin>);
  }

  TEST_CASE("encode_login") {
    auto login = CxaPitchLogin();
    login.m_session_sub_id = "0001";
    login.m_username = "FIRM";
    login.m_password = "ABCD00";
    auto buffer = Beam::SharedBuffer();
    login.encode(Beam::out(buffer));
    REQUIRE(buffer.get_size() == CxaPitchLogin::LENGTH);
    REQUIRE(std::string_view(buffer.get_data(), buffer.get_size()) ==
      std::string_view(
        "\x16\x01"
        "0001"
        "FIRM"
        "  "
        "ABCD00    ", 22));
  }

  TEST_CASE("parse_login_response") {
    auto source = std::string_view("\x03\x02" "A", 3);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchLoginResponse::TYPE);
    auto response = CxaPitchLoginResponse::parse(message);
    REQUIRE(response.m_status == CxaPitchLoginResponse::ACCEPTED);
  }

  TEST_CASE("parse_rejected_login_response") {
    auto source = std::string_view("\x03\x02" "N", 3);
    auto response =
      CxaPitchLoginResponse::parse(CxaPitchMessage::parse(source));
    REQUIRE(response.m_status == 'N');
  }

  TEST_CASE("encode_gap_request") {
    auto request = CxaPitchGapRequest();
    request.m_unit = 1;
    request.m_sequence = 4155;
    request.m_count = 50;
    auto buffer = Beam::SharedBuffer();
    request.encode(Beam::out(buffer));
    REQUIRE(buffer.get_size() == CxaPitchGapRequest::LENGTH);
    REQUIRE(std::string_view(buffer.get_data(), buffer.get_size()) ==
      std::string_view(
        "\x09\x03"
        "\x01"
        "\x3b\x10\x00\x00"
        "\x32\x00", 9));
  }

  TEST_CASE("parse_gap_response") {
    auto source = std::string_view(
      "\x0a\x04"
      "\x01"
      "\x3b\x10\x00\x00"
      "\x32\x00"
      "A", 10);
    auto message = CxaPitchMessage::parse(source);
    REQUIRE(message.m_type == CxaPitchGapResponse::TYPE);
    auto response = CxaPitchGapResponse::parse(message);
    REQUIRE(response.m_unit == 1);
    REQUIRE(response.m_sequence == 4155);
    REQUIRE(response.m_count == 50);
    REQUIRE(response.m_status == CxaPitchGapResponse::ACCEPTED);
  }

  TEST_CASE("parse_gap_response_too_short") {
    auto source = std::string_view(
      "\x09\x04"
      "\x01"
      "\x3b\x10\x00\x00"
      "\x32\x00", 9);
    REQUIRE_THROWS_AS(CxaPitchGapResponse::parse(
      CxaPitchMessage::parse(source)), CxaPitchParserException);
  }

  TEST_CASE("encode_session_messages") {
    auto buffer = Beam::SharedBuffer("prefix", 6);
    auto expected = std::string_view();
    SUBCASE("login_response") {
      auto response = CxaPitchLoginResponse('N');
      response.encode(Beam::out(buffer));
      expected = std::string_view("prefix\x03\x02" "N", 9);
    }
    SUBCASE("gap_response") {
      auto response = CxaPitchGapResponse(2, 0x12345678, 0x0123, 'O');
      response.encode(Beam::out(buffer));
      expected =
        std::string_view("prefix\x0a\x04\x02\x78\x56\x34\x12\x23\x01" "O", 16);
    }
    SUBCASE("spin_image_available") {
      auto available = CxaPitchSpinImageAvailable(0x12345678);
      available.encode(Beam::out(buffer));
      expected = std::string_view("prefix\x06\x80\x78\x56\x34\x12", 12);
    }
    SUBCASE("spin_response") {
      auto response = CxaPitchSpinResponse(0x12345678, 0x01234567, 'A');
      response.encode(Beam::out(buffer));
      expected = std::string_view(
        "prefix\x0b\x82\x78\x56\x34\x12\x67\x45\x23\x01" "A", 17);
    }
    SUBCASE("spin_finished") {
      auto finished = CxaPitchSpinFinished(0x12345678);
      finished.encode(Beam::out(buffer));
      expected = std::string_view("prefix\x06\x83\x78\x56\x34\x12", 12);
    }
    REQUIRE(std::string_view(buffer.get_data(), buffer.get_size()) == expected);
  }

  TEST_CASE("parse_short_session_message") {
    SUBCASE("login_response") {
      auto message = CxaPitchMessage::parse(std::string_view("\x02\x02", 2));
      REQUIRE_THROWS_AS(
        CxaPitchLoginResponse::parse(message), CxaPitchParserException);
    }
    SUBCASE("spin_image_available") {
      auto message =
        CxaPitchMessage::parse(std::string_view("\x05\x80\x78\x56\x34", 5));
      REQUIRE_THROWS_AS(
        CxaPitchSpinImageAvailable::parse(message), CxaPitchParserException);
    }
    SUBCASE("spin_response") {
      auto message = CxaPitchMessage::parse(
        std::string_view("\x0a\x82\x78\x56\x34\x12\x67\x45\x23\x01", 10));
      REQUIRE_THROWS_AS(
        CxaPitchSpinResponse::parse(message), CxaPitchParserException);
    }
  }

  TEST_CASE("parse_extended_session_message") {
    SUBCASE("login_response") {
      auto message =
        CxaPitchMessage::parse(std::string_view("\x04\x02" "N\xff", 4));
      auto response = CxaPitchLoginResponse::parse(message);
      REQUIRE(response.m_status == 'N');
    }
    SUBCASE("gap_response") {
      auto message = CxaPitchMessage::parse(
        std::string_view("\x0b\x04\x02\x78\x56\x34\x12\x23\x01" "O\xff", 11));
      auto response = CxaPitchGapResponse::parse(message);
      REQUIRE(response.m_unit == 2);
      REQUIRE(response.m_sequence == 0x12345678);
      REQUIRE(response.m_count == 0x0123);
      REQUIRE(response.m_status == 'O');
    }
    SUBCASE("spin_image_available") {
      auto message = CxaPitchMessage::parse(
        std::string_view("\x07\x80\x78\x56\x34\x12\xff", 7));
      auto available = CxaPitchSpinImageAvailable::parse(message);
      REQUIRE(available.m_sequence == 0x12345678);
    }
    SUBCASE("spin_response") {
      auto message = CxaPitchMessage::parse(std::string_view(
        "\x0c\x82\x78\x56\x34\x12\x67\x45\x23\x01" "A\xff", 12));
      auto response = CxaPitchSpinResponse::parse(message);
      REQUIRE(response.m_sequence == 0x12345678);
      REQUIRE(response.m_order_count == 0x01234567);
      REQUIRE(response.m_status == 'A');
    }
    SUBCASE("spin_finished") {
      auto message = CxaPitchMessage::parse(
        std::string_view("\x07\x83\x78\x56\x34\x12\xff", 7));
      auto finished = CxaPitchSpinFinished::parse(message);
      REQUIRE(finished.m_sequence == 0x12345678);
    }
  }
}
