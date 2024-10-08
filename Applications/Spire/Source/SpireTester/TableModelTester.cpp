#include "Spire/SpireTester/TableModelTester.hpp"
#include <doctest/doctest.h>
#include "Spire/Spire/TableModel.hpp"

using namespace boost;
using namespace Spire;

namespace {
  void require_equal(const std::any& actual, const std::any& expected) {
    REQUIRE(actual.type() == expected.type());
    if(actual.type() == typeid(float)) {
      REQUIRE(std::any_cast<float>(actual) == std::any_cast<float>(expected));
    } else if(actual.type() == typeid(double)) {
      REQUIRE(std::any_cast<double>(actual) == std::any_cast<double>(expected));
    } else if(actual.type() == typeid(int)) {
      REQUIRE(std::any_cast<int>(actual) == std::any_cast<int>(expected));
    } else if(actual.type() == typeid(std::string)) {
      REQUIRE(std::any_cast<std::string>(actual) ==
        std::any_cast<std::string>(expected));
    } else {
      REQUIRE(false);
    }
  }
}

void Spire::require_transaction(
    const std::deque<TableModel::Operation>& operations,
    const std::vector<TableModel::Operation>& expected) {
  auto offset = 0;
  if(expected.size() == 1) {
    if(operations.size() != 1) {
      REQUIRE(operations.size() == 3);
      REQUIRE(get<TableModel::StartTransaction>(&operations[0]) != nullptr);
      offset = 1;
    }
  } else {
    if(get<TableModel::StartTransaction>(&operations[0]) == nullptr) {
      REQUIRE(operations.size() == 2);
      REQUIRE(get<TableModel::PreRemoveOperation>(&operations[0]) != nullptr);
    } else {
      REQUIRE(operations.size() == expected.size() + 2);
      offset = 1;
    }
  }
  for(auto i = 0; i != std::ssize(expected); ++i) {
    visit(expected[i],
      [&] (const TableModel::AddOperation& expected) {
        auto operation = get<TableModel::AddOperation>(&operations[i + offset]);
        REQUIRE(operation != nullptr);
        REQUIRE(operation->m_index == expected.m_index);
      },
      [&] (const TableModel::PreRemoveOperation& expected) {
        auto operation =
          get<TableModel::PreRemoveOperation>(&operations[i + offset]);
        REQUIRE(operation != nullptr);
        REQUIRE(operation->m_index == expected.m_index);
      },
      [&] (const TableModel::MoveOperation& expected) {
        auto operation =
          get<TableModel::MoveOperation>(&operations[i + offset]);
        REQUIRE(operation != nullptr);
        REQUIRE(operation->m_source == expected.m_source);
        REQUIRE(operation->m_destination == expected.m_destination);
      },
      [&] (const TableModel::UpdateOperation& expected) {
        auto operation =
          get<TableModel::UpdateOperation>(&operations[i + offset]);
        REQUIRE(operation != nullptr);
        REQUIRE(operation->m_row == expected.m_row);
        REQUIRE(operation->m_column == expected.m_column);
        require_equal(operation->m_previous, expected.m_previous);
        require_equal(operation->m_value, expected.m_value);
      });
  }
  if(offset != 0) {
    REQUIRE(get<TableModel::EndTransaction>(&operations.back()) != nullptr);
  }
}

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
        [&] (const TableModel::PreRemoveOperation& remove_operation) {
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
    visitor(TableModel::AddOperation(0));
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
    visitor(TableModel::PreRemoveOperation(1));
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
    visitor(TableModel::AddOperation(0));
    REQUIRE(move_count == 0);
    visitor(TableModel::MoveOperation(0, 1));
    REQUIRE(move_count == 1);
    visitor(TableModel::PreRemoveOperation(1));
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
    visitor(TableModel::AddOperation(0));
    REQUIRE(add_count == 1);
    REQUIRE(default_count == 0);
    visitor(TableModel::MoveOperation(0, 1));
    REQUIRE(add_count == 1);
    REQUIRE(default_count == 1);
    visitor(TableModel::PreRemoveOperation(1));
    REQUIRE(add_count == 1);
    REQUIRE(default_count == 2);
    visitor(TableModel::UpdateOperation(0, 0, 0, 0));
    REQUIRE(add_count == 1);
    REQUIRE(default_count == 3);
  }
}
