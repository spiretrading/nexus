#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <doctest/doctest.h>
#include "Nexus/SoupBinTcp/SoupBinTcpPacket.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::endian;
using namespace Nexus;

TEST_SUITE("SoupBinTcpPacket") {
  TEST_CASE("append_alphanumeric_exact_length") {
    auto buffer = SharedBuffer();
    append("ABC", 3, out(buffer));
    REQUIRE(buffer == "ABC");
  }

  TEST_CASE("append_alphanumeric_shorter_than_length") {
    auto buffer = SharedBuffer();
    append("A", 3, out(buffer));
    REQUIRE(buffer == "A  ");
  }

  TEST_CASE("append_alphanumeric_too_long") {
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(
      append("TOOLONG", 3, out(buffer)), SoupBinTcpParserException);
  }

  TEST_CASE("read_packet_basic") {
    auto buffer = SharedBuffer();
    auto length = native_to_big(std::uint16_t(3));
    append(buffer, length);
    append(buffer, 'A');
    append(buffer, "BC", 2);
    auto reader = BufferReader(buffer);
    auto payload = SharedBuffer();
    auto packet = read_packet(reader, out(payload));
    REQUIRE(packet.m_length == 3);
    REQUIRE(packet.m_type == 'A');
    REQUIRE(payload == "BC");
    REQUIRE(packet.m_payload == payload.get_data());
  }

  TEST_CASE("read_packet_empty_payload") {
    auto buffer = SharedBuffer();
    auto length = native_to_big(std::uint16_t(1));
    append(buffer, length);
    append(buffer, 'Z');
    auto reader = BufferReader(buffer);
    auto payload = SharedBuffer();
    auto packet = read_packet(reader, out(payload));
    REQUIRE(packet.m_length == 1);
    REQUIRE(packet.m_type == 'Z');
    REQUIRE(payload.get_size() == 0);
    REQUIRE(packet.m_payload == payload.get_data());
  }
}
