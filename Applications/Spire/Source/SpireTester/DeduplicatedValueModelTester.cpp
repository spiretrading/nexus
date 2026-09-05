#include <doctest/doctest.h>
#include "Spire/Spire/DeduplicatedValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Spire;

TEST_SUITE("DeduplicatedValueModel") {
  TEST_CASE("update") {
    auto source = std::make_shared<LocalValueModel<int>>(1);
    auto model = make_deduplicated_value_model(source);
    REQUIRE(model->get() == 1);
    auto updates = 0;
    auto value = 0;
    auto connection = model->connect_update_signal([&] (auto update) {
      ++updates;
      value = update;
    });
    source->set(2);
    REQUIRE(updates == 1);
    REQUIRE(value == 2);
    REQUIRE(model->get() == 2);
    source->set(2);
    REQUIRE(updates == 1);
    REQUIRE(model->get() == 2);
    source->set(3);
    REQUIRE(updates == 2);
    REQUIRE(value == 3);
    REQUIRE(model->get() == 3);
  }

  TEST_CASE("set") {
    auto source = std::make_shared<LocalValueModel<int>>(1);
    auto model = make_deduplicated_value_model(source);
    auto updates = 0;
    auto connection = model->connect_update_signal([&] (auto update) {
      ++updates;
    });
    REQUIRE(model->set(5) == QValidator::State::Acceptable);
    REQUIRE(source->get() == 5);
    REQUIRE(model->get() == 5);
    REQUIRE(updates == 1);
    REQUIRE(model->set(5) == QValidator::State::Acceptable);
    REQUIRE(source->get() == 5);
    REQUIRE(model->get() == 5);
    REQUIRE(updates == 1);
  }
}
