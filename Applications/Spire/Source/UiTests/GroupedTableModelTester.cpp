#include <cmath>
#include <limits>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/TableRowIndexTracker.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/GroupedTableModel.hpp"

using namespace Beam;
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

  void observe(GroupedTableModel& model, int& commits, int& moves) {
    auto rows = std::vector<std::vector<std::any>>();
    for(auto row = 0; row != model.get_row_size(); ++row) {
      rows.push_back(copy_row(model, row));
    }
    model.connect_operation_signal([&, rows = std::move(rows),
        opened = false, removing = -1] (const TableModel::Operation& op)
        mutable {
      visit(op,
        [&] (const TableModel::StartTransaction&) {
          REQUIRE_FALSE(opened);
          opened = true;
        },
        [&] (const TableModel::AddOperation& add) {
          REQUIRE(model.get_row_size() == std::ssize(rows) + 1);
          rows.insert(rows.begin() + add.m_index, copy_row(model, add.m_index));
        },
        [&] (const TableModel::PreRemoveOperation& removal) {
          REQUIRE(removing == -1);
          REQUIRE(removal.m_index < model.get_row_size());
          removing = removal.m_index;
        },
        [&] (const TableModel::RemoveOperation& removal) {
          REQUIRE(model.get_row_size() == std::ssize(rows) - 1);
          REQUIRE(removing == removal.m_index);
          rows.erase(rows.begin() + removing);
          removing = -1;
        },
        [&] (const TableModel::MoveOperation& move) {
          move_element(rows, move.m_source, move.m_destination);
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
  TEST_CASE("group_key_identity") {
    auto first = std::any();
    auto second = std::any();
    SUBCASE("accounts_with_the_same_name") {
      first = DirectoryEntry::make_account(10, "Trading");
      second = DirectoryEntry::make_account(20, "Trading");
    }
    SUBCASE("floats_with_the_same_label") {
      first = 1.0f;
      second = std::nextafter(1.0f, 2.0f);
    }
    SUBCASE("invalid_utf8") {
      first = std::string("\xff");
      second = std::string("\xfe");
    }
    REQUIRE(to_text(first) == to_text(second));
    auto source = std::make_shared<ArrayTableModel>();
    source->push({first, 10});
    source->push({second, 20});
    auto model = GroupedTableModel(source, {0});
    source->push({first, 30});
    auto heading = [&] (int source_row) {
      return model.get_parent(model.index_from_source(source_row));
    };
    REQUIRE(heading(0) != heading(1));
    REQUIRE(heading(0) == heading(2));
    REQUIRE(
      model.get_group_name(heading(0)) == model.get_group_name(heading(1)));
    REQUIRE(model.get<std::int64_t>(heading(0), 1) == 40);
    REQUIRE(model.get<std::int64_t>(heading(1), 1) == 20);
    source->set(2, 0, second);
    REQUIRE(heading(1) == heading(2));
    REQUIRE(model.get_group_size(heading(0)) == 1);
    REQUIRE(model.get<std::int64_t>(heading(0), 1) == 10);
    REQUIRE(model.get<std::int64_t>(heading(1), 1) == 50);
  }

  TEST_CASE("group_name_change") {
    auto account = DirectoryEntry::make_account(10, "A");
    auto source = std::make_shared<ArrayTableModel>();
    source->push({account});
    source->push({DirectoryEntry::make_account(20, "B")});
    auto model = GroupedTableModel(source, {0});
    model.set_column_order({{0, SortedTableModel::Ordering::ASCENDING}});
    auto commits = 0;
    auto moves = 0;
    observe(model, commits, moves);
    account.m_name = "C";
    source->set(0, 0, account);
    REQUIRE(model.get_group_name(0) == "B");
    REQUIRE(model.get_group_name(2) == "C");
    REQUIRE(model.index_to_source(3) == 0);
    REQUIRE(moves > 0);
  }

  TEST_CASE("unrecognized_group_key") {
    struct Key {};
    auto source = std::make_shared<ArrayTableModel>();
    source->push({Key(), 10});
    source->push({Key(), 20});
    auto model = GroupedTableModel(source, {0});
    REQUIRE(model.get_row_size() == 4);
    model.set_grouping({0, 1});
    REQUIRE(model.get_row_size() == 6);
    REQUIRE(model.get_group_size(0) == 1);
    REQUIRE(model.get_group_size(3) == 1);
    source->set(0, 1, 11);
    REQUIRE(model.get_row_size() == 6);
    REQUIRE(model.get_group_size(0) == 1);
    model.set_grouping({0});
    REQUIRE(model.get_row_size() == 4);
    REQUIRE(model.get<std::int64_t>(0, 1) == 11);
    REQUIRE(model.get<std::int64_t>(2, 1) == 20);
  }

  TEST_CASE("group_lookup_after_removal") {
    auto source = std::make_shared<ArrayTableModel>();
    auto nan = std::numeric_limits<double>::quiet_NaN();
    source->push({0.0, 1});
    source->push({nan, 2});
    source->push({-0.0, 3});
    auto model = GroupedTableModel(source, {0});
    REQUIRE(model.get_row_size() == 5);
    REQUIRE(model.get_group_size(0) == 2);
    source->remove(2);
    REQUIRE(model.index_from_source(1) == 3);
    source->remove(0);
    source->push({-0.0, 4});
    source->push({nan, 5});
    REQUIRE(model.get_group_size(0) == 2);
    REQUIRE(model.get<std::int64_t>(0, 1) == 7);
    REQUIRE(model.index_from_source(1) == 4);
    REQUIRE(model.index_to_source(2) == 2);
  }

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

  TEST_CASE("sibling_sort_order") {
    using enum SortedTableModel::Ordering;
    auto source = make_positions();
    source->push({QString("A"), QString("CAD"), 5});
    auto model = GroupedTableModel(source, {0, 1});
    auto commits = 0;
    auto moves = 0;
    observe(model, commits, moves);
    model.set_column_order({{2, ASCENDING}});
    REQUIRE(model.get<QString>(0, 0) == "B (1)");
    REQUIRE(model.get<QString>(3, 0) == "A (3)");
    REQUIRE(model.get<QString>(4, 0) == "CAD (2)");
    REQUIRE(model.index_to_source(5) == 3);
    REQUIRE(model.index_to_source(6) == 0);
    source->set(0, 2, 50);
    REQUIRE(model.get<QString>(4, 0) == "USD (1)");
    REQUIRE(model.get<std::int64_t>(6, 2) == 55);
    source->set(1, 2, 100);
    REQUIRE(model.get<QString>(0, 0) == "A (3)");
    REQUIRE(model.get<int>(2, 2) == 30);
    source->remove(0);
    REQUIRE(model.get<QString>(1, 0) == "CAD (1)");
    source->insert({QString("A"), QString("USD"), 90}, 0);
    REQUIRE(model.get<QString>(0, 0) == "B (1)");
    model.set_column_order({{2, DESCENDING}});
    REQUIRE(model.get<QString>(0, 0) == "A (3)");
    REQUIRE(model.get<QString>(1, 0) == "USD (2)");
    REQUIRE(model.get<int>(2, 2) == 90);
    model.set_column_order({});
    REQUIRE(model.get<QString>(0, 0) == "A (3)");
    REQUIRE(model.get<QString>(1, 0) == "CAD (1)");
    REQUIRE(model.get<int>(4, 2) == 90);
    REQUIRE(moves > 0);
  }

  TEST_CASE("group_sort_values") {
    using enum SortedTableModel::Ordering;
    auto source = std::make_shared<ArrayTableModel>();
    for(auto i = 0; i != 10; ++i) {
      source->push({2, QString::number(i)});
    }
    source->push({10, QString("CAD")});
    source->push({10, QString("USD")});
    auto factories = std::vector<TableAggregationFactory>();
    SUBCASE("implicit_default") {}
    SUBCASE("explicit_default") {
      factories = {{}, make_default_table_aggregation};
    }
    SUBCASE("wrapped_default") {
      factories = {{}, [] { return make_default_table_aggregation(); }};
    }
    auto model = GroupedTableModel(source, {0}, factories);
    model.set_column_order({{0, DESCENDING}});
    REQUIRE(model.get<QString>(0, 0) == "10 (2)");
    model.set_column_order({{1, DESCENDING}});
    REQUIRE(model.get<QString>(0, 1) == "(10)");
    REQUIRE(model.get<QString>(11, 1) == "(2)");
  }

  TEST_CASE("sort_by_custom_aggregate") {
    using enum SortedTableModel::Ordering;
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), 4});
    source->push({QString("A"), 4});
    source->push({QString("B"), 7});
    auto model = GroupedTableModel(source, {0}, {{}, [] {
      return std::make_unique<SumOfSquares>();
    }});
    auto commits = 0;
    auto moves = 0;
    observe(model, commits, moves);
    model.set_column_order({{1, DESCENDING}});
    REQUIRE(model.get<QString>(0, 0) == "B (1)");
    source->transact([&] {
      source->set(2, 1, 1);
      source->set(0, 0, QString("B"));
      source->set(1, 1, 8);
    });
    REQUIRE(model.get<QString>(0, 0) == "A (1)");
    REQUIRE(model.get<int>(0, 1) == 64);
    REQUIRE(model.get<QString>(2, 0) == "B (2)");
    REQUIRE(model.get<int>(2, 1) == 17);
    REQUIRE(model.index_to_source(3) == 0);
    REQUIRE(model.index_to_source(4) == 2);
  }

  TEST_CASE("sibling_comparator") {
    using enum SortedTableModel::Ordering;
    auto source = make_positions();
    auto table = static_cast<GroupedTableModel*>(nullptr);
    auto model = GroupedTableModel(source, {0}, [&] (const AnyRef& left,
        int left_row, const AnyRef& right, int right_row, int column) {
      REQUIRE(column == 2);
      REQUIRE(is_equal(table->at(left_row, column), left));
      REQUIRE(is_equal(table->at(right_row, column), right));
      return compare(right, left);
    });
    table = &model;
    model.set_column_order({{2, ASCENDING}});
    REQUIRE(model.get<QString>(0, 0) == "A (2)");
    REQUIRE(model.index_to_source(1) == 2);
    source->set(1, 2, 50);
    REQUIRE(model.get<QString>(0, 0) == "B (1)");
    model.set_grouping({});
    REQUIRE(model.index_to_source(0) == 1);
    REQUIRE(model.index_to_source(1) == 2);
    REQUIRE(model.index_to_source(2) == 0);
  }

  TEST_CASE("group_value_comparator") {
    using enum SortedTableModel::Ordering;
    auto model = GroupedTableModel(make_positions(), {0},
      [] (const AnyRef& left, const AnyRef& right) {
        return any_cast<QString>(left) > any_cast<QString>(right);
      });
    model.set_column_order({{0, ASCENDING}});
    REQUIRE(model.get_group_name(0) == "B");
    REQUIRE(model.get_group_name(2) == "A");
  }

  TEST_CASE("nested_group_comparator_columns") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), 1});
    source->push({QString("B"), 2});
    source->push({QString("A"), 3});
    auto model = GroupedTableModel(source, {0, 1}, [] (const AnyRef& left,
        int, const AnyRef& right, int, int column) {
      if(column == 0) {
        return any_cast<QString>(left) > any_cast<QString>(right);
      }
      REQUIRE(column == 1);
      return any_cast<int>(left) > any_cast<int>(right);
    });
    model.set_column_order({{0, SortedTableModel::Ordering::ASCENDING}});
    REQUIRE(model.get_group_name(0) == "B");
    REQUIRE(model.get_group_name(3) == "A");
    REQUIRE(model.get_group_name(4) == "3");
    REQUIRE(model.get_group_name(6) == "1");
  }

  TEST_CASE("floating_point_sort_keys") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), std::numeric_limits<float>::quiet_NaN()});
    source->push({QString("A"), 1.5f});
    source->push({QString("B"), 2.5f});
    auto model = GroupedTableModel(source, {0});
    model.set_column_order({{1, SortedTableModel::Ordering::ASCENDING}});
    REQUIRE(model.get_group_name(0) == "B");
    REQUIRE(model.get_group_name(2) == "A");
    REQUIRE(model.index_to_source(3) == 1);
    REQUIRE(model.index_to_source(4) == 0);
  }

  TEST_CASE("large_integer_sort_keys") {
    using enum SortedTableModel::Ordering;
    auto source = std::make_shared<ArrayTableModel>();
    auto maximum = std::numeric_limits<std::uint64_t>::max();
    source->push({QString("A"), maximum});
    source->push({QString("B"), maximum - 1});
    source->push({QString("B"), std::uint64_t(2)});
    auto model = GroupedTableModel(source, {0});
    model.set_column_order({{1, DESCENDING}});
    REQUIRE(model.get<QString>(0, 0) == "B (2)");
    source->set(2, 1, std::uint64_t(0));
    REQUIRE(model.get<QString>(0, 0) == "A (1)");
    REQUIRE(model.index_to_source(3) == 1);
  }

  TEST_CASE("sort_order_across_regrouping") {
    using enum SortedTableModel::Ordering;
    auto source = make_positions();
    auto model = GroupedTableModel(source, {});
    auto commits = 0;
    auto moves = 0;
    observe(model, commits, moves);
    model.set_column_order({{0, ASCENDING}, {2, DESCENDING}});
    REQUIRE(model.index_to_source(0) == 2);
    REQUIRE(model.index_to_source(1) == 0);
    model.set_grouping({0});
    REQUIRE(model.get<QString>(0, 0) == "A (2)");
    REQUIRE(model.index_to_source(1) == 2);
    model.set_grouping({0, 1});
    REQUIRE(model.get<QString>(1, 0) == "CAD (1)");
    model.set_grouping({});
    REQUIRE(model.index_to_source(0) == 2);
    source->set(0, 2, 30);
    REQUIRE(model.index_to_source(0) == 0);
    source->move(0, 2);
    REQUIRE(model.index_to_source(0) == 1);
    REQUIRE(model.index_to_source(1) == 2);
    model.set_column_order({{2, NONE}});
    REQUIRE(model.index_to_source(0) == 0);
    REQUIRE(model.get<int>(0, 2) == 20);
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
    auto commits = 0;
    auto moves = 0;
    observe(model, commits, moves);
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
    REQUIRE(moves == 1);
    model.set(model.index_from_source(2), 0, QString("B"));
    REQUIRE(model.get_row_size() == 4);
    REQUIRE(model.get<QString>(0, 0) == "B (3)");
    REQUIRE(model.get<std::int64_t>(0, 2) == 65);
    REQUIRE(moves == 2);
    source->transact([&] {
      source->set(2, 2, 40);
      source->move(2, 0);
      source->set(1, 2, 5);
      source->insert({QString("C"), QString("USD"), 18}, 0);
    });
    REQUIRE(moves == 3);
    REQUIRE(model.get<std::int64_t>(0, 2) == 65);
    REQUIRE(model.get<int>(1, 2) == 40);
    REQUIRE(model.get<int>(2, 2) == 5);
    REQUIRE(model.get<QString>(4, 0) == "C (1)");
    REQUIRE(model.remove(5) == QValidator::Acceptable);
    REQUIRE(model.get_row_size() == 4);
    REQUIRE(model.index_from_source(0) == 1);
    REQUIRE(commits == 4);
  }

  TEST_CASE("set_grouping") {
    auto source = make_positions();
    source->push({QString("A"), QString("CAD"), 40});
    auto model = GroupedTableModel(source, {});
    auto commits = 0;
    auto moves = 0;
    observe(model, commits, moves);
    auto position = TableRowIndexTracker(2);
    auto account = TableRowIndexTracker(-1);
    model.connect_operation_signal([&] (const TableModel::Operation& op) {
      position.update(op);
      account.update(op);
    });
    model.set_grouping({0});
    REQUIRE(commits == 1);
    REQUIRE(model.get<QString>(0, 0) == "A (3)");
    REQUIRE(model.get<std::int64_t>(0, 2) == 80);
    REQUIRE(position.get_index() == 2);
    account.set(0);
    model.set_grouping({0, 1});
    REQUIRE(commits == 2);
    REQUIRE(account.get_index() == 0);
    REQUIRE(model.get<QString>(1, 0) == "CAD (2)");
    REQUIRE(model.get<std::int64_t>(1, 2) == 50);
    REQUIRE(model.get<QString>(4, 0) == "USD (1)");
    REQUIRE(position.get_index() == 5);
    model.set_grouping({0, 1});
    REQUIRE_THROWS_AS(model.set_grouping({1, 1}), std::invalid_argument);
    REQUIRE_THROWS_AS(model.set_grouping({3}), std::invalid_argument);
    REQUIRE(model.get_grouping() == std::vector<int>{0, 1});
    REQUIRE(commits == 2);
    SUBCASE("replace_outer_column") {
      model.set_grouping({1});
      REQUIRE(account.get_index() == -1);
      REQUIRE(model.get<QString>(0, 1) == "CAD (3)");
      REQUIRE(model.get<QString>(0, 0) == "(2)");
      REQUIRE(model.get<std::int64_t>(0, 2) == 70);
      REQUIRE(model.get<int>(2, 2) == 20);
      REQUIRE(position.get_index() == 5);
    }
    SUBCASE("remove_inner_column") {
      model.set_grouping({0});
      REQUIRE(account.get_index() == 0);
      REQUIRE(model.get<QString>(0, 0) == "A (3)");
      REQUIRE(model.get<std::int64_t>(0, 2) == 80);
      REQUIRE(model.get<int>(3, 2) == 40);
      REQUIRE(position.get_index() == 2);
    }
    SUBCASE("reverse_grouping_order") {
      model.set_grouping({1, 0});
      REQUIRE(account.get_index() == -1);
      REQUIRE(model.get<QString>(0, 1) == "CAD (3)");
      REQUIRE(model.get<QString>(1, 1) == "A (2)");
      REQUIRE(model.get<std::int64_t>(1, 2) == 50);
      REQUIRE(model.get<QString>(4, 1) == "B (1)");
      REQUIRE(model.get<QString>(6, 1) == "USD (1)");
      REQUIRE(position.get_index() == 8);
    }
    model.set_grouping({});
    REQUIRE(commits == 4);
    REQUIRE(model.get_grouping().empty());
    REQUIRE(model.get_row_size() == source->get_row_size());
    REQUIRE(position.get_index() == 2);
    for(auto i = 0; i != source->get_row_size(); ++i) {
      REQUIRE(model.get<int>(i, 2) == source->get<int>(i, 2));
    }
  }

  TEST_CASE("grouping_during_source_transaction") {
    auto source = std::make_shared<ArrayTableModel>();
    auto factories = std::vector<TableAggregationFactory>(3);
    factories[2] = [] {
      return std::make_unique<SumOfSquares>();
    };
    auto model = GroupedTableModel(source, {0}, std::move(factories));
    model.set_grouping({1});
    source->push({QString("A"), QString("CAD"), 2});
    source->push({QString("A"), QString("USD"), 3});
    auto commits = 0;
    auto moves = 0;
    observe(model, commits, moves);
    source->transact([&] {
      source->set(0, 2, 5);
      model.set_grouping({});
      source->set(1, 2, 4);
      model.set_grouping({0});
      source->set(1, 2, 6);
      REQUIRE(commits == 0);
    });
    REQUIRE(commits == 1);
    REQUIRE(model.get<QString>(0, 0) == "A (2)");
    REQUIRE(model.get<int>(0, 2) == 61);
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
    auto factories = std::vector<TableAggregationFactory>();
    SUBCASE("implicit_default") {}
    SUBCASE("explicit_default") {
      factories.resize(3);
      factories[2] = &make_default_table_aggregation;
    }
    auto model = GroupedTableModel(source, {0}, std::move(factories));
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
