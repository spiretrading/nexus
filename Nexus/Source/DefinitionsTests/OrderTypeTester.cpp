#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Definitions/OrderType.hpp"

using namespace Nexus;

namespace {
  void check_output(OrderType value, const char* expected) {
    auto out = std::ostringstream();
    out << value;
    CHECK(out.str() == expected);
  };
}

TEST_SUITE("OrderType") {
  TEST_CASE("to_char") {
    CHECK(to_char(OrderType::MARKET) == '1');
    CHECK(to_char(OrderType::LIMIT) == '2');
    CHECK(to_char(OrderType::PEGGED) == 'P');
    CHECK(to_char(OrderType::STOP) == '3');
    CHECK_THROWS_AS(
      (void)(to_char(static_cast<OrderType>(-1))), std::runtime_error);
  }

  TEST_CASE("stream") {
    check_output(OrderType::MARKET, "MARKET");
    check_output(OrderType::LIMIT, "LIMIT");
    check_output(OrderType::PEGGED, "PEGGED");
    check_output(OrderType::STOP, "STOP");
    check_output(static_cast<OrderType>(-1), "NONE");
  }
}
