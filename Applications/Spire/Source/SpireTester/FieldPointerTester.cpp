#include <doctest/doctest.h>
#include "Spire/Spire/FieldPointer.hpp"

using namespace Spire;

namespace {
  struct Point {
    int m_x;
    int m_y;
  };
}

TEST_SUITE("FieldPointer") {
  TEST_CASE("access") {
    auto x = FieldPointer(&Point::m_x);
    auto y = FieldPointer(&Point::m_y);
    auto point = Point(12, 34);
    REQUIRE(x.access<int>(point) == 12);
    REQUIRE(y.access<int>(point) == 34);
    REQUIRE(x != y);
    REQUIRE(x == &Point::m_x);
    REQUIRE(y == &Point::m_y);
    REQUIRE(x != &Point::m_y);
    REQUIRE(y != &Point::m_x);
  }

  TEST_CASE("null") {
    auto x = FieldPointer();
    auto y = FieldPointer();
    auto z = FieldPointer(&Point::m_x);
    REQUIRE(x == nullptr);
    REQUIRE(x == y);
    REQUIRE(x != z);
  }
}
