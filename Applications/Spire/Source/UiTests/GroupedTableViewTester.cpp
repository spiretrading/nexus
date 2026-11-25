#include <algorithm>
#include <QApplication>
#include <QLayout>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/GroupedTableModel.hpp"
#include "Spire/Ui/GroupedTableView.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/MenuButton.hpp"
#include "Spire/Ui/TableColumnReorderController.hpp"
#include "Spire/Ui/TableGroupingBar.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/Tag.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Tests;

TEST_SUITE("GroupedTableView") {
  UI_TEST_CASE("nested_disclosure") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), QString("CAD"), 10});
    source->push({QString("B"), QString("CAD"), 20});
    source->push({QString("A"), QString("USD"), 30});
    auto view = std::unique_ptr<TableView>(GroupedTableViewBuilder(source).
      add_column("Account").add_column("Currency").add_column("Shares").
      set_grouping({0, 1}).make());
    REQUIRE_FALSE(
      dynamic_cast<TableGroupingBar*>(view->layout()->itemAt(0)->widget()));
    view->resize(500, 400);
    view->show();
    QApplication::processEvents();
    auto& body = view->get_body();
    auto& visible = *body.get_table();
    auto toggle = [&] (int row, int column, const QString& name) {
      QApplication::processEvents();
      auto item = body.find_item(TableIndex(row, column));
      REQUIRE(item);
      auto button = dynamic_cast<ToggleButton*>(item->get_body().focusProxy());
      REQUIRE(button);
      auto target = button->focusProxy();
      REQUIRE(target);
      REQUIRE(target->accessibleName() == "Expand or collapse " + name);
      auto current = button->get_current();
      auto previous = current->get();
      move_cursor(*target, target->rect().center());
      press_mouse(*target, Qt::LeftButton);
      release_mouse(*target, Qt::LeftButton);
      QApplication::processEvents();
      REQUIRE(current->get() != previous);
    };
    REQUIRE(visible.get_row_size() == 8);
    auto indent = [&] (int row) {
      auto item = body.find_item(TableIndex(row, 0));
      REQUIRE(item);
      return item->get_body().layout()->contentsMargins().left();
    };
    REQUIRE(indent(0) < indent(1));
    REQUIRE(indent(1) < indent(2));
    toggle(1, 0, "CAD");
    REQUIRE(visible.get_row_size() == 7);
    REQUIRE(visible.get<QString>(2, 0) == "USD (1)");
    toggle(0, 0, "A");
    REQUIRE(visible.get_row_size() == 4);
    REQUIRE(visible.get<QString>(1, 0) == "B (1)");
    SUBCASE("sort_collapsed_groups") {
      auto header = view->get_header().get_item(2);
      move_cursor(*header, header->rect().center());
      press_mouse(*header, Qt::LeftButton);
      release_mouse(*header, Qt::LeftButton);
      QApplication::processEvents();
      REQUIRE(visible.get_row_size() == 4);
      REQUIRE(visible.get<QString>(0, 0) == "B (1)");
      REQUIRE(visible.get<QString>(3, 0) == "A (2)");
      source->set(1, 2, 60);
      REQUIRE(visible.get_row_size() == 4);
      REQUIRE(visible.get<QString>(0, 0) == "A (2)");
      toggle(0, 0, "A");
      REQUIRE(visible.get_row_size() == 7);
      REQUIRE(visible.get<QString>(1, 0) == "CAD (1)");
      REQUIRE(visible.get<QString>(2, 0) == "USD (1)");
    }
    SUBCASE("edit_and_regroup_collapsed_groups") {
      source->set(0, 2, 12);
      source->push({QString("A"), QString("CAD"), 5});
      REQUIRE(visible.get_row_size() == 4);
      REQUIRE(visible.get<QString>(0, 0) == "A (3)");
      REQUIRE(visible.get<std::int64_t>(0, 2) == 47);
      toggle(0, 0, "A");
      REQUIRE(visible.get_row_size() == 7);
      REQUIRE(visible.get<QString>(1, 0) == "CAD (2)");
      REQUIRE(visible.get<std::int64_t>(1, 2) == 17);
      toggle(1, 0, "CAD");
      REQUIRE(visible.get_row_size() == 9);
      REQUIRE(visible.get<int>(2, 2) == 12);
      REQUIRE(visible.get<int>(3, 2) == 5);
      toggle(0, 0, "A");
      auto table =
        std::static_pointer_cast<GroupedTableModel>(view->get_table());
      table->set_grouping({0});
      REQUIRE(visible.get_row_size() == 3);
      table->set_grouping({1});
      REQUIRE(visible.get_row_size() == 6);
      toggle(0, 1, "CAD");
      REQUIRE(visible.get_row_size() == 3);
      REQUIRE(visible.get<QString>(1, 1) == "USD (1)");
      table->set_grouping({});
      REQUIRE(visible.get_row_size() == 4);
      REQUIRE(visible.get<int>(0, 2) == 12);
    }
  }

  UI_TEST_CASE("drag_column_with_collapsed_group") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), QString("CAD"), 10});
    source->push({QString("A"), QString("CAD"), 30});
    source->push({QString("B"), QString("CAD"), 5});
    source->push({QString("B"), QString("USD"), 20});
    auto view = std::unique_ptr<TableView>(GroupedTableViewBuilder(source).
      add_column("Account").add_column("Currency").add_column("Shares").make());
    auto controller = TableColumnReorderController(*view);
    auto table = std::static_pointer_cast<GroupedTableModel>(view->get_table());
    table->set_grouping({0, 1});
    auto& header = view->get_header();
    for(auto i = 0; i != 3; ++i) {
      header.get_widths()->set(i, scale_width(100));
    }
    view->resize(scale_width(400), scale_height(360));
    view->show();
    QApplication::processEvents();
    auto& body = view->get_body();
    auto& rows = *body.get_table();
    auto account = body.find_item(TableIndex(0, 0));
    REQUIRE(account);
    auto toggle =
      dynamic_cast<ToggleButton*>(account->get_body().focusProxy());
    REQUIRE(toggle);
    toggle->get_current()->set(false);
    REQUIRE(rows.get_row_size() == 6);
    REQUIRE(rows.get<QString>(0, 0) == "A (2)");
    auto shares = header.get_item(2);
    move_cursor(*shares, shares->rect().center());
    auto receiver = QApplication::widgetAt(QCursor::pos());
    REQUIRE(receiver);
    press_mouse(*receiver, Qt::LeftButton);
    auto currency = header.get_item(1);
    move_cursor(*currency, currency->rect().center());
    auto position = QCursor::pos();
    auto move = QMouseEvent(QEvent::MouseMove,
      receiver->mapFromGlobal(position), position, Qt::NoButton,
      Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(receiver, &move);
    release_mouse(*receiver, Qt::LeftButton);
    QApplication::processEvents();
    REQUIRE(header.get_item(1) == shares);
    REQUIRE(header.get_column_order()->get(1) == 2);
    REQUIRE(rows.get<QString>(0, 0) == "A (2)");
    move_cursor(*shares, shares->rect().center());
    press_mouse(*shares, Qt::LeftButton);
    release_mouse(*shares, Qt::LeftButton);
    REQUIRE(header.get_items()->get(1).m_order ==
      TableHeaderItem::Order::ASCENDING);
    REQUIRE(rows.get_row_size() == 6);
    REQUIRE(rows.get<QString>(0, 0) == "B (2)");
    REQUIRE(rows.get<QString>(1, 0) == "CAD (1)");
    REQUIRE(rows.get<QString>(5, 0) == "A (2)");
    REQUIRE(table->get_grouping() == std::vector{0, 1});
    auto bar =
      dynamic_cast<TableGroupingBar*>(view->layout()->itemAt(0)->widget());
    REQUIRE(bar);
    auto tags = bar->layout()->itemAt(0)->layout();
    REQUIRE(tags->count() == 3);
    auto first = dynamic_cast<Tag*>(tags->itemAt(0)->widget());
    auto second = dynamic_cast<Tag*>(tags->itemAt(2)->widget());
    REQUIRE(first);
    REQUIRE(second);
    REQUIRE(first->get_label()->get() == "Account");
    REQUIRE(second->get_label()->get() == "Currency");
  }

  UI_TEST_CASE("column_order_settings") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({"A", 3});
    source->push({"B", 8});
    auto view = std::unique_ptr<TableView>(GroupedTableViewBuilder(source).
      add_column("Account").set_order(TableHeaderItem::Order::UNORDERED).
      add_column("Shares").set_order(TableHeaderItem::Order::DESCENDING).
      set_grouping({0}).make());
    view->resize(320, 240);
    view->show();
    QApplication::processEvents();
    auto& rows = *view->get_body().get_table();
    REQUIRE(rows.get<QString>(0, 0) == "B (1)");
    auto header = view->get_header().get_item(0);
    move_cursor(*header, header->rect().center());
    press_mouse(*header, Qt::LeftButton);
    release_mouse(*header, Qt::LeftButton);
    REQUIRE(rows.get<QString>(0, 0) == "B (1)");
    REQUIRE(view->get_header().get_items()->get(0).m_order ==
      TableHeaderItem::Order::UNORDERED);
  }

  UI_TEST_CASE("accumulator_factory") {
    class Total {
      public:
        explicit Total(int opening) noexcept
          : m_total(opening) {}

        void add(const AnyRef& value) {
          m_total += any_cast<int>(value);
        }

        void remove(const AnyRef& value) {
          m_total -= any_cast<int>(value);
        }

        void update(const AnyRef& previous, const AnyRef& current) {
          m_total += any_cast<int>(current) - any_cast<int>(previous);
        }

        int get() const {
          return m_total;
        }

      private:
        int m_total;
    };
    auto source = std::make_shared<ArrayTableModel>();
    source->push({"A", 10, 5});
    source->push({"B", 30, 9});
    source->push({"A", 20, 2});
    auto builder = GroupedTableViewBuilder(source);
    auto opening = 100;
    auto view = std::unique_ptr<TableView>(builder.add_column("Account").
      add_column("Balance").set_aggregation([=] {
        return Total(opening);
      }).add_column("Shares").set_aggregation(make_default_table_aggregation).
      set_grouping({0}).make());
    auto& table = *view->get_table();
    REQUIRE(table.get<int>(0, 1) == 130);
    REQUIRE(table.get<int>(3, 1) == 130);
    REQUIRE(table.get<std::int64_t>(0, 2) == 7);
    source->set(0, 1, 25);
    REQUIRE(table.get<int>(0, 1) == 145);
    source->remove(2);
    REQUIRE(table.get<int>(0, 1) == 125);
    REQUIRE(table.get<int>(2, 1) == 130);
  }

  UI_TEST_CASE("grouping_bar") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), QString("CAD"), 10});
    source->push({QString("B"), QString("CAD"), 20});
    source->push({QString("A"), QString("USD"), 30});
    auto builder = GroupedTableViewBuilder(source).
      add_column("Account", "Acct").add_column("Currency").add_column("Shares");
    auto view = std::unique_ptr<TableView>(builder.make());
    auto& account = view->get_header().get_items()->get(0);
    REQUIRE(account.m_name == "Account");
    REQUIRE(account.m_short_name == "Acct");
    view->resize(550, 360);
    view->show();
    QApplication::processEvents();
    auto bar =
      dynamic_cast<TableGroupingBar*>(view->layout()->itemAt(0)->widget());
    REQUIRE(bar);
    auto tags = bar->layout()->itemAt(0)->layout();
    auto button = dynamic_cast<MenuButton*>(bar->layout()->itemAt(1)->widget());
    REQUIRE(button);
    auto& menu = button->get_menu();
    auto table = std::static_pointer_cast<GroupedTableModel>(view->get_table());
    auto& rows = *view->get_body().get_table();
    auto select = [&] (int column) {
      press_key(*button, Qt::Key_Space);
      release_key(*button, Qt::Key_Space);
      QApplication::processEvents();
      REQUIRE(menu.isVisible());
      auto item = dynamic_cast<ListItem*>(menu.get_menu_item(column));
      REQUIRE(item);
      auto check = dynamic_cast<CheckBox*>(&item->get_body());
      REQUIRE(check);
      REQUIRE(check->get_current()->get() ==
        std::ranges::contains(table->get_grouping(), column));
      press_key(*item, Qt::Key_Return);
      QApplication::processEvents();
      REQUIRE_FALSE(menu.isVisible());
    };
    auto tag_at = [&] (int index) {
      auto item = tags->itemAt(index);
      REQUIRE(item);
      auto tag = dynamic_cast<Tag*>(item->widget());
      REQUIRE(tag);
      return tag;
    };
    auto remove = [&] (int index) {
      auto tag = tag_at(index);
      move_cursor(
        *tag, QPoint(tag->width() - scale_width(9), tag->rect().center().y()));
      press_mouse(*tag, Qt::LeftButton);
      release_mouse(*tag, Qt::LeftButton);
      QApplication::processEvents();
    };
    REQUIRE(tags->count() == 0);
    REQUIRE(menu.get_count() == 3);
    REQUIRE(rows.get_row_size() == 3);
    auto submitted = QString();
    menu.connect_submit_signal([&] (const auto&, const QString& name) {
      submitted = name;
    });
    select(0);
    REQUIRE(submitted == "Account");
    REQUIRE(rows.get<int>(2, 2) == 30);
    select(1);
    REQUIRE(table->get_grouping() == std::vector{0, 1});
    REQUIRE(rows.get_row_size() == 8);
    REQUIRE(rows.get<QString>(0, 0) == "A (2)");
    REQUIRE(rows.get<QString>(1, 0) == "CAD (1)");
    REQUIRE(rows.get<QString>(3, 0) == "USD (1)");
    REQUIRE(rows.get<int>(4, 2) == 30);
    REQUIRE(rows.get<int>(7, 2) == 20);
    REQUIRE(tags->count() == 3);
    REQUIRE(tag_at(0)->get_label()->get() == "Account");
    REQUIRE(dynamic_cast<Icon*>(tags->itemAt(1)->widget()));
    REQUIRE(tag_at(2)->get_label()->get() == "Currency");
    remove(0);
    REQUIRE(table->get_grouping() == std::vector{1});
    REQUIRE(tags->count() == 1);
    REQUIRE(tag_at(0)->get_label()->get() == "Currency");
    REQUIRE(rows.get<QString>(0, 1) == "CAD (2)");
    REQUIRE(rows.get<std::int64_t>(0, 2) == 30);
    select(0);
    REQUIRE(table->get_grouping() == std::vector{1, 0});
    remove(2);
    REQUIRE(tags->count() == 1);
    select(1);
    REQUIRE(table->get_grouping().empty());
    REQUIRE(tags->count() == 0);
    REQUIRE(rows.get_row_size() == 3);
    table->set_grouping({2, 1});
    REQUIRE(tag_at(0)->get_label()->get() == "Shares");
    REQUIRE(tag_at(2)->get_label()->get() == "Currency");
    select(2);
    REQUIRE(table->get_grouping() == std::vector{1});
    auto fixed = std::unique_ptr<TableView>(builder.set_grouping({}).make());
    REQUIRE_FALSE(
      dynamic_cast<TableGroupingBar*>(fixed->layout()->itemAt(0)->widget()));
  }
}
