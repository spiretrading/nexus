#include <doctest/doctest.h>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/GroupedTableModel.hpp"

using namespace Spire;

namespace {
  auto make_positions() {
    auto positions = std::make_shared<ArrayTableModel>();
    positions->push({QString("A"), QString("CAD"), 10});
    positions->push({QString("B"), QString("CAD"), 20});
    positions->push({QString("A"), QString("USD"), 30});
    return positions;
  }

  auto copy_row(const TableModel& table, int row) {
    auto values = std::vector<std::any>();
    for(auto column = 0; column != table.get_column_size(); ++column) {
      values.push_back(to_any(table.at(row, column)));
    }
    return values;
  }

  class SumOfSquares : public TableAggregation {
    public:
      void add(const AnyRef& value) override {
        auto number = any_cast<int>(value);
        m_sum += number * number;
      }

      void remove(const AnyRef& value) override {
        auto number = any_cast<int>(value);
        m_sum -= number * number;
      }

      void update(const AnyRef& previous, const AnyRef& current) override {
        remove(previous);
        add(current);
      }

      std::any get() const override {
        return m_sum;
      }

    private:
      int m_sum = 0;
  };
}

TEST_SUITE("GroupedTableModel") {
  TEST_CASE("source_order_without_groups") {
    auto source = make_positions();
    auto model = GroupedTableModel(source, {});
    auto moves = 0;
    model.connect_operation_signal([&] (const TableModel::Operation& op) {
      if(auto move = std::get_if<TableModel::MoveOperation>(&op)) {
        REQUIRE(move->m_source == 0);
        REQUIRE(move->m_destination == 2);
        ++moves;
      }
    });
    source->move(0, 2);
    REQUIRE(moves == 1);
    REQUIRE(model.get_row_size() == 3);
    REQUIRE(model.index_from_source(2) == 2);
    REQUIRE(model.index_to_source(2) == 2);
    REQUIRE(model.get<int>(2, 2) == 10);
    REQUIRE(model.set(2, 2, 50) == QValidator::Acceptable);
    REQUIRE(source->get<int>(2, 2) == 50);
  }

  TEST_CASE("group_membership_notifications") {
    auto source = make_positions();
    auto model = GroupedTableModel(source, {0});
    REQUIRE(model.get_row_size() == 5);
    REQUIRE(model.get<QString>(0, 0) == "A (2)");
    REQUIRE(model.get<QString>(0, 1) == "(2)");
    REQUIRE(model.get<std::int64_t>(0, 2) == 40);
    REQUIRE(model.get<QString>(3, 0) == "B (1)");
    REQUIRE(model.remove(0) == QValidator::Invalid);
    REQUIRE(model.set(0, 0, QString("C")) == QValidator::Invalid);
    auto rows = std::vector<std::vector<std::any>>();
    for(auto row = 0; row != model.get_row_size(); ++row) {
      rows.push_back(copy_row(model, row));
    }
    auto commits = 0;
    auto opened = false;
    auto removing = -1;
    auto moves = 0;
    model.connect_operation_signal([&] (const TableModel::Operation& op) {
      visit(op,
        [&] (const TableModel::StartTransaction&) {
          REQUIRE_FALSE(opened);
          opened = true;
        },
        [&] (const TableModel::AddOperation& add) {
          rows.insert(rows.begin() + add.m_index, copy_row(model, add.m_index));
        },
        [&] (const TableModel::PreRemoveOperation& removal) {
          REQUIRE(removing == -1);
          REQUIRE(removal.m_index < model.get_row_size());
          removing = removal.m_index;
        },
        [&] (const TableModel::RemoveOperation& removal) {
          REQUIRE(removing == removal.m_index);
          rows.erase(rows.begin() + removing);
          removing = -1;
        },
        [&] (const TableModel::MoveOperation& move) {
          auto row = std::move(rows.at(move.m_source));
          rows.erase(rows.begin() + move.m_source);
          rows.insert(rows.begin() + move.m_destination, std::move(row));
          ++moves;
        },
        [&] (const TableModel::UpdateOperation& update) {
          auto& cell = rows.at(update.m_row).at(update.m_column);
          if(cell.has_value() || update.m_previous.has_value()) {
            REQUIRE(is_equal(cell, update.m_previous));
          }
          cell = update.m_value;
        },
        [&] (const TableModel::EndTransaction&) {
          REQUIRE(opened);
          opened = false;
          ++commits;
          REQUIRE(std::ssize(rows) == model.get_row_size());
          for(auto row = 0; row != model.get_row_size(); ++row) {
            for(auto column = 0; column != model.get_column_size(); ++column) {
              REQUIRE(is_equal(rows[row][column], model.at(row, column)));
            }
            auto index = model.index_to_source(row);
            if(index != -1) {
              REQUIRE(model.index_from_source(index) == row);
            }
          }
        });
    });
    source->transact([&] {
      source->set(0, 2, 12);
      source->set(0, 0, QString("B"));
      source->set(0, 2, 15);
    });
    REQUIRE(commits == 1);
    REQUIRE(model.get<QString>(0, 0) == "A (1)");
    REQUIRE(model.get<std::int64_t>(0, 2) == 30);
    REQUIRE(model.get<QString>(2, 0) == "B (2)");
    REQUIRE(model.get<std::int64_t>(2, 2) == 35);
    model.set(model.index_from_source(2), 0, QString("B"));
    REQUIRE(model.get_row_size() == 4);
    REQUIRE(model.get<QString>(0, 0) == "B (3)");
    REQUIRE(model.get<std::int64_t>(0, 2) == 65);
    source->transact([&] {
      source->set(2, 2, 40);
      source->move(2, 0);
      source->set(1, 2, 5);
      source->insert({QString("C"), QString("USD"), 18}, 0);
    });
    REQUIRE(moves == 1);
    REQUIRE(model.get<std::int64_t>(0, 2) == 65);
    REQUIRE(model.get<int>(1, 2) == 40);
    REQUIRE(model.get<int>(2, 2) == 5);
    REQUIRE(model.get<QString>(4, 0) == "C (1)");
    REQUIRE(model.remove(5) == QValidator::Acceptable);
    REQUIRE(model.get_row_size() == 4);
    REQUIRE(model.index_from_source(0) == 1);
    REQUIRE(commits == 4);
  }

  TEST_CASE("nested_groups_and_custom_aggregation") {
    auto source = std::make_shared<ArrayTableModel>();
    auto factories = std::vector<TableAggregationFactory>(3);
    factories[2] = [] {
      return std::make_unique<SumOfSquares>();
    };
    auto model = GroupedTableModel(source, {1, 0}, std::move(factories));
    source->push({QString("A"), QString("CAD"), 2});
    source->push({QString("A"), QString("USD"), 3});
    source->push({QString("B"), QString("CAD"), 4});
    REQUIRE(model.get<QString>(0, 1) == "CAD (2)");
    REQUIRE(model.get<QString>(1, 1) == "A (1)");
    REQUIRE(model.get<int>(0, 2) == 20);
    REQUIRE(model.get<int>(1, 2) == 4);
    source->transact([&] {
      source->set(0, 0, QString("B"));
      source->set(0, 2, 5);
    });
    REQUIRE(model.get_row_size() == 7);
    REQUIRE(model.get<QString>(0, 1) == "CAD (2)");
    REQUIRE(model.get<QString>(0, 0) == "(1)");
    REQUIRE(model.get<QString>(1, 1) == "B (2)");
    REQUIRE(model.get<int>(0, 2) == 41);
    REQUIRE(model.get<int>(1, 2) == 41);
    REQUIRE(model.get<int>(4, 2) == 9);
    source->remove(0);
    REQUIRE(model.get<int>(0, 2) == 16);
    source->remove(0);
    source->remove(0);
    REQUIRE(model.get_row_size() == 0);
    source->push({QString("C"), QString("EUR"), 6});
    REQUIRE(model.get_row_size() == 3);
    REQUIRE(model.get<int>(0, 2) == 36);
  }

  TEST_CASE("mixed_column_after_membership_change") {
    auto source = make_positions();
    auto model = GroupedTableModel(source, {0});
    source->transact([&] {
      source->set(1, 2, QString("10"));
      source->set(1, 0, QString("A"));
    });
    REQUIRE(model.get<QString>(0, 2) == "(2)");
    source->remove(0);
    REQUIRE(model.get<QString>(0, 2) == "(2)");
    source->set(0, 2, 5);
    REQUIRE(model.get<std::int64_t>(0, 2) == 35);
  }

  TEST_CASE("invalid_grouping_columns") {
    auto source = make_positions();
    REQUIRE_THROWS_AS(GroupedTableModel(source, {-1}), std::invalid_argument);
    REQUIRE_THROWS_AS(GroupedTableModel(source, {3}), std::invalid_argument);
    REQUIRE_THROWS_AS(GroupedTableModel(source, {1, 1}), std::invalid_argument);
  }
}
