#include <doctest/doctest.h>
#include "Spire/Styles/Block.hpp"
#include "Spire/Ui/Box.hpp"

using namespace Spire;
using namespace Spire::Styles;

TEST_SUITE("Block") {
  TEST_CASE("modify_properties") {
    auto block = Block();
    block.set(BackgroundColor(QColor::fromRgb(255, 0, 0)));
    block.set(BorderTopColor(QColor::fromRgb(0, 255, 0)));
    auto visits = 0;
    for(auto& property : block.get_properties()) {
      property.visit([&] (const BackgroundColor& color) {
        auto expression =
          color.get_expression().as<ConstantExpression<QColor>>();
        REQUIRE(expression.get_constant() == QColor::fromRgb(255, 0, 0));
        ++visits;
      },
      [&] (const BorderTopColor& color) {
        auto expression =
          color.get_expression().as<ConstantExpression<QColor>>();
        REQUIRE(expression.get_constant() == QColor::fromRgb(0, 255, 0));
        ++visits;
      });
    }
    REQUIRE(visits == 2);
    block.set(BackgroundColor(QColor::fromRgb(0, 0, 255)));
    auto background = find<BackgroundColor>(block);
    REQUIRE(background.is_initialized());
    auto expression =
      background->get_expression().as<ConstantExpression<QColor>>();
    REQUIRE(expression.get_constant() == QColor::fromRgb(0, 0, 255));
    block.remove<BackgroundColor>();
    REQUIRE(block.get_properties().size() == 1);
    REQUIRE_NOTHROW(block.get_properties().front().visit(
      [] (const BorderTopColor& color) {}));
  }

  TEST_CASE("modify_composite_properties") {
    auto block = Block();
    block.set(border(1, QColor::fromRgb(0, 0, 255)));
    auto border_color = find<BorderColor>(block);
    REQUIRE(border_color.is_initialized());
    auto expression = border_color->get<BorderTopColor>().get_expression();
    REQUIRE(expression.as<ConstantExpression<QColor>>().get_constant() ==
      QColor::fromRgb(0, 0, 255));
  }
}
