#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/FeeHandling/LiquidityFlag.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("LiquidityFlag") {
  TEST_CASE("format") {
    REQUIRE(std::format("{}", LiquidityFlag::NONE) == "");
    REQUIRE(std::format("{}", LiquidityFlag::ACTIVE) == "A");
    REQUIRE(std::format("{}", LiquidityFlag::PASSIVE) == "P");
    REQUIRE(std::format("{}", static_cast<LiquidityFlag>(99)) == "?");
    REQUIRE(to_string(static_cast<LiquidityFlag>(99)) == "?");
    REQUIRE(std::format("{:*>3}", LiquidityFlag::ACTIVE) == "**A");
    REQUIRE(std::format("{:>2}", LiquidityFlag::NONE) == "  ");
  }

  TEST_CASE("stream") {
    REQUIRE(to_string(LiquidityFlag::NONE) == "");
    REQUIRE(to_string(LiquidityFlag::ACTIVE) == "A");
    REQUIRE(to_string(LiquidityFlag::PASSIVE) == "P");
  }
}
