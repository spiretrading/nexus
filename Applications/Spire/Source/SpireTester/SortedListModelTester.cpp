#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/SortedListModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  template<typename T>
  auto span(const ListModel<T>& model) {
    auto span = std::vector<T>();
    for(auto i = 0; i != model.get_size(); ++i) {
      span.push_back(model.get(i));
    }
    return span;
  }

  bool reverse_comparator(int left, int right) {
    return !std::less<int>()(left, right);
  }
}

TEST_SUITE("SortedListModel") {
  TEST_CASE("sort") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto sorted_model = SortedListModel(source);
    REQUIRE(span(sorted_model) == std::vector{1, 2, 4, 9});
  }

  TEST_CASE("sort_constructor_with_comparator") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(1);
    auto sorted_model = SortedListModel(source, &reverse_comparator);
    REQUIRE(span(sorted_model) == std::vector{9, 4, 2, 1});
  }

  TEST_CASE("push") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(2);
    source->push(6);
    source->push(9);
    auto sorted_model = SortedListModel(source);
    auto operations = std::deque<ListModel<int>::Operation>();
    sorted_model.connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); });
    REQUIRE(span(sorted_model) == std::vector{2, 2, 4, 6, 9, 9});
    source->push(2);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::AddOperation(0)
      });
    operations.clear();
    REQUIRE(span(sorted_model) == std::vector{2, 2, 2, 4, 6, 9, 9});
    source->push(7);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::AddOperation(5)
      });
    operations.clear();
    REQUIRE(span(sorted_model) == std::vector{2, 2, 2, 4, 6, 7, 9, 9});
  }

  TEST_CASE("insert") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(2);
    source->push(6);
    source->push(9);
    auto sorted_model = SortedListModel(source);
    auto operations = std::deque<ListModel<int>::Operation>();
    sorted_model.connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); });
    REQUIRE(span(sorted_model) == std::vector{2, 2, 4, 6, 9, 9});
    source->insert(2, 4);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::AddOperation(0)
      });
    operations.clear();
    REQUIRE(span(sorted_model) == std::vector{2, 2, 2, 4, 6, 9, 9});
    source->insert(7, 0);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::AddOperation(5)
      });
    operations.clear();
    REQUIRE(span(sorted_model) == std::vector{2, 2, 2, 4, 6, 7, 9, 9});
  }

  TEST_CASE("remove") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(2);
    source->push(6);
    source->push(9);
    auto sorted_model = SortedListModel(source);
    auto operations = std::deque<ListModel<int>::Operation>();
    sorted_model.connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); });
    REQUIRE(span(sorted_model) == std::vector{2, 2, 4, 6, 9, 9});
    source->remove(2);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::PreRemoveOperation(4),
        ListModel<int>::RemoveOperation(4)
      });
    operations.clear();
    REQUIRE(span(sorted_model) == std::vector{2, 2, 4, 6, 9});
  }

  TEST_CASE("update") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(2);
    source->push(6);
    source->push(9);
    auto sorted_model = SortedListModel(source);
    auto operations = std::deque<ListModel<int>::Operation>();
    sorted_model.connect_operation_signal(
      [&] (const auto& operation) {
        visit(operation, [&] (const auto& operation) {
          operations.push_back(operation);
        });
      });
    REQUIRE(span(sorted_model) == std::vector{2, 2, 4, 6, 9, 9});
    source->set(2, 0);
    require_list_transaction<int>(operations,
      {
        ListModel<int>::MoveOperation(4, 0),
        ListModel<int>::UpdateOperation(0, 0, 0)
      });
    operations.clear();
    REQUIRE(span(sorted_model) == std::vector{0, 2, 2, 4, 6, 9});
  }

  TEST_CASE("multiple_operations") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(6);
    auto sorted_model = SortedListModel(source);
    auto signal_count = 0;
    sorted_model.connect_operation_signal(
      [&] (const auto& operation) { ++signal_count; });
    source->push(2);
    source->set(3, 1);
    source->insert(6, 2);
    REQUIRE(signal_count == 6);
    REQUIRE(span(sorted_model) == std::vector{9, 6, 4, 2, 2, 1});
  }

  TEST_CASE("transaction") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(4);
    source->push(2);
    source->push(9);
    source->push(6);
    auto sorted_model = SortedListModel(source);
    source->transact([&] {
      source->insert(5, 1);
      source->insert(3, 1);
    });
    REQUIRE(span(sorted_model) == std::vector{2, 3, 4, 5, 6, 9});
  }
}
