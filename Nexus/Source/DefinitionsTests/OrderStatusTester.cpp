#include <doctest/doctest.h>
#include "Nexus/Definitions/OrderStatus.hpp"

using namespace Nexus;

TEST_SUITE("OrderStatus") {
  TEST_CASE("is_terminal") {
    REQUIRE(is_terminal(OrderStatus::REJECTED));
    REQUIRE(is_terminal(OrderStatus::EXPIRED));
    REQUIRE(is_terminal(OrderStatus::CANCELED));
    REQUIRE(is_terminal(OrderStatus::FILLED));
    REQUIRE(is_terminal(OrderStatus::DONE_FOR_DAY));
    REQUIRE_FALSE(is_terminal(OrderStatus::PENDING_NEW));
    REQUIRE_FALSE(is_terminal(OrderStatus::NEW));
    REQUIRE_FALSE(is_terminal(OrderStatus::PARTIALLY_FILLED));
    REQUIRE_FALSE(is_terminal(OrderStatus::SUSPENDED));
    REQUIRE_FALSE(is_terminal(OrderStatus::STOPPED));
    REQUIRE_FALSE(is_terminal(OrderStatus::PENDING_CANCEL));
    REQUIRE_FALSE(is_terminal(OrderStatus::CANCEL_REJECT));
  }
}
