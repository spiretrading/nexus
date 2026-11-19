#include <algorithm>
#include <stdexcept>
#include <vector>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayValueToListModel.hpp"
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/Spire/FilteredListModel.hpp"
#include "Spire/Spire/ListCurrentIndexModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Spire;

namespace {
  struct NonzeroValues : LocalValueModel<std::vector<int>> {
    using LocalValueModel::LocalValueModel;

    QValidator::State set(const Type& value) override {
      if(value.empty()) {
        throw std::runtime_error("At least one value is required.");
      } else if(std::ranges::find(value, 0) != value.end()) {
        return QValidator::Invalid;
      }
      return LocalValueModel::set(value);
    }
  };
}

TEST_SUITE("ArrayValueToListModel") {
  TEST_CASE("rejected_source_write") {
    auto source = std::make_shared<NonzeroValues>(std::vector{1});
    auto model = std::make_shared<ArrayValueToListModel<int>>(source);
    auto filtered = FilteredListModel(model, [] (const auto&, int) {
      return false;
    });
    auto operations = 0;
    model->connect_operation_signal([&] (const auto&) { ++operations; });
    SUBCASE("remove") {
      REQUIRE_THROWS_AS(model->remove(0), std::runtime_error);
      REQUIRE(operations == 0);
    }
    SUBCASE("insert") {
      REQUIRE(model->insert(0, 0) == QValidator::Invalid);
      REQUIRE(operations == 0);
    }
    SUBCASE("update") {
      REQUIRE(model->set(0, 0) == QValidator::Invalid);
      REQUIRE(operations == 0);
    }
    SUBCASE("transaction") {
      model->transact([&] { model->push(0); });
    }
    SUBCASE("throwing_transaction") {
      REQUIRE_THROWS_AS(model->transact([&] { model->remove(0); }),
        std::runtime_error);
    }
    REQUIRE(model->get_size() == 1);
    REQUIRE(model->get(0) == 1);
    REQUIRE(filtered.get_size() == 1);
    REQUIRE(filtered.get(0) == 1);
    REQUIRE(model->push(2) == QValidator::Acceptable);
    REQUIRE(source->get() == std::vector{1, 2});
    REQUIRE(filtered.get(1) == 2);
  }

  TEST_CASE("unaffected_rows_after_rejected_transaction") {
    auto source = make_constant_value_model(std::vector{1, 2, 2, 3});
    auto model = std::make_shared<ArrayValueToListModel<int>>(source);
    auto current = ListCurrentIndexModel(model, 2);
    model->transact([&] {
      SUBCASE("update") {
        model->set(0, 0);
        model->set(3, 4);
      }
      SUBCASE("structural_edits") {
        model->insert(2, 1);
        model->remove(0);
        model->transact([&] { model->insert(0, 0); });
      }
      SUBCASE("equal_values") {
        model->remove(1);
        model->insert(2, 2);
      }
    });
    REQUIRE(model->get_size() == 4);
    for(auto i = 0; i != model->get_size(); ++i) {
      REQUIRE(model->get(i) == source->get()[i]);
    }
    REQUIRE(current.get().has_value());
    REQUIRE(*current.get() == 2);
  }

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
