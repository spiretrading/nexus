#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/BufferReader.hpp>
#include <doctest/doctest.h>
#include "Nexus/SoupBinTcp/SoupBinTcpPacket.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Nexus;

TEST_SUITE("SoupBinTcpPacket") {
  TEST_CASE("append_alphanumeric_exact_length") {
    auto buffer = SharedBuffer();
    append("ABC", 3, Store(buffer));
    REQUIRE(buffer.GetSize() == 3);
    REQUIRE(std::memcmp(buffer.GetData(), "ABC", 3) == 0);
  }

  TEST_CASE("append_alphanumeric_shorter_than_length") {
    auto buffer = SharedBuffer();
    append("A", 3, Store(buffer));
    REQUIRE(buffer.GetSize() == 3);
    REQUIRE(std::memcmp(buffer.GetData(), "A  ", 3) == 0);
  }

  TEST_CASE("append_alphanumeric_too_long") {
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(
      append("TOOLONG", 3, Store(buffer)), SoupBinTcpParserException);
  }

  TEST_CASE("read_packet_basic") {
    auto buffer = SharedBuffer();
    auto length = ToBigEndian(std::uint16_t(3));
    buffer.Append(length);
    buffer.Append('A');
    buffer.Append("BC", 2);
    auto reader = BufferReader(buffer);
    auto payload = SharedBuffer();
    auto packet = read_packet(reader, Store(payload));
    REQUIRE(packet.m_length == 3);
    REQUIRE(packet.m_type == 'A');
    REQUIRE(payload.GetSize() == 2);
    REQUIRE(std::memcmp(payload.GetData(), "BC", 2) == 0);
    REQUIRE(packet.m_payload == payload.GetData());
  }

  TEST_CASE("read_packet_empty_payload") {
    auto buffer = SharedBuffer();
    auto length = ToBigEndian(std::uint16_t(1));
    buffer.Append(length);
    buffer.Append('Z');
    auto reader = BufferReader(buffer);
    auto payload = SharedBuffer();
    auto packet = read_packet(reader, Store(payload));
    REQUIRE(packet.m_length == 1);
    REQUIRE(packet.m_type == 'Z');
    REQUIRE(payload.GetSize() == 0);
    REQUIRE(packet.m_payload == payload.GetData());
  }
}
