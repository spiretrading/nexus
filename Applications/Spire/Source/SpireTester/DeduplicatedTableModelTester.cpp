#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/DeduplicatedTableModel.hpp"
#include "Spire/SpireTester/TableModelTester.hpp"

using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("DeduplicatedTableModel") {
  TEST_CASE("update") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2});
    auto model = DeduplicatedTableModel(source);
    auto operations = std::deque<TableModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    source->set(0, 0, 1);
    REQUIRE(operations.empty());
    REQUIRE(model.get<int>(0, 0) == 1);
    source->set(0, 0, 5);
    require_transaction(operations, {
      TableModel::UpdateOperation(0, 0, 1, 5)
    });
    operations.clear();
    REQUIRE(model.get<int>(0, 0) == 5);
    source->set(0, 1, 2);
    REQUIRE(operations.empty());
  }

  TEST_CASE("structural_operations") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1});
    auto model = DeduplicatedTableModel(source);
    auto operations = std::deque<TableModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    source->push({2});
    require_transaction(operations, {
      TableModel::AddOperation(1)
    });
    operations.clear();
    source->move(1, 0);
    require_transaction(operations, {
      TableModel::MoveOperation(1, 0)
    });
    operations.clear();
    REQUIRE(model.get_row_size() == 2);
    source->remove(0);
    require_transaction(operations, {
      TableModel::PreRemoveOperation(0),
      TableModel::RemoveOperation(0)
    });
    REQUIRE(model.get_row_size() == 1);
  }

  TEST_CASE("comparator") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1, 2});
    auto model = DeduplicatedTableModel(source,
      [] (const auto& previous, const auto& value, auto column) {
        return column == 1;
      });
    auto operations = std::deque<TableModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    source->set(0, 1, 9);
    REQUIRE(operations.empty());
    source->set(0, 0, 1);
    require_transaction(operations, {
      TableModel::UpdateOperation(0, 0, 1, 1)
    });
  }

  TEST_CASE("set") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({1});
    auto model = DeduplicatedTableModel(source);
    auto operations = std::deque<TableModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    REQUIRE(model.set(0, 0, 7) == QValidator::State::Acceptable);
    REQUIRE(source->get<int>(0, 0) == 7);
    require_transaction(operations, {
      TableModel::UpdateOperation(0, 0, 1, 7)
    });
    operations.clear();
    REQUIRE(model.set(0, 0, 7) == QValidator::State::Acceptable);
    REQUIRE(operations.empty());
  }
}
