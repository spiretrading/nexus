#include <doctest/doctest.h>
#include "Spire/Spire/TableModel.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("TableModel") {
  TEST_CASE("visit_all_operations") {
    auto start_count = 0;
    auto end_count = 0;
    auto add_count = 0;
    auto move_count = 0;
    auto remove_count = 0;
    auto update_count = 0;
    auto visitor = [&] (const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::StartTransaction&) {
          ++start_count;
        },
        [&] (const TableModel::EndTransaction&) {
          ++end_count;
        },
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
    visitor(TableModel::StartTransaction());
    REQUIRE(start_count == 1);
    REQUIRE(end_count == 0);
    REQUIRE(add_count == 0);
    REQUIRE(move_count == 0);
    REQUIRE(remove_count == 0);
    REQUIRE(update_count == 0);
    visitor(TableModel::AddOperation(0, nullptr));
    REQUIRE(start_count == 1);
    REQUIRE(end_count == 0);
    REQUIRE(add_count == 1);
    REQUIRE(move_count == 0);
    REQUIRE(remove_count == 0);
    REQUIRE(update_count == 0);
    visitor(TableModel::MoveOperation(0, 1));
    REQUIRE(start_count == 1);
    REQUIRE(end_count == 0);
    REQUIRE(add_count == 1);
    REQUIRE(move_count == 1);
    REQUIRE(remove_count == 0);
    REQUIRE(update_count == 0);
    visitor(TableModel::RemoveOperation(1, nullptr));
    REQUIRE(start_count == 1);
    REQUIRE(end_count == 0);
    REQUIRE(add_count == 1);
    REQUIRE(move_count == 1);
    REQUIRE(remove_count == 1);
    REQUIRE(update_count == 0);
    visitor(TableModel::UpdateOperation(0, 0, 0, 0));
    REQUIRE(start_count == 1);
    REQUIRE(end_count == 0);
    REQUIRE(add_count == 1);
    REQUIRE(move_count == 1);
    REQUIRE(remove_count == 1);
    REQUIRE(update_count == 1);
    visitor(TableModel::EndTransaction());
    REQUIRE(start_count == 1);
    REQUIRE(end_count == 1);
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
    visitor(TableModel::AddOperation(0, nullptr));
    REQUIRE(move_count == 0);
    visitor(TableModel::MoveOperation(0, 1));
    REQUIRE(move_count == 1);
    visitor(TableModel::RemoveOperation(1, nullptr));
    REQUIRE(move_count == 1);
    visitor(TableModel::UpdateOperation(0, 0, 0, 0));
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
    visitor(TableModel::AddOperation(0, nullptr));
    REQUIRE(add_count == 1);
    REQUIRE(default_count == 0);
    visitor(TableModel::MoveOperation(0, 1));
    REQUIRE(add_count == 1);
    REQUIRE(default_count == 1);
    visitor(TableModel::RemoveOperation(1, nullptr));
    REQUIRE(add_count == 1);
    REQUIRE(default_count == 2);
    visitor(TableModel::UpdateOperation(0, 0, 0, 0));
    REQUIRE(add_count == 1);
    REQUIRE(default_count == 3);
  }
}
