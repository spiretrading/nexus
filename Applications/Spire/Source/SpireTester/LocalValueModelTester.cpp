#include <doctest/doctest.h>
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Spire;

TEST_SUITE("LocalValueModel") {
  TEST_CASE("default") {
    auto model = LocalValueModel<int>();
    REQUIRE(model.get() == 0);
  }

  TEST_CASE("initial") {
    auto model = LocalValueModel(42);
    REQUIRE(model.get() == 42);
  }

  TEST_CASE("set_and_get") {
    auto model = LocalValueModel(0);
    REQUIRE(model.get() == 0);
    REQUIRE(model.set(10) == QValidator::Acceptable);
    REQUIRE(model.get() == 10);
  }

  TEST_CASE("test") {
    auto model = LocalValueModel(5);
    REQUIRE(model.test(5) == QValidator::Acceptable);
  }

  TEST_CASE("update") {
    auto model = LocalValueModel(1);
    auto update_count = 0;
    auto connection = model.connect_update_signal([&] (const auto& value) {
      ++update_count;
      CHECK(value == model.get());
    });
    model.set(2);
    REQUIRE(update_count == 1);
    model.set(3);
    REQUIRE(update_count == 2);
    connection.disconnect();
    model.set(4);
    REQUIRE(update_count == 2);
  }
}
