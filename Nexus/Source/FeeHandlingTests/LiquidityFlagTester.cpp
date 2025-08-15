#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/FeeHandling/LiquidityFlag.hpp"

using namespace Nexus;

TEST_SUITE("liquidity_flag") {
  TEST_CASE("stream") {
    auto stream = std::ostringstream();
    stream << LiquidityFlag::NONE;
    REQUIRE(stream.str().empty());
    stream.str("");
    stream << LiquidityFlag::ACTIVE;
    REQUIRE(stream.str() == "A");
    stream.str("");
    stream << LiquidityFlag::PASSIVE;
    REQUIRE(stream.str() == "P");
  }
}
