#include <doctest/doctest.h>
#include "Spire/Spire/ValidatedValueModel.hpp"

using namespace Spire;

TEST_SUITE("ValidatedValueModel") {
  TEST_CASE("validate") {
    auto source = std::make_shared<LocalValueModel<int>>(2);
    auto model = make_validated_value_model([] (auto value) {
      if(value % 2 == 0) {
        return QValidator::State::Acceptable;
      }
      return QValidator::State::Invalid;
    }, source);
    REQUIRE(model->get() == 2);
    REQUIRE(model->set(1) == QValidator::Invalid);
    REQUIRE(model->get() == 2);
    REQUIRE(model->set(4) == QValidator::Acceptable);
    REQUIRE(model->get() == 4);
  }
}
