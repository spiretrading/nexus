#include <doctest/doctest.h>
#include "Spire/Styles/ConstantExpression.hpp"
#include "Spire/Styles/MinimumExpression.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

class Spire::Styles::Stylist {};

TEST_SUITE("MinimumExpression") {
  TEST_CASE("evaluate") {
    auto expression = min<int>(12, 21);
    auto stylist = Stylist();
    auto evaluator = make_evaluator(expression, stylist);
    auto evaluation = evaluator(seconds(0));
    REQUIRE(evaluation.m_value == 12);
    REQUIRE(evaluation.m_next_frame == pos_infin);
  }

  TEST_CASE("compound") {
    auto expression = min<int>(min<int>(3, 12), 21);
    auto stylist = Stylist();
    auto evaluator = make_evaluator(expression, stylist);
    auto evaluation = evaluator(seconds(0));
    REQUIRE(evaluation.m_value == 3);
    REQUIRE(evaluation.m_next_frame == pos_infin);
  }
}
