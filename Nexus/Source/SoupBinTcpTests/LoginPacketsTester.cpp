#include <Beam/IO/SharedBuffer.hpp>
#include <doctest/doctest.h>
#include "Nexus/SoupBinTcp/LoginPackets.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  auto make_login_accepted_packet(
      const std::string& session, std::uint64_t sequence_number) {
    auto buffer = SharedBuffer();
    for(auto i = session.size(); i < 10; ++i) {
      append(buffer, ' ');
    }
    append(buffer, session.c_str(), session.size());
    auto seq_str = std::to_string(sequence_number);
    for(auto i = seq_str.size(); i < 20; ++i) {
      append(buffer, ' ');
    }
    append(buffer, seq_str.c_str(), seq_str.size());
    return buffer;
  }

  auto make_login_rejected_packet(const std::string& reason) {
    auto buffer = SharedBuffer();
    if(reason.empty()) {
      append(buffer, ' ');
    } else {
      append(buffer, reason[0]);
    }
    return buffer;
  }
}

TEST_SUITE("LoginPackets") {
  TEST_CASE("parse_login_accepted_packet_basic") {
    auto session = "ABC";
    auto sequence_number = std::uint64_t(123456);
    auto buffer = make_login_accepted_packet(session, sequence_number);
    auto packet = SoupBinTcpPacket();
    packet.m_payload = static_cast<const char*>(buffer.get_data());
    auto result = parse_login_accepted_packet(packet);
    REQUIRE(result.m_session == session);
    REQUIRE(result.m_sequence_number == sequence_number);
  }

  TEST_CASE("parse_login_accepted_packet_all_spaces_session") {
    auto session = "";
    auto sequence_number = std::uint64_t(0);
    auto buffer = make_login_accepted_packet(session, sequence_number);
    auto packet = SoupBinTcpPacket();
    packet.m_payload = static_cast<const char*>(buffer.get_data());
    auto result = parse_login_accepted_packet(packet);
    REQUIRE(result.m_session == "");
    REQUIRE(result.m_sequence_number == 0);
  }

  TEST_CASE("parse_login_accepted_packet_max_values") {
    auto session = "ABCDEFGHIJ";
    auto sequence_number = std::numeric_limits<std::uint64_t>::max();
    auto buffer = make_login_accepted_packet(session, sequence_number);
    auto packet = SoupBinTcpPacket();
    packet.m_payload = static_cast<const char*>(buffer.get_data());
    auto result = parse_login_accepted_packet(packet);
    REQUIRE(result.m_session == session);
    REQUIRE(result.m_sequence_number == sequence_number);
  }

  TEST_CASE("parse_login_rejected_packet_basic") {
    auto reason = "X";
    auto buffer = make_login_rejected_packet(reason);
    auto packet = SoupBinTcpPacket();
    packet.m_payload = static_cast<const char*>(buffer.get_data());
    auto result = parse_login_rejected_packet(packet);
    REQUIRE(result.m_reason == "X");
  }

  TEST_CASE("parse_login_rejected_packet_blank") {
    auto reason = "";
    auto buffer = make_login_rejected_packet(reason);
    auto packet = SoupBinTcpPacket();
    packet.m_payload = static_cast<const char*>(buffer.get_data());
    auto result = parse_login_rejected_packet(packet);
    REQUIRE(result.m_reason == "");
  }
}
