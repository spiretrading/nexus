#include <boost/variant/get.hpp>
#include <doctest/doctest.h>
#include "Spire/Ui/TableModel.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("TableModel") {
  TEST_CASE("visit_all_operations") {
    auto visitor = [&] (const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& add_operation) {
          REQUIRE(add_operation.m_index == 0);
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
    };
    auto add_operation = TableModel::AddOperation{0};
    visitor(add_operation);
    auto move_operation = TableModel::MoveOperation{0, 1};
    visitor(move_operation);
    auto remove_operation = TableModel::RemoveOperation{1};
    visitor(remove_operation);
    auto update_operation = TableModel::UpdateOperation{0, 0};
    visitor(update_operation);
  }

  TEST_CASE("visit_partial_operations") {
    auto visitor = [&] (const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::MoveOperation& move_operation) {
          REQUIRE(move_operation.m_source == 0);
          REQUIRE(move_operation.m_destination == 1);
        });
    };
    auto add_operation = TableModel::AddOperation{0};
    visitor(add_operation);
    auto move_operation = TableModel::MoveOperation{0, 1};
    visitor(move_operation);
    auto remove_operation = TableModel::RemoveOperation{1};
    visitor(remove_operation);
    auto update_operation = TableModel::UpdateOperation{0, 0};
    visitor(update_operation);
  }

  TEST_CASE("visit_default_operation") {
    auto default_operation_count = 0;
    auto visitor = [&] (const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& add_operation) {
          REQUIRE(add_operation.m_index == 0);
        },
        [&] (const auto& operation) {
          REQUIRE(true);
          ++default_operation_count;
        });
    };
    auto add_operation = TableModel::AddOperation{0};
    visitor(add_operation);
    auto move_operation = TableModel::MoveOperation{0, 1};
    visitor(move_operation);
    auto remove_operation = TableModel::RemoveOperation{1};
    visitor(remove_operation);
    auto update_operation = TableModel::UpdateOperation{0, 0};
    visitor(update_operation);
    REQUIRE(default_operation_count == 3);
  }

  TEST_CASE("visit_all_operations_in_transaction") {
    auto transaction = TableModel::Transaction();
    transaction.m_operations.push_back(TableModel::AddOperation{0});
    transaction.m_operations.push_back(TableModel::MoveOperation{0, 1});
    transaction.m_operations.push_back(TableModel::RemoveOperation{1});
    transaction.m_operations.push_back(TableModel::UpdateOperation{0, 0});
    visit(transaction,
      [&] (const TableModel::AddOperation& add_operation) {
        REQUIRE(add_operation.m_index == 0);
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
  }

  TEST_CASE("visit_partial_operations_in_stransaction") {
    auto transaction = TableModel::Transaction();
    transaction.m_operations.push_back(TableModel::AddOperation{0});
    transaction.m_operations.push_back(TableModel::MoveOperation{0, 1});
    transaction.m_operations.push_back(TableModel::RemoveOperation{1});
    transaction.m_operations.push_back(TableModel::UpdateOperation{0, 0});
    visit(transaction,
      [&] (const TableModel::RemoveOperation& remove_operation) {
        REQUIRE(remove_operation.m_index == 1);
      });
  }

  TEST_CASE("visit_default_operation_in_stransaction") {
    auto transaction = TableModel::Transaction();
    transaction.m_operations.push_back(TableModel::AddOperation{0});
    transaction.m_operations.push_back(TableModel::MoveOperation{0, 1});
    transaction.m_operations.push_back(TableModel::RemoveOperation{1});
    transaction.m_operations.push_back(TableModel::UpdateOperation{0, 0});
    auto default_operation_count = 0;
    visit(transaction,
      [&] (const TableModel::RemoveOperation& remove_operation) {
        REQUIRE(remove_operation.m_index == 1);
      },
      [&] (const auto& operation) {
        REQUIRE(true);
        ++default_operation_count;
      });
    REQUIRE(default_operation_count == 3);
  }

  TEST_CASE("visit_all_operations_in_transaction_with_one_operation") {
    auto transaction = TableModel::Transaction();
    transaction.m_operations.push_back(TableModel::AddOperation{0});
    visit(transaction,
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
  }

  TEST_CASE("visit_default_operation_in_transaction_with_one_operation") {
    auto transaction = TableModel::Transaction();
    transaction.m_operations.push_back(TableModel::AddOperation{0});
    visit(transaction,
      [&] (const auto& operation) {
        auto add_operation = get<TableModel::AddOperation>(&operation);
        REQUIRE(add_operation != nullptr);
        REQUIRE(add_operation->m_index == 0);
      });
  }
}
