#include <stdexcept>
#include <vector>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayValueToListModel.hpp"
#include "Spire/Spire/FilteredListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Spire;

TEST_SUITE("ArrayValueToListModel") {
  TEST_CASE("source_update_exception") {
    auto source = make_local_value_model(std::vector{1, 2});
    auto model = std::make_shared<ArrayValueToListModel<int>>(source);
    auto filtered = FilteredListModel(model, [] (const auto& list, int index) {
      return list.get(index) < 0;
    });
    auto connection = source->connect_update_signal([] (const auto&) {
      throw std::runtime_error("Failed.");
    });
    SUBCASE("remove") {
      REQUIRE_THROWS_AS(model->remove(0), std::runtime_error);
      REQUIRE(filtered.get_size() == 1);
      REQUIRE(filtered.get(0) == 2);
    }
    SUBCASE("insert") {
      REQUIRE_THROWS_AS(model->insert(4, 0), std::runtime_error);
      REQUIRE(filtered.get_size() == 3);
      REQUIRE(filtered.get(0) == 4);
    }
    SUBCASE("update") {
      REQUIRE_THROWS_AS(model->set(0, -1), std::runtime_error);
      REQUIRE(filtered.get_size() == 1);
      REQUIRE(filtered.get(0) == 2);
    }
    connection.disconnect();
    auto index = model->get_size();
    auto operations = std::vector<ListModel<int>::Operation>();
    model->connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); });
    model->push(3);
    REQUIRE(operations.size() == 1);
    REQUIRE(
      std::get<ListModel<int>::AddOperation>(operations.front()).m_index ==
        index);
  }

  TEST_CASE("transaction_updates_the_source") {
    auto source = make_local_value_model(std::vector{1, 2, 3});
    auto model = ArrayValueToListModel(source);
    model.transact([&] {
      model.set(0, 10);
      model.transact([&] { model.set(2, 30); });
      REQUIRE(source->get() == std::vector{1, 2, 3});
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
