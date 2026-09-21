#include <string>
#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpHeartbeat.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  constexpr auto HEARTBEAT = "\x02" "0207         CDF00V T "
    "[HEARTBEAT 2012-10-10 03:25:02-001349853902.844623]"
    "[LAST SENT 000001345-03:05:03-001349852703.441869]"
    "[LAST HB   000001345-03:24:02-001349853842.845443]"
    "OCSA-CDF-1           ATDOTDR  00.1\x03";
}

TEST_SUITE("TmxIpHeartbeat") {
  TEST_CASE("parse") {
    auto packet = TmxIpPacket::parse(HEARTBEAT);
    auto heartbeat = TmxIpHeartbeat::parse(packet);
    REQUIRE(heartbeat.m_timestamp ==
      time_from_string("2012-10-10 07:25:02.844623"));
    REQUIRE(heartbeat.m_last_sequence == 1345);
    REQUIRE(heartbeat.m_last_timestamp ==
      time_from_string("2012-10-10 07:05:03.441869"));
    REQUIRE(heartbeat.m_previous_sequence == 1345);
    REQUIRE(heartbeat.m_previous_timestamp ==
      time_from_string("2012-10-10 07:24:02.845443"));
    REQUIRE(heartbeat.m_host == "TDOTDR");
    REQUIRE(heartbeat.m_version == "00.1");
    REQUIRE(heartbeat.m_host.data() >= packet.m_payload.data());
    REQUIRE(heartbeat.m_host.data() <
      packet.m_payload.data() + packet.m_payload.size());
  }

  TEST_CASE("malformed_payload") {
    auto packet = TmxIpPacket::parse(HEARTBEAT);
    auto payload = std::string(packet.m_payload);
    SUBCASE("truncated") {
      for(auto size = std::size_t(0); size < payload.size(); ++size) {
        packet.m_payload = std::string_view(payload).substr(0, size);
        REQUIRE_THROWS_AS(TmxIpHeartbeat::parse(packet), TmxIpParserException);
      }
      payload.pop_back();
    }
    SUBCASE("trailing_data") {
      payload += 'X';
    }
    SUBCASE("section") {
      payload[payload.find("LAST SENT")] = 'X';
    }
    SUBCASE("sequence") {
      payload[payload.find("000001345")] = '-';
    }
    SUBCASE("sequence_suffix") {
      payload.replace(payload.find("000001345"), sizeof("000001345") - 1,
        "00000134X");
    }
    SUBCASE("previous_sequence") {
      payload[payload.rfind("000001345")] = ' ';
    }
    SUBCASE("epoch") {
      payload[payload.find("001349853902")] = 'X';
    }
    SUBCASE("epoch_overflow") {
      payload.replace(payload.find("001349853902"), sizeof("001349853902") - 1,
        "999999999999");
    }
    SUBCASE("fraction") {
      payload[payload.find("844623")] = ' ';
    }
    SUBCASE("decimal_point") {
      payload[payload.find('.')] = ',';
    }
    SUBCASE("date") {
      payload.replace(
        payload.find("2012-10-10"), sizeof("2012-10-10") - 1, "2012-02-30");
    }
    SUBCASE("time") {
      payload.replace(
        payload.find("03:25:02"), sizeof("03:25:02") - 1, "24:25:02");
    }
    packet.m_payload = payload;
    REQUIRE_THROWS_AS(TmxIpHeartbeat::parse(packet), TmxIpParserException);
  }

  TEST_CASE("packet_type") {
    auto packet = TmxIpPacket::parse(HEARTBEAT);
    SUBCASE("business_message") {
      packet.m_header.m_type = ' ';
    }
    SUBCASE("fragment") {
      packet.m_header.m_continuation = TmxIpHeader::Continuation::FIRST;
    }
    REQUIRE_THROWS_AS(TmxIpHeartbeat::parse(packet), TmxIpParserException);
  }
}
