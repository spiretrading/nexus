#include <string>
#include <string_view>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchEncoder.hpp"

using namespace Nexus;

TEST_SUITE("CxaPitchEncoder") {
  TEST_CASE("write_integers") {
    auto buffer = Beam::SharedBuffer();
    auto encoder = CxaPitchEncoder(Beam::Ref(buffer));
    encoder.write_uint8(0x2A);
    encoder.write_uint16(50);
    encoder.write_uint32(4155);
    REQUIRE(std::string_view(buffer.get_data(), buffer.get_size()) ==
      std::string_view("\x2a" "\x32\x00" "\x3b\x10\x00\x00", 7));
  }

  TEST_CASE("write_text") {
    auto buffer = Beam::SharedBuffer();
    auto encoder = CxaPitchEncoder(Beam::Ref(buffer));
    encoder.write_text("FIRM", 4);
    encoder.write_text("AB", 4);
    encoder.write_text("", 2);
    encoder.write_text("ABCDEF", 4);
    encoder.pad(3);
    REQUIRE(std::string_view(buffer.get_data(), buffer.get_size()) ==
      "FIRMAB    ABCD   ");
  }
}
