#include <stdexcept>
#include <vector>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayValueToListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Spire;

TEST_SUITE("ArrayValueToListModel") {
  TEST_CASE("transaction_updates_the_source") {
    auto source = make_local_value_model(std::vector{1, 2, 3});
    auto model = ArrayValueToListModel(source);
    model.transact([&] {
      model.set(0, 10);
      model.set(2, 30);
    });
    REQUIRE(source->get() == std::vector{10, 2, 30});
  }

  TEST_CASE("throwing_transaction_keeps_writing_to_the_source") {
    auto source = make_local_value_model(std::vector{1, 2, 3});
    auto model = ArrayValueToListModel(source);
    REQUIRE_THROWS(model.transact([&] {
      model.set(0, 10);
      throw std::runtime_error("Failed.");
    }));
    REQUIRE(source->get() == std::vector{10, 2, 3});
    model.set(1, 20);
    REQUIRE(source->get() == std::vector{10, 20, 3});
  }
}
