#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Definitions/OrderStatus.hpp"

using namespace Nexus;

namespace {
  void require_output(OrderStatus status, const char* expected) {
    auto out = std::ostringstream();
    out << status;
    REQUIRE(out.str() == expected);
  }
}

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

  TEST_CASE("stream") {
    require_output(OrderStatus::PENDING_NEW, "PENDING_NEW");
    require_output(OrderStatus::REJECTED, "REJECTED");
    require_output(OrderStatus::NEW, "NEW");
    require_output(OrderStatus::PARTIALLY_FILLED, "PARTIALLY_FILLED");
    require_output(OrderStatus::EXPIRED, "EXPIRED");
    require_output(OrderStatus::CANCELED, "CANCELED");
    require_output(OrderStatus::SUSPENDED, "SUSPENDED");
    require_output(OrderStatus::STOPPED, "STOPPED");
    require_output(OrderStatus::FILLED, "FILLED");
    require_output(OrderStatus::DONE_FOR_DAY, "DONE_FOR_DAY");
    require_output(OrderStatus::PENDING_CANCEL, "PENDING_CANCEL");
    require_output(OrderStatus::CANCEL_REJECT, "CANCEL_REJECT");
    require_output(static_cast<OrderStatus>(-1), "NONE");
  }
}
