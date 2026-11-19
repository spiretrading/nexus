#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"
#include "Spire/Ui/ListSelectionValueModel.hpp"

using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("ListSelectionValueModel") {
  TEST_CASE("construct") {
    auto values = std::make_shared<ArrayListModel<std::string>>();
    values->push("hello");
    values->push("world");
    values->push("goodbye");
    values->push("sky");
    auto selection = std::make_shared<ArrayListModel<int>>();
    selection->push(2);
    auto selected_values =
      ListSelectionValueModel<std::string>(values, selection);
    REQUIRE(selected_values.get_size() == 1);
    REQUIRE(selected_values.get(0) == "goodbye");
    selection->push(0);
    REQUIRE(selected_values.get_size() == 2);
    REQUIRE(selected_values.get(0) == "goodbye");
    REQUIRE(selected_values.get(1) == "hello");
  }

  TEST_CASE("move") {
    using Model = ListModel<int>;
    auto values =
      std::make_shared<ArrayListModel<int>>(std::vector{10, 20, 30, 40});
    auto selection =
      std::make_shared<ArrayListModel<int>>(std::vector{0, 1, 2});
    auto selected = ListSelectionValueModel<int>(values, selection);
    auto operations = std::deque<Model::Operation>();
    selected.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    auto test_move = [&] (int source, int destination) {
      auto previous = selected.get(source);
      selection->move(source, destination);
      require_list_transaction<int>(
        operations, {Model::MoveOperation(source, destination)});
      operations.clear();
      selection->set(destination, 3);
      REQUIRE(operations.size() == 1);
      auto update = std::get_if<Model::UpdateOperation>(&operations.front());
      REQUIRE(update);
      REQUIRE(update->get_previous() == previous);
      REQUIRE(update->get_value() == 40);
    };
    SUBCASE("forward") {
      test_move(0, 2);
    }
    SUBCASE("backward") {
      test_move(2, 0);
    }
  }

  TEST_CASE("source_transaction") {
    using Model = ListModel<int>;
    auto values = std::make_shared<ArrayListModel<int>>(std::vector{10, 20});
    auto selection = std::make_shared<ArrayListModel<int>>();
    auto selected = ListSelectionValueModel<int>(values, selection);
    auto operations = std::deque<Model::Operation>();
    selected.connect_operation_signal([&] (const auto& operation) {
      operations.push_back(operation);
    });
    selection->transact([&] {
      selection->push(0);
      selection->push(1);
      selection->remove(0);
    });
    REQUIRE(operations.size() == 6);
    require_list_transaction<int>(operations, {Model::AddOperation(0),
      Model::AddOperation(1), Model::PreRemoveOperation(0),
      Model::RemoveOperation(0)});
    REQUIRE(selected.get_size() == 1);
    REQUIRE(selected.get(0) == 20);
  }
}
