#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/FeeHandling/LiquidityFlag.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("LiquidityFlag") {
  TEST_CASE("stream") {
    REQUIRE(to_string(LiquidityFlag::NONE) == "");
    REQUIRE(to_string(LiquidityFlag::ACTIVE) == "A");
    REQUIRE(to_string(LiquidityFlag::PASSIVE) == "P");
  }
}
