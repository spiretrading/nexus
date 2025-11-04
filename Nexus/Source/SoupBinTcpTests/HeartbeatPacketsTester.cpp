#include <Beam/IO/SharedBuffer.hpp>
#include <doctest/doctest.h>
#include "Nexus/SoupBinTcp/HeartbeatPackets.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("HeartbeatPackets") {
  TEST_CASE("make_client_heartbeat_packet_contents") {
    auto buffer = SharedBuffer();
    make_client_heartbeat_packet(Store(buffer));
    REQUIRE(buffer.GetSize() == 3);
    auto length = std::uint16_t();
    std::memcpy(&length, buffer.GetData(), sizeof(length));
    length = Beam::FromBigEndian(length);
    REQUIRE(length == 1);
    REQUIRE(static_cast<const char*>(buffer.GetData())[2] == 'R');
  }

  TEST_CASE("make_client_heartbeat_packet_multiple_calls") {
    auto buffer = SharedBuffer();
    make_client_heartbeat_packet(Store(buffer));
    make_client_heartbeat_packet(Store(buffer));
    REQUIRE(buffer.GetSize() == 6);
    auto length1 = std::uint16_t();
    std::memcpy(&length1, buffer.GetData(), sizeof(length1));
    length1 = Beam::FromBigEndian(length1);
    REQUIRE(length1 == 1);
    REQUIRE(static_cast<const char*>(buffer.GetData())[2] == 'R');
    auto length2 = std::uint16_t();
    std::memcpy(&length2,
      static_cast<const char*>(buffer.GetData()) + 3, sizeof(length2));
    length2 = Beam::FromBigEndian(length2);
    REQUIRE(length2 == 1);
    REQUIRE(static_cast<const char*>(buffer.GetData())[5] == 'R');
  }
}
