#include <doctest/doctest.h>
#include "Spire/Styles/CubicBezierExpression.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

class Spire::Styles::Stylist {};

TEST_SUITE("CubicBezierExpression") {
  TEST_CASE("constructor") {
    auto expression = cubic_bezier(5, 15, 0.3, 0.4, 0.6, 0.8, milliseconds(250));
    REQUIRE(expression.get_start() == ConstantExpression(5));
    REQUIRE(expression.get_end() == ConstantExpression(15));
    REQUIRE(expression.get_x1() == doctest::Approx(0.3));
    REQUIRE(expression.get_y1() == doctest::Approx(0.4));
    REQUIRE(expression.get_x2() == doctest::Approx(0.6));
    REQUIRE(expression.get_y2() == doctest::Approx(0.8));
    REQUIRE(expression.get_duration() == milliseconds(250));
  }
  TEST_CASE("linear_evaluation") {
    auto expression =
      cubic_bezier(10.0, 20.0, 0.0, 0.0, 1.0, 1.0, milliseconds(1000));
    auto evaluator = make_evaluator(expression, Stylist());
    auto r0 = evaluator(milliseconds(0));
    REQUIRE(r0.m_value == doctest::Approx(10.0));
    auto r500 = evaluator(milliseconds(500));
    REQUIRE(r500.m_value == doctest::Approx(15.0));
    auto r1000 = evaluator(milliseconds(1000));
    REQUIRE(r1000.m_value == doctest::Approx(20.0));
  }
  TEST_CASE("ease_in_out") {
    auto expression =
      cubic_bezier(0.0, 1.0, 0.42, 0.0, 0.58, 1.0, milliseconds(1000));
    auto evaluator = make_evaluator(expression, Stylist());
    REQUIRE(evaluator(milliseconds(0)).m_value == doctest::Approx(0.0));
    REQUIRE(evaluator(milliseconds(1000)).m_value == doctest::Approx(1.0));
    auto v25 = evaluator(milliseconds(250)).m_value;
    REQUIRE(v25 < doctest::Approx(0.25));
    auto v75 = evaluator(milliseconds(750)).m_value;
    REQUIRE(v75 > doctest::Approx(0.75));
  }
  TEST_CASE("clamp") {
    auto expression =
      cubic_bezier(3.14, 6.28, 0.2, 0.2, 0.8, 0.8, milliseconds(500));
    auto evaluator = make_evaluator(expression, Stylist());
    REQUIRE(evaluator(milliseconds(-100)).m_value == doctest::Approx(3.14));
    REQUIRE(evaluator(milliseconds(600)).m_value == doctest::Approx(6.28));
  }
}
