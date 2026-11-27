#include <doctest/doctest.h>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/ReadOnlyTableModel.hpp"
#include "Spire/SpireTester/TableModelTester.hpp"

using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("ReadOnlyTableModel") {
  TEST_CASE("read_only_cells") {
    struct Source : ArrayTableModel {
      AnyRef at(int row, int column) const override {
        auto cell = ArrayTableModel::at(row, column);
        return *any_cast<const std::shared_ptr<int>>(cell);
      }
    };
    auto source = std::make_shared<Source>();
    source->push({std::make_shared<int>(42)});
    REQUIRE_FALSE(source->at(0, 0).is_const());
    auto table = ReadOnlyTableModel(source);
    auto cell = table.at(0, 0);
    REQUIRE(cell.is_const());
    REQUIRE_THROWS_AS(cell.assign(7), std::bad_any_cast);
    REQUIRE(table.set(0, 0, 7) == QValidator::Invalid);
    REQUIRE(table.remove(0) == QValidator::Invalid);
    REQUIRE(source->get<int>(0, 0) == 42);
  }

  TEST_CASE("source_operations") {
    auto source = std::make_shared<ArrayTableModel>();
    auto table = ReadOnlyTableModel(source);
    auto operations = std::deque<TableModel::Operation>();
    auto connection = scoped_connection(table.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    source->transact([&] {
      source->push({1, 2});
      source->push({3, 4});
      source->set(0, 1, 5);
      source->move(0, 1);
      source->remove(0);
    });
    require_transaction(operations, {TableModel::AddOperation(0),
      TableModel::AddOperation(1), TableModel::UpdateOperation(0, 1, 2, 5),
      TableModel::MoveOperation(0, 1), TableModel::PreRemoveOperation(0),
      TableModel::RemoveOperation(0)});
    REQUIRE(table.get_row_size() == 1);
    REQUIRE(table.get_column_size() == 2);
    REQUIRE(table.get<int>(0, 1) == 5);
  }

  TEST_CASE("connection_lifetime") {
    auto source = std::make_shared<ArrayTableModel>();
    auto calls = 0;
    auto subscription = scoped_connection();
    {
      auto table = ReadOnlyTableModel(source);
      subscription = table.connect_operation_signal([&] (const auto&) {
        ++calls;
      });
      source->push({1});
      REQUIRE(calls == 1);
    }
    source->push({2});
    REQUIRE(calls == 1);
    REQUIRE_FALSE(subscription.connected());
  }
}
