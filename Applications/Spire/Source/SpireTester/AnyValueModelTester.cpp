#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/AnyValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Spire;

TEST_SUITE("AnyValueModel") {
  TEST_CASE("wrapper") {
    auto source = std::make_shared<LocalValueModel<int>>(123);
    auto operations = std::deque<AnyRef>();
    auto any_model = AnyValueModel(source);
    any_model.connect_update_signal([&] (const auto& value) {
      operations.push_back(value);
    });
    REQUIRE(any_cast<int>(any_model.get()) == 123);
    REQUIRE(any_model.set(AnyRef(55)) == QValidator::State::Acceptable);
    REQUIRE(operations.size() == 1);
    auto update = operations.front();
    operations.pop_front();
    REQUIRE(any_cast<const int>(update) == 55);
    REQUIRE(source->get() == 55);
    REQUIRE(any_model.set(AnyRef(3.14)) == QValidator::State::Invalid);
    REQUIRE(source->get() == 55);
    source->set(321);
    REQUIRE(operations.size() == 1);
    update = operations.front();
    operations.pop_front();
    REQUIRE(any_cast<const int>(update) == 321);
  }
}
