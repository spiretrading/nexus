#include <doctest/doctest.h>
#include "Spire/Spire/Field.hpp"

using namespace Spire;

namespace {
  struct Point {
    int m_x;
    int m_y;
  };
}

TEST_SUITE("Field") {
  TEST_CASE("access") {
    auto field = Field(&Point::m_x);
    auto point = Point(12, 54);
    REQUIRE(field.access<int>(point) == 12);
  }

  TEST_CASE("comparison") {
    auto x_field = Field(&Point::m_x);
    auto x2_field = Field(&Point::m_x);
    auto y_field = Field(&Point::m_y);
    REQUIRE(x_field == x2_field);
    REQUIRE(x_field != y_field);
  }
}
