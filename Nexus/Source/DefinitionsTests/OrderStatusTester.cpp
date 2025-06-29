#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Definitions/OrderStatus.hpp"

using namespace Nexus;

namespace {
  void check_output(OrderStatus status, const char* expected) {
    auto out = std::ostringstream();
    out << status;
    CHECK(out.str() == expected);
  }
}

TEST_SUITE("OrderStatus") {
  TEST_CASE("is_terminal") {
    CHECK(is_terminal(OrderStatus::REJECTED));
    CHECK(is_terminal(OrderStatus::EXPIRED));
    CHECK(is_terminal(OrderStatus::CANCELED));
    CHECK(is_terminal(OrderStatus::FILLED));
    CHECK(is_terminal(OrderStatus::DONE_FOR_DAY));
    CHECK_FALSE(is_terminal(OrderStatus::PENDING_NEW));
    CHECK_FALSE(is_terminal(OrderStatus::NEW));
    CHECK_FALSE(is_terminal(OrderStatus::PARTIALLY_FILLED));
    CHECK_FALSE(is_terminal(OrderStatus::SUSPENDED));
    CHECK_FALSE(is_terminal(OrderStatus::STOPPED));
    CHECK_FALSE(is_terminal(OrderStatus::PENDING_CANCEL));
    CHECK_FALSE(is_terminal(OrderStatus::CANCEL_REJECT));
  }

  TEST_CASE("stream") {
    check_output(OrderStatus::PENDING_NEW, "PENDING_NEW");
    check_output(OrderStatus::REJECTED, "REJECTED");
    check_output(OrderStatus::NEW, "NEW");
    check_output(OrderStatus::PARTIALLY_FILLED, "PARTIALLY_FILLED");
    check_output(OrderStatus::EXPIRED, "EXPIRED");
    check_output(OrderStatus::CANCELED, "CANCELED");
    check_output(OrderStatus::SUSPENDED, "SUSPENDED");
    check_output(OrderStatus::STOPPED, "STOPPED");
    check_output(OrderStatus::FILLED, "FILLED");
    check_output(OrderStatus::DONE_FOR_DAY, "DONE_FOR_DAY");
    check_output(OrderStatus::PENDING_CANCEL, "PENDING_CANCEL");
    check_output(OrderStatus::CANCEL_REJECT, "CANCEL_REJECT");
    check_output(static_cast<OrderStatus>(-1), "NONE");
  }
}
