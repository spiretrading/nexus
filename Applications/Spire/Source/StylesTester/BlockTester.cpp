#include <doctest/doctest.h>
#include "Spire/Styles/Block.hpp"
#include "Spire/Ui/Box.hpp"

using namespace Spire;
using namespace Spire::Styles;

TEST_SUITE("Block") {
  TEST_CASE("modify_properties") {
    auto block = Block();
    block.set(BackgroundColor(QColor(0xFF0000)));
    block.set(BorderTopColor(QColor(0x00FF00)));
    auto visits = 0;
    for(auto& property : block) {
      property.visit([&] (const BackgroundColor& color) {
        auto expression =
          color.get_expression().as<ConstantExpression<QColor>>();
        REQUIRE(expression.get_constant() == QColor(0xFF0000));
        ++visits;
      },
      [&] (const BorderTopColor& color) {
        auto expression =
          color.get_expression().as<ConstantExpression<QColor>>();
        REQUIRE(expression.get_constant() == QColor(0x00FF00));
        ++visits;
      });
    }
    REQUIRE(visits == 2);
    block.set(BackgroundColor(QColor(0x0000FF)));
    auto background = find<BackgroundColor>(block);
    REQUIRE(background.is_initialized());
    auto expression =
      background->get_expression().as<ConstantExpression<QColor>>();
    REQUIRE(expression.get_constant() == QColor(0x0000FF));
    block.remove<BackgroundColor>();
  }

  TEST_CASE("modify_composite_properties") {
    auto block = Block();
    block.set(border(1, QColor(0x0000FF)));
    auto border_color = find<BorderColor>(block);
    REQUIRE(border_color.is_initialized());
    auto expression = border_color->get<BorderTopColor>().get_expression();
    REQUIRE(expression.as<ConstantExpression<QColor>>().get_constant() ==
      QColor(0x0000FF));
  }
}
