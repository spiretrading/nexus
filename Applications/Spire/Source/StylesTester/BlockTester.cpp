#include <doctest/doctest.h>
#include "Spire/Styles/Block.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("Block") {
  TEST_CASE("modify_properties") {
    auto block = Block();
    block.set(ColorProperty(QColor(0xFF0000)));
    block.set(IntProperty(42));
    auto visits = 0;
    for(auto& property : block) {
      property.visit([&] (const ColorProperty& property) {
        auto expression =
          property.get_expression().as<ConstantExpression<QColor>>();
        REQUIRE(expression.get_constant() == QColor(0xFF0000));
        ++visits;
      },
      [&] (const IntProperty& property) {
        auto expression =
          property.get_expression().as<ConstantExpression<int>>();
        REQUIRE(expression.get_constant() == 42);
        ++visits;
      });
    }
    REQUIRE(visits == 2);
    block.set(ColorProperty(QColor(0x0000FF)));
    auto color = find<ColorProperty>(block);
    REQUIRE(color.is_initialized());
    auto expression =
      color->get_expression().as<ConstantExpression<QColor>>();
    REQUIRE(expression.get_constant() == QColor(0x0000FF));
    block.remove<IntProperty>();
  }

  TEST_CASE("modify_composite_properties") {
    auto block = Block();
    block.set(ColorIntProperty(QColor(0x0000FF), 18));
    auto color = find<ColorProperty>(block);
    REQUIRE(color.is_initialized());
    auto expression = color->get_expression();
    REQUIRE(expression.as<ConstantExpression<QColor>>().get_constant() ==
      QColor(0x0000FF));
  }
}
