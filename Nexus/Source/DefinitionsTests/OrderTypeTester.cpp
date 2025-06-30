#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Definitions/OrderType.hpp"

using namespace Nexus;

namespace {
  void require_output(OrderType value, const char* expected) {
    auto out = std::ostringstream();
    out << value;
    REQUIRE(out.str() == expected);
  };
}

TEST_SUITE("OrderType") {
  TEST_CASE("to_char") {
    REQUIRE(to_char(OrderType::MARKET) == '1');
    REQUIRE(to_char(OrderType::LIMIT) == '2');
    REQUIRE(to_char(OrderType::PEGGED) == 'P');
    REQUIRE(to_char(OrderType::STOP) == '3');
    REQUIRE_THROWS_AS(
      (void)(to_char(static_cast<OrderType>(-1))), std::runtime_error);
  }

  TEST_CASE("stream") {
    require_output(OrderType::MARKET, "MARKET");
    require_output(OrderType::LIMIT, "LIMIT");
    require_output(OrderType::PEGGED, "PEGGED");
    require_output(OrderType::STOP, "STOP");
    require_output(static_cast<OrderType>(-1), "NONE");
  }
}
