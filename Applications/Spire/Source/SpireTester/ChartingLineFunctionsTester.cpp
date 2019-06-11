#include <catch.hpp>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartPoint.hpp"
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/Charting/TrendLine.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

TEST_CASE("test_slope", "[Charting]") {
  REQUIRE(slope(10, 10, 20, 20) == 1);
  REQUIRE(slope(10, 10, 20, 30) == 2);
  REQUIRE(slope(20, 20, 30, 10) == -1);
  REQUIRE(slope(20, 20, 30, 0) == -2);
  REQUIRE(slope(10, 10, 20, 10) == 0);
  REQUIRE(slope(20, 10, 10, 10) == 0);
  REQUIRE(slope(10, 10, 10, 20) == std::numeric_limits<int>::infinity());
  REQUIRE(slope(10, 20, 10, 10) == std::numeric_limits<int>::infinity());
  print_test_name("test_slope");
}

TEST_CASE("test_distance_squared", "[Charting]") {
  print_test_name("test_distance_squared");
}

TEST_CASE("test_is_within_interval", "[Charting]") {
  REQUIRE(is_within_interval(5, 1, 10));
  REQUIRE(!is_within_interval(-5, 1, 10));
  REQUIRE(!is_within_interval(15, 1, 10));
  REQUIRE(is_within_interval(1, 1, 10));
  REQUIRE(is_within_interval(10, 1, 10));
  REQUIRE(is_within_interval(-5, -10, -1));
  REQUIRE(!is_within_interval(-15, -10, -1));
  REQUIRE(!is_within_interval(5, -10, -1));
  REQUIRE(is_within_interval(5, 1, 10, 2));
  REQUIRE(!is_within_interval(-5, 1, 10, 2));
  REQUIRE(!is_within_interval(15, 1, 10, 2));
  REQUIRE(is_within_interval(12, 1, 10, 2));
  REQUIRE(is_within_interval(-1, 1, 10, 2));
  REQUIRE(is_within_interval(-5, -10, -1, 2));
  REQUIRE(!is_within_interval(-15, -10, -1, 2));
  REQUIRE(!is_within_interval(5, -10, -1, 2));
  REQUIRE(is_within_interval(-12, -10, -1, 2));
  REQUIRE(is_within_interval(1, -10, -1, 2));
  print_test_name("test_is_within_interval");
}

TEST_CASE("test_y_intercept", "[Charting]") {
  REQUIRE(y_intercept(10.0, 10.0, 1.0) == 0.0);
  REQUIRE(y_intercept(-10.0, 10.0, -1.0) == 0.0);
  REQUIRE(y_intercept(10.0, 10.0, 2.0) == -10.0);
  REQUIRE(y_intercept(20.0, 20.0, -2.0) == 60.0);
  REQUIRE(y_intercept(10.0, 10.0, 0.0) == 10.0);
  REQUIRE(y_intercept(10.0, 10.0, 0.0) == 10.0);
  print_test_name("test_y_intercept");
}

TEST_CASE("test_calculate_y", "[Charting]") {
  print_test_name("test_calculate_y");
}

TEST_CASE("test_closest_point_distance_squared", "[Charting]") {
  print_test_name("test_closest_point_distance_squared");
}
