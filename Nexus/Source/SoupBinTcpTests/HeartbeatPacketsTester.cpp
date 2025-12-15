#include <Beam/IO/SharedBuffer.hpp>
#include <doctest/doctest.h>
#include "Nexus/SoupBinTcp/HeartbeatPackets.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::endian;
using namespace Nexus;

TEST_SUITE("HeartbeatPackets") {
  TEST_CASE("make_client_heartbeat_packet_contents") {
    auto buffer = SharedBuffer();
    make_client_heartbeat_packet(out(buffer));
    REQUIRE(buffer.get_size() == 3);
    auto length = std::uint16_t();
    std::memcpy(&length, buffer.get_data(), sizeof(length));
    length = big_to_native(length);
    REQUIRE(length == 1);
    REQUIRE(static_cast<const char*>(buffer.get_data())[2] == 'R');
  }

  TEST_CASE("make_client_heartbeat_packet_multiple_calls") {
    auto buffer = SharedBuffer();
    make_client_heartbeat_packet(out(buffer));
    make_client_heartbeat_packet(out(buffer));
    REQUIRE(buffer.get_size() == 6);
    auto length1 = std::uint16_t();
    std::memcpy(&length1, buffer.get_data(), sizeof(length1));
    length1 = big_to_native(length1);
    REQUIRE(length1 == 1);
    REQUIRE(static_cast<const char*>(buffer.get_data())[2] == 'R');
    auto length2 = std::uint16_t();
    std::memcpy(&length2,
      static_cast<const char*>(buffer.get_data()) + 3, sizeof(length2));
    length2 = big_to_native(length2);
    REQUIRE(length2 == 1);
    REQUIRE(static_cast<const char*>(buffer.get_data())[5] == 'R');
  }
}
