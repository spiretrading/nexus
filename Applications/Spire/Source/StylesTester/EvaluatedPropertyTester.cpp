#include <doctest/doctest.h>
#include "Spire/Styles/EvaluatedProperty.hpp"
#include "Spire/Ui/Box.hpp"

using namespace Spire;
using namespace Spire::Styles;

TEST_SUITE("EvaluatedProperty") {
  TEST_CASE("property_type") {
    {
      auto property =
        EvaluatedProperty(BackgroundColor(QColor(0xFF0000)), QColor(0xFF0000));
      REQUIRE(property.get_property_type() == typeid(BackgroundColor));
    }
    {
      auto property = EvaluatedProperty(
        std::in_place_type<BackgroundColor>, QColor(0xFF0000));
      REQUIRE(property.get_property_type() == typeid(BackgroundColor));
    }
  }

  TEST_CASE("as_property") {
    auto evaluated_property =
      EvaluatedProperty(BackgroundColor(QColor(0xFF0000)), QColor(0xFF0000));
    auto property = evaluated_property.as_property();
    REQUIRE(property.get_type() == typeid(BackgroundColor));
    REQUIRE(property.expression_as<QColor>().as<
      ConstantExpression<QColor>>().get_constant() == QColor(0xFF0000));
  }

  TEST_CASE("as") {
    auto property = EvaluatedProperty(PaddingTop(42), 42);
    REQUIRE(property.as<int>() == 42);
  }

  TEST_CASE("equality") {
    auto a = EvaluatedProperty(PaddingTop(42), 42);
    auto b = EvaluatedProperty(PaddingTop(42), 42);
    auto c = EvaluatedProperty(PaddingTop(43), 43);
    auto d =
      EvaluatedProperty(BackgroundColor(QColor(0xFF0000)), QColor(0xFF0000));
    REQUIRE(a == b);
    REQUIRE(b != c);
    REQUIRE(c != d);
  }
}
