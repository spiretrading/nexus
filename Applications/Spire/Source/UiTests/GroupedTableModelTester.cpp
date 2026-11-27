#include <cmath>
#include <limits>
#include <set>
#include <doctest/doctest.h>
#include <QColor>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/TableRowIndexTracker.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DefaultTableAggregator.hpp"
#include "Spire/Ui/DistinctTableAggregator.hpp"
#include "Spire/Ui/GroupedTableModel.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
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
            REQUIRE(Spire::is_equal(cell, update.m_previous));
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
              REQUIRE(
                Spire::is_equal(rows[row][column], model.at(row, column)));
            }
            auto index = model.index_to_source(row);
            if(index != -1) {
              REQUIRE(model.index_from_source(index) == row);
            }
          }
        });
    });
  }

  class SumOfSquares : public TableAggregator {
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
    auto model = GroupedTableModel(source);
    model.set_grouping({0});
    source->push({first, 30});
    auto heading = [&] (int source_row) {
      return model.get_parent(model.index_from_source(source_row));
    };
    REQUIRE(heading(0) != heading(1));
    REQUIRE(heading(0) == heading(2));
    REQUIRE(
      model.get_group_name(heading(0)) == model.get_group_name(heading(1)));
    REQUIRE(model.get<int>(heading(0), 1) == 40);
    REQUIRE(model.get<int>(heading(1), 1) == 20);
    source->set(2, 0, second);
    REQUIRE(heading(1) == heading(2));
    REQUIRE(model.get_group_size(heading(0)) == 1);
    REQUIRE(model.get<int>(heading(0), 1) == 10);
    REQUIRE(model.get<int>(heading(1), 1) == 50);
  }

  TEST_CASE("equal_color_group_keys") {
    auto first = QColor();
    auto second = QColor();
    auto distinct = QColor();
    SUBCASE("hsl_lightness") {
      first = QColor::fromHslF(0, 0, 32767.0 / 65535);
      second = QColor::fromHslF(0, 0, 32768.0 / 65535);
      distinct = QColor::fromHslF(0, 0, 1);
    }
    SUBCASE("hsv_hue") {
      first = QColor::fromHsvF(-1, 1, 1);
      second = QColor::fromHsvF(295.35 / 360, 1, 1);
      distinct = QColor::fromHsvF(0, 1, 1);
    }
    REQUIRE(first == second);
    REQUIRE(first.rgba() != second.rgba());
    auto source = std::make_shared<ArrayTableModel>();
    source->push({first, 10});
    auto model = GroupedTableModel(source);
    model.set_grouping({0});
    source->push({distinct, 30});
    source->push({second, 20});
    auto heading = [&] (int row) {
      return model.get_parent(model.index_from_source(row));
    };
    REQUIRE(heading(0) == heading(2));
    REQUIRE(heading(0) != heading(1));
    REQUIRE(model.get_group_size(heading(0)) == 2);
    REQUIRE(model.get<int>(heading(0), 1) == 30);
  }

  TEST_CASE("numeric_group_names") {
    auto check = [] (const std::any& first, const std::any& second,
        const QString& first_name, const QString& second_name) {
      auto source = std::make_shared<ArrayTableModel>();
      source->push({first});
      source->push({second});
      source->push({first});
      auto model = GroupedTableModel(source);
      model.set_grouping({0});
      REQUIRE(model.get<QString>(0, 0) == first_name + " (2)");
      REQUIRE(model.get<QString>(3, 0) == second_name + " (1)");
    };
    check(short(7), short(8), "7", "8");
    check(static_cast<signed char>(-7), static_cast<signed char>(8), "-7", "8");
    auto maximum = std::numeric_limits<std::uint64_t>::max();
    check(make_optional(maximum), optional<std::uint64_t>(0), to_text(maximum),
      "0");
    check(
      make_optional(1.25L), optional<long double>(), to_text(1.25), QString());
    check(make_optional(Quantity(7)), make_optional(Quantity(8)), "7", "8");
  }

  TEST_CASE("group_name_change") {
    auto account = DirectoryEntry::make_account(10, "A");
    auto source = std::make_shared<ArrayTableModel>();
    source->push({account});
    source->push({DirectoryEntry::make_account(20, "B")});
    auto model = GroupedTableModel(source);
    model.set_grouping({0});
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

  TEST_CASE("aggregate_account_rename") {
    auto account = DirectoryEntry::make_account(10, "A");
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("Desk"), account});
    source->push({QString("Desk"), DirectoryEntry::make_account(20, "B")});
    SUBCASE("distinct_accounts") {
      auto model = GroupedTableModel(source);
      model.set_grouping({0});
      REQUIRE(model.get<DistinctCount>(0, 1) == DistinctCount(2));
      account.m_name = "B";
      source->set(0, 1, account);
      REQUIRE(model.get<DistinctCount>(0, 1) == DistinctCount(2));
      source->set(0, 1, DirectoryEntry::make_account(20, "B"));
      REQUIRE(model.get<DistinctCount>(0, 1) == DistinctCount(1));
    }
    SUBCASE("custom_aggregate") {
      struct FirstAccount : TableAggregator {
        std::multiset<DirectoryEntry> m_accounts;

        void add(const AnyRef& value) override {
          m_accounts.insert(any_cast<DirectoryEntry>(value));
        }

        void remove(const AnyRef& value) override {
          m_accounts.erase(m_accounts.find(any_cast<DirectoryEntry>(value)));
        }

        void update(const AnyRef& previous, const AnyRef& current) override {
          remove(previous);
          add(current);
        }

        std::any get() const override {
          if(m_accounts.empty()) {
            return {};
          }
          return *m_accounts.begin();
        }
      };
      auto is_equal = [] (const AnyRef& previous, const AnyRef& current) {
        REQUIRE(previous.is_const());
        REQUIRE(current.is_const());
        if(auto a = any_cast<DirectoryEntry>(&previous)) {
          auto b = any_cast<DirectoryEntry>(&current);
          return b && *a == *b && a->m_name == b->m_name;
        }
        return Spire::is_equal(previous, current);
      };
      auto model = GroupedTableModel(source, {{}, [] {
        return std::make_unique<FirstAccount>();
      }}, {}, is_equal);
      model.set_grouping({0});
      auto updates = 0;
      model.connect_operation_signal([&] (const TableModel::Operation& op) {
        visit(op, [&] (const TableModel::UpdateOperation& update) {
          if(update.m_row == 0 && update.m_column == 1) {
            ++updates;
            REQUIRE(
              std::any_cast<DirectoryEntry>(update.m_previous).m_name == "A");
            REQUIRE(
              std::any_cast<DirectoryEntry>(update.m_value).m_name == "B");
          }
        });
      });
      account.m_name = "B";
      source->set(0, 1, account);
      REQUIRE(model.get<DirectoryEntry>(0, 1).m_name == "B");
      REQUIRE(updates == 1);
    }
  }

  TEST_CASE("unrecognized_group_key") {
    struct Key {};
    auto source = std::make_shared<ArrayTableModel>();
    source->push({Key(), 10});
    source->push({Key(), 20});
    auto model = GroupedTableModel(source);
    model.set_grouping({0});
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
    REQUIRE(model.get<int>(0, 1) == 11);
    REQUIRE(model.get<int>(2, 1) == 20);
  }

  TEST_CASE("group_lookup_after_removal") {
    auto source = std::make_shared<ArrayTableModel>();
    auto nan = std::numeric_limits<double>::quiet_NaN();
    source->push({0.0, 1});
    source->push({nan, 2});
    source->push({-0.0, 3});
    auto model = GroupedTableModel(source);
    model.set_grouping({0});
    REQUIRE(model.get_row_size() == 5);
    REQUIRE(model.get_group_size(0) == 2);
    source->remove(2);
    REQUIRE(model.index_from_source(1) == 3);
    source->remove(0);
    source->push({-0.0, 4});
    source->push({-nan, 5});
    REQUIRE(model.get_group_size(0) == 2);
    REQUIRE(model.get<int>(0, 1) == 7);
    REQUIRE(model.index_from_source(1) == 4);
    REQUIRE(model.index_to_source(2) == 2);
  }

  TEST_CASE("source_order_without_groups") {
    auto source = make_positions();
    auto model = GroupedTableModel(source);
    REQUIRE(model.get_grouping().empty());
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
    auto model = GroupedTableModel(source);
    model.set_grouping({0, 1});
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
    REQUIRE(model.get<int>(6, 2) == 55);
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
    auto factories = std::vector<GroupedTableModel::AggregatorFactory>();
    SUBCASE("implicit_default") {}
    SUBCASE("explicit_default") {
      factories.resize(2);
      factories[1] = [] {
        return std::make_unique<DefaultTableAggregator>();
      };
    }
    auto model = GroupedTableModel(source, factories);
    model.set_grouping({0});
    model.set_column_order({{0, DESCENDING}});
    REQUIRE(model.get<QString>(0, 0) == "10 (2)");
    model.set_column_order({{1, DESCENDING}});
    REQUIRE(model.get<DistinctCount>(0, 1) == DistinctCount(10));
    REQUIRE(model.get<DistinctCount>(11, 1) == DistinctCount(2));
  }

  TEST_CASE("sort_by_custom_aggregate") {
    using enum SortedTableModel::Ordering;
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), 4});
    source->push({QString("A"), 4});
    source->push({QString("B"), 7});
    auto model = GroupedTableModel(source, {{}, [] {
      return std::make_unique<SumOfSquares>();
    }});
    model.set_grouping({0});
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
    auto model = GroupedTableModel(source, [&] (const AnyRef& left,
        int left_row, const AnyRef& right, int right_row, int column) {
      REQUIRE(column == 2);
      REQUIRE(Spire::is_equal(table->at(left_row, column), left));
      REQUIRE(Spire::is_equal(table->at(right_row, column), right));
      return compare(right, left);
    });
    table = &model;
    model.set_grouping({0});
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
    auto model = GroupedTableModel(make_positions(),
      [] (const AnyRef& left, const AnyRef& right) {
        return any_cast<QString>(left) > any_cast<QString>(right);
      });
    model.set_grouping({0});
    model.set_column_order({{0, ASCENDING}});
    REQUIRE(model.get_group_name(0) == "B");
    REQUIRE(model.get_group_name(2) == "A");
  }

  TEST_CASE("nested_group_comparator_columns") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), 1});
    source->push({QString("B"), 2});
    source->push({QString("A"), 3});
    auto model = GroupedTableModel(source, [] (const AnyRef& left,
        int, const AnyRef& right, int, int column) {
      if(column == 0) {
        return any_cast<QString>(left) > any_cast<QString>(right);
      }
      REQUIRE(column == 1);
      return any_cast<int>(left) > any_cast<int>(right);
    });
    model.set_grouping({0, 1});
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
    auto model = GroupedTableModel(source);
    model.set_grouping({0});
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
    source->push({QString("A"), maximum - 1});
    source->push({QString("B"), maximum - 1});
    source->push({QString("B"), std::uint64_t(2)});
    auto comparator = GroupedTableModel::Comparator();
    SUBCASE("default_comparator") {}
    SUBCASE("reused_comparator") {
      comparator = GroupedTableModel(source).get_comparator();
    }
    auto model = GroupedTableModel(source, comparator);
    model.set_grouping({0});
    model.set_column_order({{1, DESCENDING}});
    REQUIRE(model.get<QString>(0, 0) == "A (1)");
    REQUIRE(model.get<std::uint64_t>(0, 1) == maximum - 1);
    REQUIRE_FALSE(model.at(2, 1).has_value());
    source->set(2, 1, std::uint64_t(1));
    REQUIRE(model.get<QString>(0, 0) == "B (2)");
    REQUIRE(model.get<std::uint64_t>(0, 1) == maximum);
    auto locale = QLocale(QLocale::English, QLocale::Canada);
    REQUIRE(to_text(model.at(0, 1), locale) == to_text(maximum, locale));
    REQUIRE(model.index_to_source(1) == 1);
  }

  TEST_CASE("sort_order_across_regrouping") {
    using enum SortedTableModel::Ordering;
    auto source = make_positions();
    auto model = GroupedTableModel(source);
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
    auto model = GroupedTableModel(source);
    model.set_grouping({0});
    REQUIRE(model.get_row_size() == 5);
    REQUIRE(model.get<QString>(0, 0) == "A (2)");
    REQUIRE(model.get<DistinctCount>(0, 1) == DistinctCount(2));
    REQUIRE(model.get<int>(0, 2) == 40);
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
    REQUIRE(model.get<int>(0, 2) == 30);
    REQUIRE(model.get<QString>(2, 0) == "B (2)");
    REQUIRE(model.get<int>(2, 2) == 35);
    REQUIRE(moves == 1);
    model.set(model.index_from_source(2), 0, QString("B"));
    REQUIRE(model.get_row_size() == 4);
    REQUIRE(model.get<QString>(0, 0) == "B (3)");
    REQUIRE(model.get<int>(0, 2) == 65);
    REQUIRE(moves == 2);
    source->transact([&] {
      source->set(2, 2, 40);
      source->move(2, 0);
      source->set(1, 2, 5);
      source->insert({QString("C"), QString("USD"), 18}, 0);
    });
    REQUIRE(moves == 3);
    REQUIRE(model.get<int>(0, 2) == 65);
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
    auto model = GroupedTableModel(source);
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
    REQUIRE(model.get<int>(0, 2) == 80);
    REQUIRE(position.get_index() == 2);
    account.set(0);
    model.set_grouping({0, 1});
    REQUIRE(commits == 2);
    REQUIRE(account.get_index() == 0);
    REQUIRE(model.get<QString>(1, 0) == "CAD (2)");
    REQUIRE(model.get<int>(1, 2) == 50);
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
      REQUIRE(model.get<DistinctCount>(0, 0) == DistinctCount(2));
      REQUIRE(model.get<int>(0, 2) == 70);
      REQUIRE(model.get<int>(2, 2) == 20);
      REQUIRE(position.get_index() == 5);
    }
    SUBCASE("remove_inner_column") {
      model.set_grouping({0});
      REQUIRE(account.get_index() == 0);
      REQUIRE(model.get<QString>(0, 0) == "A (3)");
      REQUIRE(model.get<int>(0, 2) == 80);
      REQUIRE(model.get<int>(3, 2) == 40);
      REQUIRE(position.get_index() == 2);
    }
    SUBCASE("reverse_grouping_order") {
      model.set_grouping({1, 0});
      REQUIRE(account.get_index() == -1);
      REQUIRE(model.get<QString>(0, 1) == "CAD (3)");
      REQUIRE(model.get<QString>(1, 1) == "A (2)");
      REQUIRE(model.get<int>(1, 2) == 50);
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

  TEST_CASE("first_grouping_row_mapping") {
    auto source = std::make_shared<ArrayTableModel>();
    for(auto i = 0; i != 120; ++i) {
      source->push({i % 12, i % 5, i});
    }
    auto model = GroupedTableModel(source);
    SUBCASE("source_order") {}
    SUBCASE("sorted_leaves") {
      model.set_column_order({{2, SortedTableModel::Ordering::DESCENDING}});
    }
    auto tracked = TableRowIndexTracker(model.index_from_source(37));
    model.connect_operation_signal([&] (const TableModel::Operation& op) {
      tracked.update(op);
      if(std::holds_alternative<TableModel::StartTransaction>(op)) {
        return;
      }
      REQUIRE(model.index_to_source(tracked.get_index()) == 37);
      REQUIRE(model.index_from_source(37) == tracked.get_index());
      REQUIRE(model.get<int>(tracked.get_index(), 2) == 37);
    });
    model.set_grouping({0, 1});
    REQUIRE(model.get_row_size() == 192);
    for(auto i = 0; i != source->get_row_size(); ++i) {
      REQUIRE(model.index_to_source(model.index_from_source(i)) == i);
    }
  }

  TEST_CASE("additional_grouping_row_mapping") {
    auto source = make_positions();
    auto model = GroupedTableModel(source);
    SUBCASE("one_existing_level") {
      model.set_grouping({0});
    }
    SUBCASE("two_existing_levels") {
      model.set_grouping({0, 1});
    }
    auto position = TableRowIndexTracker(model.index_from_source(1));
    auto heading = TableRowIndexTracker(model.get_parent(position.get_index()));
    auto name = model.get_group_name(heading.get_index());
    model.connect_operation_signal([&] (const TableModel::Operation& op) {
      heading.update(op);
      position.update(op);
      if(std::holds_alternative<TableModel::StartTransaction>(op)) {
        return;
      }
      auto row = heading.get_index();
      REQUIRE(model.get_group_name(row) == name);
      REQUIRE(model.get_group_size(row) == 1);
      REQUIRE(model.get<int>(row, 2) == 20);
      REQUIRE(model.index_from_source(1) == position.get_index());
      REQUIRE(model.index_to_source(position.get_index()) == 1);
      auto parent = model.get_parent(position.get_index());
      while(parent != row && parent != -1) {
        parent = model.get_parent(parent);
      }
      REQUIRE(parent == row);
    });
    model.set_grouping({0, 1, 2});
    REQUIRE(model.get_row_size() == 11);
    REQUIRE(model.get_first_leaf(heading.get_index()) == position.get_index());
  }

  TEST_CASE("first_grouping_factory_failure") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), 2});
    source->push({QString("B"), 3});
    auto attempts = 0;
    auto model = GroupedTableModel(source, {{}, [&] {
      ++attempts;
      if(attempts == 2) {
        throw std::runtime_error("Aggregation unavailable.");
      }
      return std::make_unique<SumOfSquares>();
    }});
    REQUIRE_THROWS_AS(model.set_grouping({0}), std::runtime_error);
    REQUIRE(model.get_grouping().empty());
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(model.get<QString>(1, 0) == "B");
    model.set_grouping({0});
    REQUIRE(model.get_row_size() == 4);
    REQUIRE(model.get<int>(2, 1) == 9);
  }

  TEST_CASE("grouping_during_source_transaction") {
    auto source = std::make_shared<ArrayTableModel>();
    auto factories = std::vector<GroupedTableModel::AggregatorFactory>(3);
    factories[2] = [] {
      return std::make_unique<SumOfSquares>();
    };
    auto model = GroupedTableModel(source, std::move(factories));
    model.set_grouping({0});
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

  TEST_CASE("nested_groups_and_custom_aggregator") {
    auto source = std::make_shared<ArrayTableModel>();
    auto factories = std::vector<GroupedTableModel::AggregatorFactory>(3);
    factories[2] = [] {
      return std::make_unique<SumOfSquares>();
    };
    auto model = GroupedTableModel(source, std::move(factories));
    model.set_grouping({1, 0});
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
    REQUIRE(model.get<DistinctCount>(0, 0) == DistinctCount(1));
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

  TEST_CASE("distinct_count_after_membership_change") {
    auto source = make_positions();
    auto factories = std::vector<GroupedTableModel::AggregatorFactory>();
    SUBCASE("implicit_default") {}
    SUBCASE("explicit_default") {
      factories.resize(2);
      factories[1] = [] {
        return std::make_unique<DefaultTableAggregator>();
      };
    }
    auto model = GroupedTableModel(source, std::move(factories));
    model.set_grouping({0});
    source->transact([&] {
      source->set(1, 1, QString("EUR"));
      source->set(1, 0, QString("A"));
    });
    REQUIRE(model.get<DistinctCount>(0, 1) == DistinctCount(3));
    source->remove(0);
    REQUIRE(model.get<DistinctCount>(0, 1) == DistinctCount(2));
    source->set(0, 1, QString("USD"));
    REQUIRE(model.get<DistinctCount>(0, 1) == DistinctCount(1));
  }

  TEST_CASE("aggregate_equality_on_empty_source") {
    auto source = std::make_shared<ArrayTableModel>();
    auto model = GroupedTableModel(source, {}, {},
      [] (const auto&, const auto&) {
        return false;
      });
    model.set_grouping({0});
    source->push({QString("A"), 1, 2});
    auto columns = std::set<int>();
    model.connect_operation_signal([&] (const TableModel::Operation& op) {
      if(auto update = std::get_if<TableModel::UpdateOperation>(&op)) {
        if(update->m_row == 0 && update->m_column != 0) {
          columns.insert(update->m_column);
        }
      }
    });
    source->push({QString("A"), 0, 0});
    REQUIRE(columns == std::set<int>{1, 2});
  }

  TEST_CASE("invalid_grouping_columns") {
    auto source = make_positions();
    auto model = GroupedTableModel(source);
    REQUIRE_THROWS_AS(model.set_grouping({-1}), std::invalid_argument);
    REQUIRE_THROWS_AS(model.set_grouping({3}), std::invalid_argument);
    REQUIRE_THROWS_AS(model.set_grouping({1, 1}), std::invalid_argument);
  }
}
