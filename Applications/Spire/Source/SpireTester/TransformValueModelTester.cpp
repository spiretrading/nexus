#include <doctest/doctest.h>
#include "Spire/Spire/TransformValueModel.hpp"

using namespace Spire;

TEST_SUITE("TransformValueModel") {
  TEST_CASE("simplified_transformers") {
    auto source = std::make_shared<LocalValueModel<int>>(1);
    auto model = TransformValueModel(source,
      [] (int current) {
        return std::to_string(current);
      },
      [] (const std::string& current) {
        return std::stoi(current);
      });
    REQUIRE(model.get_current() == "1");
    source->set_current(123);
    REQUIRE(model.get_current() == "123");
    REQUIRE(model.set_current("221") == QValidator::State::Acceptable);
    REQUIRE(source->get_current() == 221);
    REQUIRE(model.set_current("abc") == QValidator::State::Invalid);
    REQUIRE(source->get_current() == 221);
  }

  TEST_CASE("elaborate_transformers") {
    auto source = std::make_shared<LocalValueModel<int>>(1);
    auto model = TransformValueModel(source,
      [] (int current) {
        return std::to_string(current);
      },
      [] (int current, const std::string& next) {
        return std::stoi(next);
      });
    REQUIRE(model.get_current() == "1");
    source->set_current(123);
    REQUIRE(model.get_current() == "123");
    REQUIRE(model.set_current("221") == QValidator::State::Acceptable);
    REQUIRE(source->get_current() == 221);
    REQUIRE(model.set_current("abc") == QValidator::State::Invalid);
    REQUIRE(source->get_current() == 221);
  }

  TEST_CASE("shared_factory") {
    auto source = std::make_shared<LocalValueModel<int>>(1);
    auto model = make_transform_value_model(source,
      [] (int current) {
        return std::to_string(current);
      },
      [] (const std::string& next) {
        return std::stoi(next);
      });
  }
}
