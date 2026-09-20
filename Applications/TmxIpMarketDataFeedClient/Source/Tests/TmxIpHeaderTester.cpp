#include <ostream>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpHeader.hpp"

using namespace Nexus;

TEST_SUITE("TmxIpHeader") {
  TEST_CASE("parse") {
    auto source = std::string_view("0042000000123CDF00  T ");
    auto header = TmxIpHeader::parse(source);
    REQUIRE(header.m_length == 42);
    REQUIRE(header.m_sequence == std::uint32_t(123));
    REQUIRE(header.m_service == "CDF");
    REQUIRE(header.m_service.data() >= source.data());
    REQUIRE(header.m_service.data() + header.m_service.size() <=
      source.data() + source.size());
    REQUIRE(header.m_retransmission == '0');
    REQUIRE(header.m_continuation == TmxIpHeader::Continuation::NONE);
    REQUIRE(header.m_type == ' ');
    REQUIRE(header.m_exchange == 'T');
    REQUIRE(!is_heartbeat(header));
    header = TmxIpHeader::parse("9999999999999CB113  Q payload");
    REQUIRE(header.m_length == 9999);
    REQUIRE(header.m_sequence == std::uint32_t(999999999));
    REQUIRE(header.m_service == "CB1");
    REQUIRE(header.m_retransmission == '1');
    REQUIRE(header.m_continuation == TmxIpHeader::Continuation::MIDDLE);
    REQUIRE(header.m_exchange == 'Q');
    header = TmxIpHeader::parse("0022000000001LS201  S ");
    REQUIRE(header.m_length == TmxIpHeader::LENGTH);
    REQUIRE(header.m_sequence == std::uint32_t(1));
    REQUIRE(header.m_service == "LS2");
    REQUIRE(header.m_continuation == TmxIpHeader::Continuation::FIRST);
    header = TmxIpHeader::parse("0022000000001BK202  B ");
    REQUIRE(header.m_continuation == TmxIpHeader::Continuation::LAST);
  }

  TEST_CASE("heartbeat") {
    for(auto source : {"0207         CDF 0V T ", "0207         CDF00V T "}) {
      auto header = TmxIpHeader::parse(source);
      REQUIRE(header.m_length == 207);
      REQUIRE(!header.m_sequence.has_value());
      REQUIRE(header.m_service == "CDF");
      REQUIRE(header.m_continuation == TmxIpHeader::Continuation::NONE);
      REQUIRE(header.m_type == 'V');
      REQUIRE(header.m_exchange == 'T');
      REQUIRE(is_heartbeat(header));
    }
  }

  TEST_CASE("malformed_header") {
    auto source = std::string_view("0042000000123CDF00  T ");
    for(auto size = std::size_t(0); size < TmxIpHeader::LENGTH; ++size) {
      REQUIRE_THROWS_AS(
        TmxIpHeader::parse(source.substr(0, size)), TmxIpParserException);
    }
    for(auto source : {
        "0021000000123CDF00  T ", "0000000000123CDF00  T ",
        " 042000000123CDF00  T ", "+042000000123CDF00  T ",
        "-042000000123CDF00  T ", "004X000000123CDF00  T ",
        "0042000000000CDF00  T ", "0042+00000123CDF00  T ",
        "0042-00000123CDF00  T ", "004200000012XCDF00  T ",
        "0042        1CDF00  T ", "004200000012 CDF00  T ",
        "0042         CDF00  T ", "0042000000123CDF00V T ",
        "0042000000123C F00  T ", "0042000000123CDF20  T ",
        "0042000000123CDF 0  T ", "0042000000123CDF04  T ",
        "0042000000123CDF0/  T ", "0042000000123CDF00X T ",
        "0042         CDF00VV T", "0042000000123CDF00  TT",
        "0042000000123CDF00    "}) {
      REQUIRE_THROWS_AS(TmxIpHeader::parse(source), TmxIpParserException);
    }
  }
}
