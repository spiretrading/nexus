#include <doctest/doctest.h>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"

using namespace Spire;

TEST_SUITE("ToTextModel") {
  TEST_CASE("state") {
    auto local_model = std::make_shared<LocalValueModel<int>>(1);
    auto model = ToTextModel<int>(local_model);
    REQUIRE(model.get_state() == QValidator::Acceptable);
    local_model->set_current(2);
    REQUIRE(model.get_state() == QValidator::Acceptable);
    model.set_current("3");
    REQUIRE(model.get_state() == QValidator::Acceptable);
    REQUIRE(model.set_current("4") == QValidator::Acceptable);
    REQUIRE(model.get_state() == QValidator::Acceptable);
    REQUIRE(model.set_current("five") == QValidator::Invalid);
    REQUIRE(model.get_state() == QValidator::Acceptable);
  }

  TEST_CASE("current") {
    auto local_model = std::make_shared<LocalValueModel<int>>(1);
    auto model = ToTextModel<int>(local_model);
    REQUIRE(model.get() == QString("1"));
    local_model->set_current(2);
    REQUIRE(model.get() == QString("2"));
    model.set_current("3");
    REQUIRE(model.get() == QString("3"));
    model.set_current("four");
    REQUIRE(model.get() == QString("3"));
  }

  TEST_CASE("current_signal") {
    auto local_model = std::make_shared<LocalValueModel<int>>(1);
    auto model = ToTextModel<int>(local_model);
    auto current = QString();
    auto call_count = 0;
    model.connect_current_signal([&] (auto value) {
      current = value;
      ++call_count;
    });
    REQUIRE(current == QString());
    REQUIRE(call_count == 0);
    local_model->set_current(2);
    REQUIRE(current == QString("2"));
    REQUIRE(call_count == 1);
    model.set_current("3");
    REQUIRE(current == QString("3"));
    REQUIRE(call_count == 2);
    model.set_current("3");
    REQUIRE(current == QString("3"));
    REQUIRE(call_count == 3);
    model.set_current("four");
    REQUIRE(current == QString("3"));
    REQUIRE(call_count == 3);
  }
}
