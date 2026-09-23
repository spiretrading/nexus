#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkRecoveryMessages.hpp"

using namespace Nexus;

namespace {
  std::string checksum(std::string body) {
    auto sum = 0U;
    for(auto byte : body) {
      sum += static_cast<unsigned char>(byte);
    }
    return body + std::format("10={:03}\x01", sum % 256);
  }
}

TEST_SUITE("OtcLinkRecoveryMessages") {
  TEST_CASE("response") {
    auto body = std::string("35=BX\x01" "59=SPIRE\x01" "1346=7\x01"
      "1355=11\x01" "1348=0\x01" "1182=100\x01" "1183=101\x01");
    SUBCASE("accepted") {
      auto response = OtcLinkRecoveryResponse::parse(checksum(body));
      REQUIRE(response.m_recipient == "SPIRE");
      REQUIRE(response.m_id == 7);
      REQUIRE(response.m_channel == 11);
      REQUIRE(response.m_status == OtcLinkRecoveryResponse::Status::ACCEPTED);
      REQUIRE(response.m_first_sequence == 100U);
      REQUIRE(response.m_last_sequence == 101U);
    }
    SUBCASE("rejected") {
      for(auto status = 1; status <= 4; ++status) {
        auto response = OtcLinkRecoveryResponse::parse(checksum(std::format(
          "35=BX\x01" "59=SPIRE\x01" "1346=7\x01" "1355=11\x01"
          "1348={}\x01" "58=Unavailable\x01", status)));
        REQUIRE(static_cast<int>(response.m_status) == status);
        REQUIRE(response.m_text == "Unavailable");
        REQUIRE(!response.m_first_sequence);
        REQUIRE(!response.m_last_sequence);
      }
    }
    SUBCASE("checksum") {
      auto source = checksum(body);
      source[0] = '4';
      REQUIRE_THROWS_AS(
        OtcLinkRecoveryResponse::parse(source), OtcLinkParserException);
      REQUIRE_THROWS_AS(
        OtcLinkRecoveryResponse::parse(body), OtcLinkParserException);
      REQUIRE_THROWS_AS(OtcLinkRecoveryResponse::parse(checksum(body) + "x"),
        OtcLinkParserException);
    }
    SUBCASE("fields") {
      for(auto field : {"1346=8\x01", "1355=12\x01", "1182=102\x01"}) {
        REQUIRE_THROWS_AS(
          OtcLinkRecoveryResponse::parse(checksum(body + field)),
          OtcLinkParserException);
      }
      for(auto fields : {"1348=0\x01", "1348=5\x01", "1348=-1\x01",
          "1348=0\x01" "1182=100\x01" "1183=99\x01",
          "1348=0\x01" "1182=100\x01" "1183=4294967296\x01"}) {
        auto source = checksum(std::string("35=BX\x01" "59=SPIRE\x01"
          "1346=7\x01" "1355=11\x01") + fields);
        REQUIRE_THROWS_AS(
          OtcLinkRecoveryResponse::parse(source), OtcLinkParserException);
      }
    }
  }

  TEST_CASE("request") {
    auto request = OtcLinkRecoveryRequest("SPIRE", 7, 11, 100, 2000);
    REQUIRE(request.encode() == "35=BW\x01" "49=SPIRE\x01" "1346=7\x01"
      "1347=0\x01" "1355=11\x01" "1182=100\x01" "1183=2099\x01"
      "10=213\x01");
    SUBCASE("count") {
      for(auto count : {0U, 2001U}) {
        request.m_count = count;
        REQUIRE_THROWS_AS(request.encode(), OtcLinkParserException);
      }
    }
    SUBCASE("overflow") {
      request.m_sequence = std::numeric_limits<std::uint32_t>::max();
      REQUIRE_THROWS_AS(request.encode(), OtcLinkParserException);
      request.m_count = 1;
      REQUIRE_NOTHROW(request.encode());
    }
    SUBCASE("sender") {
      request.m_sender = "";
      REQUIRE_THROWS_AS(request.encode(), OtcLinkParserException);
      request.m_sender = "A\x01" "49=B";
      REQUIRE_THROWS_AS(request.encode(), OtcLinkParserException);
    }
  }
}
