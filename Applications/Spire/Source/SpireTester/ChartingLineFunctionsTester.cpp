#include <catch2/catch.hpp>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartPoint.hpp"
#include "Spire/Charting/TrendLine.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

TEST_CASE("test_slope", "[Charting]") {
  REQUIRE(slope({10, 10}, {20, 20}) == 1);
  REQUIRE(slope({10, 10}, {20, 30}) == 2);
  REQUIRE(slope({20, 20}, {30, 10}) == -1);
  REQUIRE(slope({20, 20}, {30, 0}) == -2);
  REQUIRE(slope({10, 10}, {20, 10}) == 0);
  REQUIRE(slope({20, 10}, {10, 10}) == 0);
  print_test_name("test_slope");
}

TEST_CASE("test_distance_squared", "[Charting]") {
  REQUIRE(distance_squared({10, 10}, {10, 12}) == 4);
  REQUIRE(distance_squared({10, 10}, {12, 10}) == 4);
  REQUIRE(distance_squared({10, 10}, {10, 8}) == 4);
  REQUIRE(distance_squared({10, 10}, {8, 10}) == 4);
  REQUIRE(distance_squared({10, 10}, {9, 11}) == 2);
  REQUIRE(distance_squared({10, 10}, {11, 9}) == 2);
  REQUIRE(distance_squared({-10, -10}, {-10, -12}) == 4);
  REQUIRE(distance_squared({-10, -10}, {-12, -10}) == 4);
  REQUIRE(distance_squared({-10, -10}, {-10, -8}) == 4);
  REQUIRE(distance_squared({-10, -10}, {-8, -10}) == 4);
  REQUIRE(distance_squared({-10, -10}, {-9, -11}) == 2);
  REQUIRE(distance_squared({-10, -10}, {-11, -9}) == 2);
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
  REQUIRE(y_intercept({10.0, 10.0}, 1.0) == 0.0);
  REQUIRE(y_intercept({-10.0, 10.0}, -1.0) == 0.0);
  REQUIRE(y_intercept({10.0, 10.0}, 2.0) == -10.0);
  REQUIRE(y_intercept({20.0, 20.0}, -2.0) == 60.0);
  REQUIRE(y_intercept({10.0, 10.0}, 0.0) == 10.0);
  REQUIRE(y_intercept({10.0, 10.0}, 0.0) == 10.0);
  print_test_name("test_y_intercept");
}

TEST_CASE("test_calculate_y", "[Charting]") {
  REQUIRE(calculate_y(1.0, 10.0, 0.0) == 10.0);
  REQUIRE(calculate_y(-1.0, 10.0, 0.0) == -10.0);
  REQUIRE(calculate_y(2.0, 10.0, -10.0) == 10.0);
  REQUIRE(calculate_y(-2.0, 10.0, -10.0) == -30.0);
  REQUIRE(calculate_y(1.0, 10.0, 0.0) == 10.0);
  print_test_name("test_calculate_y");
}

TEST_CASE("test_closest_point_distance_squared", "[Charting]") {
  REQUIRE(closest_point_distance_squared({10, 10}, {7, 10}, {10, 5}) == 9);
  REQUIRE(closest_point_distance_squared({10, 10}, {10, 15}, {16, 10}) == 25);
  REQUIRE(closest_point_distance_squared({10.0, 10.0}, {8.0, 12.0},
    {14.0, 10.0}) == 8.0);
  REQUIRE(closest_point_distance_squared({-10, -10}, {-7, -10}, {-10, -5}) ==
    9);
  REQUIRE(closest_point_distance_squared({-10, -10}, {-10, -15}, {-16, -10}) ==
    25);
  REQUIRE(closest_point_distance_squared({-10.0, -10.0}, {-8.0, -12.0},
    {-14.0, -10.0}) == 8.0);
  print_test_name("test_closest_point_distance_squared");
}
