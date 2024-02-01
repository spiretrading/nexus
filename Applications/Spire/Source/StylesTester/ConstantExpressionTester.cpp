#include <doctest/doctest.h>
#include "Spire/Styles/ConstantExpression.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

class Spire::Styles::Stylist {};

TEST_SUITE("ConstantExpression") {
  TEST_CASE("evaluate") {
    auto constant = ConstantExpression(42);
    REQUIRE(constant.get_constant() == 42);
    auto stylist = Stylist();
    auto evaluator = make_evaluator(constant, stylist);
    auto evaluation = evaluator(seconds(0));
    REQUIRE(evaluation.m_value == 42);
    REQUIRE(evaluation.m_next_frame == pos_infin);
  }
}
