#include <doctest/doctest.h>
#include "Spire/Ui/TableModel.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("TableModel") {
  TEST_CASE("visit_all_operations") {
    auto add_count = 0;
    auto move_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    auto visitor = [&] (const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& add_operation) {
          ++add_count;
          REQUIRE(add_operation.m_index == 0);
        },
        [&] (const TableModel::MoveOperation& move_operation) {
          ++move_count;
          REQUIRE(move_operation.m_source == 0);
          REQUIRE(move_operation.m_destination == 1);
        },
        [&] (const TableModel::RemoveOperation& remove_operation) {
          ++remove_count;
          REQUIRE(remove_operation.m_index == 1);
        },
        [&] (const TableModel::UpdateOperation& update_operation) {
          ++update_count;
          REQUIRE(update_operation.m_row == 0);
          REQUIRE(update_operation.m_column == 0);
        });
    };
    auto add_operation = TableModel::AddOperation(0, nullptr);
    visitor(add_operation);
    REQUIRE(add_count == 1);
    REQUIRE(move_count == 0);
    REQUIRE(remove_count == 0);
    REQUIRE(update_count == 0);
    auto move_operation = TableModel::MoveOperation(0, 1);
    visitor(move_operation);
    REQUIRE(add_count == 1);
    REQUIRE(move_count == 1);
    REQUIRE(remove_count == 0);
    REQUIRE(update_count == 0);
    auto remove_operation = TableModel::RemoveOperation(1, nullptr);
    visitor(remove_operation);
    REQUIRE(add_count == 1);
    REQUIRE(move_count == 1);
    REQUIRE(remove_count == 1);
    REQUIRE(update_count == 0);
    auto update_operation = TableModel::UpdateOperation(0, 0, 0, 0);
    visitor(update_operation);
    REQUIRE(add_count == 1);
    REQUIRE(move_count == 1);
    REQUIRE(remove_count == 1);
    REQUIRE(update_count == 1);
  }

  TEST_CASE("visit_partial_operations") {
    auto move_count = 0;
    auto visitor = [&] (const TableModel::Operation& operation) {
      visit(operation, [&] (const TableModel::MoveOperation& move_operation) {
        ++move_count;
        REQUIRE(move_operation.m_source == 0);
        REQUIRE(move_operation.m_destination == 1);
      });
    };
    auto add_operation = TableModel::AddOperation(0, nullptr);
    visitor(add_operation);
    REQUIRE(move_count == 0);
    auto move_operation = TableModel::MoveOperation(0, 1);
    visitor(move_operation);
    REQUIRE(move_count == 1);
    auto remove_operation = TableModel::RemoveOperation(1, nullptr);
    visitor(remove_operation);
    REQUIRE(move_count == 1);
    auto update_operation = TableModel::UpdateOperation(0, 0, 0, 0);
    visitor(update_operation);
    REQUIRE(move_count == 1);
  }

  TEST_CASE("visit_default_operation") {
    auto add_count = 0;
    auto default_count = 0;
    auto visitor = [&] (const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& add_operation) {
          ++add_count;
          REQUIRE(add_operation.m_index == 0);
        },
        [&] (const auto& operation) {
          ++default_count;
        });
    };
    auto add_operation = TableModel::AddOperation(0, nullptr);
    visitor(add_operation);
    REQUIRE(add_count == 1);
    REQUIRE(default_count == 0);
    auto move_operation = TableModel::MoveOperation(0, 1);
    visitor(move_operation);
    REQUIRE(add_count == 1);
    REQUIRE(default_count == 1);
    auto remove_operation = TableModel::RemoveOperation(1, nullptr);
    visitor(remove_operation);
    REQUIRE(add_count == 1);
    REQUIRE(default_count == 2);
    auto update_operation = TableModel::UpdateOperation(0, 0, 0, 0);
    visitor(update_operation);
    REQUIRE(add_count == 1);
    REQUIRE(default_count == 3);
  }

  TEST_CASE("visit_all_operations_in_transaction") {
    auto index = 0;
    auto add_count = 0;
    auto move_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    auto transaction = TableModel::Transaction();
    transaction.push_back(TableModel::AddOperation(0, nullptr));
    transaction.push_back(TableModel::MoveOperation(0, 1));
    transaction.push_back(TableModel::RemoveOperation(1, nullptr));
    transaction.push_back(TableModel::UpdateOperation(0, 0, 0, 0));
    visit(TableModel::Operation(transaction),
      [&] (const TableModel::AddOperation& add_operation) {
        ++add_count;
        REQUIRE(index == 0);
        REQUIRE(add_operation.m_index == 0);
        ++index;
      },
      [&] (const TableModel::MoveOperation& move_operation) {
        ++move_count;
        REQUIRE(index == 1);
        REQUIRE(move_operation.m_source == 0);
        REQUIRE(move_operation.m_destination == 1);
        ++index;
      },
      [&] (const TableModel::RemoveOperation& remove_operation) {
        ++remove_count;
        REQUIRE(index == 2);
        REQUIRE(remove_operation.m_index == 1);
        ++index;
      },
      [&] (const TableModel::UpdateOperation& update_operation) {
        ++update_count;
        REQUIRE(index == 3);
        REQUIRE(update_operation.m_row == 0);
        REQUIRE(update_operation.m_column == 0);
        ++index;
      });
    REQUIRE(add_count == 1);
    REQUIRE(move_count == 1);
    REQUIRE(remove_count == 1);
    REQUIRE(update_count == 1);
  }

  TEST_CASE("visit_partial_operations_in_transaction") {
    auto remove_count = 0;
    auto transaction = TableModel::Transaction();
    transaction.push_back(TableModel::AddOperation(0, nullptr));
    transaction.push_back(TableModel::MoveOperation(0, 1));
    transaction.push_back(TableModel::RemoveOperation(1, nullptr));
    transaction.push_back(TableModel::UpdateOperation(0, 0, 0, 0));
    transaction.push_back(TableModel::RemoveOperation(2, nullptr));
    visit(TableModel::Operation(transaction),
      [&] (const TableModel::RemoveOperation& remove_operation) {
        ++remove_count;
        REQUIRE(remove_operation.m_index == remove_count);
      });
    REQUIRE(remove_count == 2);
  }

  TEST_CASE("visit_default_operation_in_transaction") {
    auto remove_count = 0;
    auto default_count = 0;
    auto transaction = TableModel::Transaction();
    transaction.push_back(TableModel::AddOperation(0, nullptr));
    transaction.push_back(TableModel::MoveOperation(0, 1));
    transaction.push_back(TableModel::RemoveOperation(1, nullptr));
    transaction.push_back(TableModel::UpdateOperation(0, 0, 0, 0));
    visit(TableModel::Operation(transaction),
      [&] (const TableModel::RemoveOperation& remove_operation) {
        ++remove_count;
        REQUIRE(remove_operation.m_index == 1);
      },
      [&] (const auto& operation) {
        ++default_count;
      });
    REQUIRE(remove_count == 1);
    REQUIRE(default_count == 3);
  }

  TEST_CASE("visit_all_operations_in_transaction_with_one_operation") {
    auto add_count = 0;
    auto transaction = TableModel::Transaction();
    transaction.push_back(TableModel::AddOperation(0, nullptr));
    visit(TableModel::Operation(transaction),
      [&] (const TableModel::AddOperation& add_operation) {
        ++add_count;
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
    REQUIRE(add_count == 1);
  }
}
