#include <doctest/doctest.h>
#include "Spire/Styles/ChainExpression.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

class Spire::Styles::Stylist {};

TEST_SUITE("ChainExpression") {
  TEST_CASE("evaluate") {
    auto expression = chain(42, 55);
    auto stylist = Stylist();
    auto evaluator = make_evaluator(expression, stylist);
    auto evaluation = evaluator(seconds(0));
    REQUIRE(evaluation.m_value == 42);
    REQUIRE(evaluation.m_next_frame == seconds(0));
    evaluation = evaluator(seconds(1));
    REQUIRE(evaluation.m_value == 42);
    REQUIRE(evaluation.m_next_frame == seconds(0));
  }
}
