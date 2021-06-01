#include <doctest/doctest.h>
#include "Spire/Styles/EvaluatedBlock.hpp"
#include "Spire/Ui/Box.hpp"

using namespace Spire;
using namespace Spire::Styles;

TEST_SUITE("EvaluatedBlock") {
  TEST_CASE("to_block") {
    auto evaluated_block = EvaluatedBlock();
    evaluated_block.set(
      EvaluatedProperty(std::in_place_type<BackgroundColor>, QColor(0xFF0000)));
    evaluated_block.set(
      EvaluatedProperty(std::in_place_type<BorderTopColor>, QColor(0x00FF00)));
    auto block = evaluated_block.to_block();
    auto has_background_color = false;
    auto has_border_color = false;
    for(auto& property : block) {
      property.visit(
        [&] (const BackgroundColor& color) {
          REQUIRE(color.get_expression().as<
            ConstantExpression<QColor>>().get_constant() == QColor(0xFF0000));
          has_background_color = true;
        },
        [&] (const BorderTopColor& color) {
          REQUIRE(color.get_expression().as<
            ConstantExpression<QColor>>().get_constant() == QColor(0x00FF00));
          has_border_color = true;
        });
    }
    REQUIRE(has_background_color);
    REQUIRE(has_border_color);
  }
}
