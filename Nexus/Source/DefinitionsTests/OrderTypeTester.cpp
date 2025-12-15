#include <doctest/doctest.h>
#include "Nexus/Definitions/OrderType.hpp"

using namespace Nexus;

TEST_SUITE("OrderType") {
  TEST_CASE("to_char") {
    REQUIRE(to_char(OrderType::MARKET) == '1');
    REQUIRE(to_char(OrderType::LIMIT) == '2');
    REQUIRE(to_char(OrderType::PEGGED) == 'P');
    REQUIRE(to_char(OrderType::STOP) == '3');
    REQUIRE_THROWS_AS(
      (void)(to_char(static_cast<OrderType>(-1))), std::runtime_error);
  }
}
