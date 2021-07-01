#include <doctest/doctest.h>
#include "Spire/Spire/ConstantValueModel.hpp"

using namespace Spire;

TEST_SUITE("ConstantValueModel") {
  TEST_CASE("constructor") {
    auto model = ConstantValueModel(123);
    REQUIRE(model.get_current() == 123);
    REQUIRE(model.set_current(321) == QValidator::Invalid);
  }

  TEST_CASE("maker") {
    auto model = make_constant_value_model(321);
    REQUIRE(model->get_current() == 321);
    REQUIRE(model->set_current(123) == QValidator::Invalid);
  }
}
