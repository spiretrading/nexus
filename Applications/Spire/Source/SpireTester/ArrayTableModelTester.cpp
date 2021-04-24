#include <boost/variant/get.hpp>
#include <doctest/doctest.h>
#include "Spire/Ui/ArrayTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("ArrayTableModel") {
  TEST_CASE("push") {
    auto model = ArrayTableModel();
    REQUIRE(model.get_row_size() == 0);
    REQUIRE(model.get_column_size() == 0);
    auto signal_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = get<TableModel::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
      }));
    REQUIRE_NOTHROW(model.push({3, 1, 4}));
    REQUIRE(signal_count == 1);
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(model.get<int>(0, 0) == 3);
    REQUIRE(model.get<int>(0, 1) == 1);
    REQUIRE(model.get<int>(0, 2) == 4);
    REQUIRE_THROWS(model.push({2, 7}));
    REQUIRE(signal_count == 1);
    REQUIRE_NOTHROW(model.push({2, 7, 1}));
    REQUIRE(signal_count == 2);
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(model.get<int>(1, 0) == 2);
    REQUIRE(model.get<int>(1, 1) == 7);
    REQUIRE(model.get<int>(1, 2) == 1);
  }

  TEST_CASE("remove") {
    auto model = ArrayTableModel();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        REQUIRE(false);
      }));
    REQUIRE_THROWS(model.remove(0));
    auto signal_count = 0;
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto add_operation = get<TableModel::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == model.get_row_size() - 1);
      });
    REQUIRE_NOTHROW(model.push({1, 2}));
    REQUIRE(signal_count == 1);
    REQUIRE_NOTHROW(model.push({3, 4}));
    REQUIRE(signal_count == 2);
    REQUIRE_NOTHROW(model.push({5, 6}));
    REQUIRE(signal_count == 3);
    REQUIRE(model.get_row_size() == 3);
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto remove_operation = get<TableModel::RemoveOperation>(&operation);
        REQUIRE(remove_operation != nullptr);
        REQUIRE(remove_operation->m_index == 0);
      });
    REQUIRE_NOTHROW(model.remove(0));
    REQUIRE(signal_count == 4);
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(model.get<int>(0, 0) == 3);
    REQUIRE(model.get<int>(0, 1) == 4);
    REQUIRE(model.get<int>(1, 0) == 5);
    REQUIRE(model.get<int>(1, 1) == 6);
    connection = model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        REQUIRE(false);
      });
    REQUIRE_THROWS(model.remove(2));
  }

  TEST_CASE("subtransactions") {
    auto model = ArrayTableModel();
    auto signal_count = 0;
    model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        ++signal_count;
        auto transaction = get<TableModel::Transaction>(&operation);
        REQUIRE(transaction != nullptr);
        REQUIRE(transaction->m_operations.size() == 5);
        auto& operations = transaction->m_operations;
        REQUIRE(get<TableModel::AddOperation>(&operations[0]));
        REQUIRE(get<TableModel::UpdateOperation>(&operations[1]));
        REQUIRE(get<TableModel::AddOperation>(&operations[2]));
        REQUIRE(get<TableModel::RemoveOperation>(&operations[3]));
        REQUIRE(get<TableModel::AddOperation>(&operations[4]));
      });
    model.transact([&] {
      model.push({1, 2, 3});
      model.transact([&] {
        model.set(0, 0, 10);
        model.transact([&] {
          model.push({9, 0, 2});
          model.remove(1);
        });
        model.push({8, 9, 9});
      });
    });
    REQUIRE(signal_count == 1);
  }
}
