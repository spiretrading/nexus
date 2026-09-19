#include <algorithm>
#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <doctest/doctest.h>
#include "Nexus/SoupBinTcp/SoupBinTcpPacket.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::endian;
using namespace Nexus;

namespace {
  struct PacketReader {
    BufferReader<SharedBuffer> m_reader;
    std::size_t m_limit;
    int m_reads = 0;

    bool poll() const {
      return m_reader.poll();
    }

    template<IsBuffer B>
    std::size_t read(Out<B> buffer, std::size_t size) {
      ++m_reads;
      return m_reader.read(buffer, std::min(size, m_limit));
    }
  };
}

TEST_SUITE("SoupBinTcpPacket") {
  TEST_CASE("alphanumeric_field") {
    auto buffer = SharedBuffer();
    append("ABC", 3, out(buffer));
    REQUIRE(buffer == "ABC");
  }

  TEST_CASE("padded_field") {
    auto buffer = SharedBuffer();
    append("A", 3, out(buffer));
    REQUIRE(buffer == "A  ");
  }

  TEST_CASE("oversized_field") {
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(
      append("TOOLONG", 3, out(buffer)), SoupBinTcpParserException);
  }

  TEST_CASE("read") {
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
    REQUIRE(packet.get_payload() == "BC");
    REQUIRE(packet.m_payload == payload.get_data() + sizeof(packet.m_type));
  }

  TEST_CASE("empty_payload") {
    auto buffer = SharedBuffer();
    auto length = native_to_big(std::uint16_t(1));
    append(buffer, length);
    append(buffer, 'Z');
    auto reader = BufferReader(buffer);
    auto payload = SharedBuffer();
    auto packet = read_packet(reader, out(payload));
    REQUIRE(packet.m_length == 1);
    REQUIRE(packet.m_type == 'Z');
    REQUIRE(packet.get_payload().empty());
    REQUIRE(packet.m_payload == payload.get_data() + sizeof(packet.m_type));
  }

  TEST_CASE("zero_length") {
    auto reader = BufferReader(SharedBuffer("\x00\x00S", 3));
    auto payload = SharedBuffer();
    REQUIRE_THROWS_AS(read_packet(reader, out(payload)),
      SoupBinTcpParserException);
  }

  TEST_CASE("truncated_frame") {
    auto frame = std::string_view("\x00\x03SAB", 5);
    for(auto size = std::size_t(0); size < frame.size(); ++size) {
      auto reader = BufferReader(SharedBuffer(frame.data(), size));
      auto buffer = SharedBuffer();
      REQUIRE_THROWS_AS(read_packet(reader, out(buffer)), EndOfFileException);
    }
  }

  TEST_CASE("framing") {
    auto limit = std::size_t(100);
    SUBCASE("coalesced") {}
    SUBCASE("fragmented") {
      limit = 1;
    }
    auto reader = PacketReader(
      BufferReader(SharedBuffer("\x00\x03SAB\x00\x01H", 8)), limit);
    auto buffer = SharedBuffer("prefix", 6);
    auto packet = read_packet(reader, out(buffer));
    REQUIRE(packet.m_type == 'S');
    REQUIRE(packet.get_payload() == "AB");
    if(limit != 1) {
      REQUIRE(reader.m_reads == 2);
    }
    packet = read_packet(reader, out(buffer));
    REQUIRE(packet.m_type == 'H');
    REQUIRE(packet.get_payload().empty());
    REQUIRE(buffer == "prefixSABH");
    REQUIRE_FALSE(reader.poll());
  }
}
