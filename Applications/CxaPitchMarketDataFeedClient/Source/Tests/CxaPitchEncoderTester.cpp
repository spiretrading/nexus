#include <stdexcept>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/StaticBuffer.hpp>
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
    REQUIRE(
      buffer == std::string_view("\x2a" "\x32\x00" "\x3b\x10\x00\x00", 7));
  }

  TEST_CASE("write_text") {
    auto buffer = Beam::SharedBuffer();
    auto encoder = CxaPitchEncoder(Beam::Ref(buffer));
    encoder.write_text("FIRM", 4);
    encoder.write_text("AB", 4);
    encoder.write_text("", 2);
    encoder.write_text("ABCDEF", 4);
    encoder.pad(3);
    REQUIRE(buffer == "FIRMAB    ABCD   ");
  }

  TEST_CASE("shared_padding") {
    auto original = Beam::SharedBuffer("FIRMxxxx", 8);
    auto buffer = original;
    buffer.shrink(4);
    auto encoder = CxaPitchEncoder(Beam::Ref(buffer));
    encoder.pad(4);
    REQUIRE(buffer == "FIRM    ");
    REQUIRE(original == "FIRMxxxx");
  }

  TEST_CASE("padding_capacity") {
    auto buffer = Beam::StaticBuffer<8>("FIRMxxxx", 8);
    buffer.shrink(4);
    auto encoder = CxaPitchEncoder(Beam::Ref(buffer));
    REQUIRE_THROWS_AS(encoder.pad(5), std::out_of_range);
    REQUIRE(buffer == "FIRMxxxx");
  }
}
