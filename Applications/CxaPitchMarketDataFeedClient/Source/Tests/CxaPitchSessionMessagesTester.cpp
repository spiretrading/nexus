#include <string_view>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Out.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchMessage.hpp"
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
    REQUIRE(response.m_status != CxaPitchLoginResponse::ACCEPTED);
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
}
