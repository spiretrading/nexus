#include <doctest/doctest.h>
#include "Spire/Styles/EvaluatedProperty.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("EvaluatedProperty") {
  TEST_CASE("property_type") {
    {
      auto property =
        EvaluatedProperty(ColorProperty(QColor(0xFF0000)), QColor(0xFF0000));
      REQUIRE(property.get_property_type() == typeid(ColorProperty));
    }
    {
      auto property =
        EvaluatedProperty(std::in_place_type<ColorProperty>, QColor(0xFF0000));
      REQUIRE(property.get_property_type() == typeid(ColorProperty));
    }
  }

  TEST_CASE("as_property") {
    auto evaluated_property =
      EvaluatedProperty(ColorProperty(QColor(0xFF0000)), QColor(0xFF0000));
    auto property = evaluated_property.as_property();
    REQUIRE(property.get_type() == typeid(ColorProperty));
    REQUIRE(property.expression_as<QColor>().as<
      ConstantExpression<QColor>>().get_constant() == QColor(0xFF0000));
  }

  TEST_CASE("as") {
    auto property = EvaluatedProperty(IntProperty(42), 42);
    REQUIRE(property.as<int>() == 42);
  }

  TEST_CASE("equality") {
    auto a = EvaluatedProperty(IntProperty(42), 42);
    auto b = EvaluatedProperty(IntProperty(42), 42);
    auto c = EvaluatedProperty(IntProperty(43), 43);
    auto d =
      EvaluatedProperty(ColorProperty(QColor(0xFF0000)), QColor(0xFF0000));
    REQUIRE(a == b);
    REQUIRE(b != c);
    REQUIRE(c != d);
  }
}
