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
    auto sequence = std::to_string(sequence_number);
    for(auto i = sequence.size(); i < 20; ++i) {
      append(buffer, ' ');
    }
    append(buffer, sequence.c_str(), sequence.size());
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
  TEST_CASE("login_request") {
    auto session = std::string();
    auto sequence = std::uint64_t(0);
    auto expected_session = std::string(10, ' ');
    auto expected_sequence = "0                   ";
    SUBCASE("current") {}
    SUBCASE("resume") {
      session = "SESSION";
      sequence = 123;
      expected_session = "SESSION   ";
      expected_sequence = "123                 ";
    }
    auto buffer = SharedBuffer();
    make_login_request_packet("user", "pass", session, sequence, out(buffer));
    auto expected = std::string("\x00\x2fLuser  pass      ", 19) +
      expected_session + expected_sequence;
    REQUIRE(buffer.get_size() == expected.size());
    REQUIRE(std::string_view(buffer.get_data(), buffer.get_size()) == expected);
  }

  TEST_CASE("accepted") {
    auto session = "ABC";
    auto sequence_number = std::uint64_t(123456);
    auto buffer = make_login_accepted_packet(session, sequence_number);
    auto packet = SoupBinTcpPacket(31, 'A', buffer.get_data());
    auto result = parse_login_accepted_packet(packet);
    REQUIRE(result.m_session == session);
    REQUIRE(result.m_sequence_number == sequence_number);
  }

  TEST_CASE("blank_session") {
    auto session = "";
    auto sequence_number = std::uint64_t(0);
    auto buffer = make_login_accepted_packet(session, sequence_number);
    auto packet = SoupBinTcpPacket(31, 'A', buffer.get_data());
    auto result = parse_login_accepted_packet(packet);
    REQUIRE(result.m_session == "");
    REQUIRE(result.m_sequence_number == 0);
  }

  TEST_CASE("maximum_fields") {
    auto session = "ABCDEFGHIJ";
    auto sequence_number = std::numeric_limits<std::uint64_t>::max();
    auto buffer = make_login_accepted_packet(session, sequence_number);
    auto packet = SoupBinTcpPacket(31, 'A', buffer.get_data());
    auto result = parse_login_accepted_packet(packet);
    REQUIRE(result.m_session == session);
    REQUIRE(result.m_sequence_number == sequence_number);
  }

  TEST_CASE("rejected") {
    auto reason = "X";
    auto buffer = make_login_rejected_packet(reason);
    auto packet = SoupBinTcpPacket(2, 'J', buffer.get_data());
    auto result = parse_login_rejected_packet(packet);
    REQUIRE(result.m_reason == 'X');
  }

  TEST_CASE("blank_reason") {
    auto reason = "";
    auto buffer = make_login_rejected_packet(reason);
    auto packet = SoupBinTcpPacket(2, 'J', buffer.get_data());
    auto result = parse_login_rejected_packet(packet);
    REQUIRE(result.m_reason == ' ');
  }

  TEST_CASE("invalid_packet") {
    SUBCASE("accepted") {
      auto buffer = make_login_accepted_packet("SESSION123", 1);
      auto packet = SoupBinTcpPacket(31, 'A', buffer.get_data());
      SUBCASE("length") {
        for(auto length = 0; length < 31; ++length) {
          packet.m_length = static_cast<std::uint16_t>(length);
          REQUIRE_THROWS_AS(parse_login_accepted_packet(packet),
            SoupBinTcpParserException);
        }
      }
      SUBCASE("type") {
        packet.m_type = 'S';
        REQUIRE_THROWS_AS(parse_login_accepted_packet(packet),
          SoupBinTcpParserException);
      }
    }
    SUBCASE("rejected") {
      auto buffer = make_login_rejected_packet("A");
      auto packet = SoupBinTcpPacket(2, 'J', buffer.get_data());
      SUBCASE("length") {
        packet.m_length = 1;
      }
      SUBCASE("type") {
        packet.m_type = 'S';
      }
      REQUIRE_THROWS_AS(parse_login_rejected_packet(packet),
        SoupBinTcpParserException);
    }
  }
}
