#include <doctest/doctest.h>
#include "Spire/Styles/EvaluatedBlock.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("EvaluatedBlock") {
  TEST_CASE("to_block") {
    auto evaluated_block = EvaluatedBlock();
    evaluated_block.set(
      EvaluatedProperty(std::in_place_type<ColorProperty>, QColor(0xFF0000)));
    evaluated_block.set(EvaluatedProperty(std::in_place_type<IntProperty>, 13));
    auto block = evaluated_block.to_block();
    auto has_color = false;
    auto has_int = false;
    for(auto& property : block) {
      property.visit(
        [&] (const ColorProperty& property) {
          REQUIRE(property.get_expression().as<
            ConstantExpression<QColor>>().get_constant() == QColor(0xFF0000));
          has_color = true;
        },
        [&] (const IntProperty& property) {
          REQUIRE(property.get_expression().as<
            ConstantExpression<int>>().get_constant() == 13);
          has_int = true;
        });
    }
    REQUIRE(has_color);
    REQUIRE(has_int);
  }
}
