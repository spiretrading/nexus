#include <array>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/StaticBuffer.hpp>
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

  TEST_CASE("nonpositive_widths") {
    auto buffer = Beam::SharedBuffer("FIRM", 4);
    auto encoder = CxaPitchEncoder(Beam::Ref(buffer));
    for(auto size : std::array{0, -1, std::numeric_limits<int>::min()}) {
      encoder.pad(size);
      REQUIRE(std::string_view(buffer.get_data(), buffer.get_size()) == "FIRM");
      encoder.write_text("AB", size);
      REQUIRE(std::string_view(buffer.get_data(), buffer.get_size()) == "FIRM");
    }
  }

  TEST_CASE("shared_padding") {
    auto original = Beam::SharedBuffer("FIRMxxxx", 8);
    auto buffer = original;
    buffer.shrink(4);
    auto encoder = CxaPitchEncoder(Beam::Ref(buffer));
    encoder.pad(4);
    REQUIRE(
      std::string_view(buffer.get_data(), buffer.get_size()) == "FIRM    ");
    REQUIRE(
      std::string_view(original.get_data(), original.get_size()) == "FIRMxxxx");
  }

  TEST_CASE("padding_capacity") {
    auto buffer = Beam::StaticBuffer<8>("FIRMxxxx", 8);
    buffer.shrink(4);
    auto encoder = CxaPitchEncoder(Beam::Ref(buffer));
    REQUIRE_THROWS_AS(encoder.pad(5), std::out_of_range);
    REQUIRE(
      std::string_view(buffer.get_data(), buffer.get_size()) == "FIRMxxxx");
  }
}
