#include <boost/variant/get.hpp>
#include <doctest/doctest.h>
#include "Spire/Ui/ArrayTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("TableModel") {
  TEST_CASE("visit_all_operations") {
    auto model = ArrayTableModel();
    auto index = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        model.visit(operation,
          [&] (const TableModel::AddOperation& add_operation) {
            REQUIRE(add_operation.m_index == index);
          },
          [&] (const TableModel::MoveOperation& move_operation) {
            REQUIRE(move_operation.m_source == 0);
            REQUIRE(move_operation.m_destination == 1);
          },
          [&] (const TableModel::RemoveOperation& remove_operation) {
            REQUIRE(remove_operation.m_index == index);
          },
          [&] (const TableModel::UpdateOperation& update_operation) {
            REQUIRE(update_operation.m_row == 0);
            REQUIRE(update_operation.m_column == 0);
          });
      }));
    index = 0;
    REQUIRE_NOTHROW(model.push({1, 2}));
    index = 1;
    REQUIRE_NOTHROW(model.insert({3, 4}, index));
    REQUIRE_NOTHROW(model.move(0, 1));
    REQUIRE_NOTHROW(model.set(0, 0, 0));
    index = 1;
    REQUIRE_NOTHROW(model.remove(index));
    index = 0;
    REQUIRE_NOTHROW(model.remove(index));
  }

  TEST_CASE("visit_partial_operations") {
    auto model = ArrayTableModel();
    auto index = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        model.visit(operation,
          [&] (const TableModel::AddOperation& add_operation) {
            REQUIRE(add_operation.m_index == index);
          },
          [&] (const TableModel::RemoveOperation& remove_operation) {
            REQUIRE(remove_operation.m_index == index);
          });
      }));
    index = 0;
    REQUIRE_NOTHROW(model.push({1, 2}));
    index = 1;
    REQUIRE_NOTHROW(model.insert({3, 4}, index));
    REQUIRE_NOTHROW(model.move(0, 1));
    REQUIRE_NOTHROW(model.set(0, 0, 0));
    index = 1;
    REQUIRE_NOTHROW(model.remove(index));
    index = 0;
    REQUIRE_NOTHROW(model.remove(index));
  }

  TEST_CASE("visit_default_operation") {
    auto model = ArrayTableModel();
    auto index = 0;
    auto operation_count = 0;
    auto default_operation_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& operation) {
        model.visit(operation,
          [&] (const TableModel::AddOperation& add_operation) {
            REQUIRE(add_operation.m_index == index);
          },
          [&] (const auto& operation) {
            REQUIRE(true);
            ++default_operation_count;
          });
      }));
    index = 0;
    ++operation_count;
    REQUIRE_NOTHROW(model.push({1, 2}));
    index = 1;
    ++operation_count;
    REQUIRE_NOTHROW(model.insert({3, 4}, index));
    ++operation_count;
    REQUIRE_NOTHROW(model.move(0, 1));
    ++operation_count;
    REQUIRE_NOTHROW(model.set(0, 0, 0));
    index = 1;
    ++operation_count;
    REQUIRE_NOTHROW(model.remove(index));
    index = 0;
    ++operation_count;
    REQUIRE_NOTHROW(model.remove(index));
    REQUIRE(operation_count - 2 == default_operation_count);
  }

  TEST_CASE("visit_all_operations_in_transaction") {
    auto model = ArrayTableModel();
    model.push({1, 2, 3});
    model.push({4, 5, 6});
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& transaction) {
        model.visit(transaction,
          [&] (const TableModel::AddOperation& add_operation) {
            REQUIRE(add_operation.m_index == 1);
          },
          [&] (const TableModel::MoveOperation& move_operation) {
            REQUIRE(move_operation.m_source == 0);
            REQUIRE(move_operation.m_destination == 1);
          },
          [&] (const TableModel::RemoveOperation& remove_operation) {
            REQUIRE(remove_operation.m_index == 1);
          },
          [&] (const TableModel::UpdateOperation& update_operation) {
            REQUIRE(update_operation.m_row == 0);
            REQUIRE(update_operation.m_column == 0);
          });
      }));
    model.transact([&] {
      model.insert({9, 0, 2}, 1);
      model.transact([&] {
        model.set(0, 0, 10);
        model.transact([&] {
          model.remove(1);
        });
        model.move(0, 1);
      });
    });
  }

  TEST_CASE("visit_partial_operations_in_stransaction") {
    auto model = ArrayTableModel();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& transaction) {
        model.visit(transaction,
          [&] (const TableModel::RemoveOperation& remove_operation) {
            REQUIRE(remove_operation.m_index == 1);
          });
      }));
    model.push({1, 2, 3});
    model.push({4, 5, 6});
    model.transact([&] {
      model.insert({9, 0, 2}, 1);
      model.transact([&] {
        model.set(0, 0, 10);
        model.transact([&] {
          model.push({7, 8, 9});
          model.remove(1);
        });
        model.move(0, 1);
        model.set(1, 1, 0);
      });
    });
  }

  TEST_CASE("visit_default_operation_in_stransaction") {
    auto model = ArrayTableModel();
    model.push({1, 2, 3});
    model.push({4, 5, 6});
    auto operation_count = 0;
    auto default_operation_count = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& transaction) {
        model.visit(transaction,
          [&] (const TableModel::RemoveOperation& remove_operation) {
            REQUIRE(remove_operation.m_index == 1);
          },
          [&] (const auto& operation) {
            REQUIRE(true);
            ++default_operation_count;
          });
      }));
    model.transact([&] {
      ++operation_count;
      model.insert({9, 0, 2}, 1);
      model.transact([&] {
        ++operation_count;
        model.set(0, 0, 10);
        model.transact([&] {
          ++operation_count;
          model.remove(1);
          ++operation_count;
          model.set(1, 1, 0);
        });
        ++operation_count;
        model.move(0, 1);
      });
    });
    REQUIRE(operation_count - 1 == default_operation_count);
  }

  TEST_CASE("visit_all_operations_in_transaction_with_one_operation") {
    auto model = ArrayTableModel();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& transaction) {
        model.visit(transaction,
          [&] (const TableModel::AddOperation& add_operation) {
            REQUIRE(add_operation.m_index == 0);
          },
          [&] (const TableModel::MoveOperation& move_operation) {
            REQUIRE(false);
          },
          [&] (const TableModel::RemoveOperation& remove_operation) {
            REQUIRE(false);
          },
          [&] (const TableModel::UpdateOperation& update_operation) {
            REQUIRE(false);
          });
      }));
    model.transact([&] {
      model.push({1, 2, 3});
    });
  }

  TEST_CASE("visit_partial_operations_in_transaction_with_one_operation") {
    auto model = ArrayTableModel();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& transaction) {
        model.visit(transaction,
          [&] (const TableModel::AddOperation& add_operation) {
            REQUIRE(add_operation.m_index == 0);
          });
      }));
    model.transact([&] {
      model.push({1, 2, 3});
    });
  }

  TEST_CASE("visit_default_operation_in_transaction_with_one_operation") {
    auto model = ArrayTableModel();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const TableModel::Operation& transaction) {
        model.visit(transaction,
          [&] (const auto& operation) {
            auto add_operation = get<TableModel::AddOperation>(&operation);
            REQUIRE(add_operation != nullptr);
            REQUIRE(add_operation->m_index == 0);
          });
      }));
    model.transact([&] {
      model.push({1, 2, 3});
    });
  }
}
