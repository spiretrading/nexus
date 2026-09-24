#include <limits>
#include <Beam/IO/SharedBuffer.hpp>
#include <doctest/doctest.h>
#include "Nexus/MoldUdp64/MoldUdp64Request.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("MoldUdp64Request") {
  TEST_CASE("encode") {
    auto request = MoldUdp64Request("SESSION123", 0x0102030405060708, 0x0910);
    auto expected = std::string_view("SESSION123"
      "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x10", 20);
    SUBCASE("fields") {}
    SUBCASE("maximum_fields") {
      request.m_sequence_number = std::numeric_limits<std::uint64_t>::max();
      request.m_count = std::numeric_limits<std::uint16_t>::max();
      expected = std::string_view("SESSION123"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 20);
    }
    auto buffer = SharedBuffer("prefix", 6);
    encode(request, out(buffer));
    REQUIRE(buffer.get_size() == 6 + MoldUdp64Request::LENGTH);
    REQUIRE(std::string_view(buffer.get_data(), 6) == "prefix");
    REQUIRE(std::string_view(
      buffer.get_data() + 6, MoldUdp64Request::LENGTH) == expected);
  }
}
