#include <doctest/doctest.h>
#include "Spire/Styles/Block.hpp"
#include "Spire/Styles/BoxStyles.hpp"

using namespace Spire;
using namespace Spire::Styles;

TEST_SUITE("Block") {
  TEST_CASE("modify_properties") {
    auto block = Block();
    block.set(BackgroundColor(QColor::fromRgb(255, 0, 0)));
    block.set(BorderColor(QColor::fromRgb(0, 255, 0)));
    auto visits = 0;
    for(auto& property : block.get_properties()) {
      property.visit([&] (const BackgroundColor& color) {
        REQUIRE(color.get_expression().as<QColor>() ==
          QColor::fromRgb(255, 0, 0));
        ++visits;
      },
      [&] (const BorderColor& color) {
        REQUIRE(color.get_expression().as<QColor>() ==
          QColor::fromRgb(0, 255, 0));
        ++visits;
      });
    }
    REQUIRE(visits == 2);
    block.set(BackgroundColor(QColor::fromRgb(0, 0, 255)));
    auto background = find<BackgroundColor>(block);
    REQUIRE(background.is_initialized());
    REQUIRE(background->get_expression().as<QColor>() ==
      QColor::fromRgb(0, 0, 255));
    block.remove<BackgroundColor>();
    REQUIRE(block.get_properties().size() == 1);
    REQUIRE_NOTHROW(block.get_properties().front().visit(
      [&] (const BorderColor& color) {}));
  }
}
