#include <algorithm>
#include <QApplication>
#include <QLabel>
#include <QLayout>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Ui/BoxPainter.hpp"
#include "Spire/Ui/GroupedTableModel.hpp"
#include "Spire/Ui/GroupedTableView.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/MenuButton.hpp"
#include "Spire/Ui/RecycledTableViewItemBuilder.hpp"
#include "Spire/Ui/TableColumnReorderController.hpp"
#include "Spire/Ui/TableGroupingBar.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/Tag.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Tests;

TEST_SUITE("GroupedTableView") {
  UI_TEST_CASE("source_column_count") {
    auto source = std::make_shared<ArrayTableModel>();
    auto builder = GroupedTableViewBuilder(source);
    builder.add_column("Account").add_column("Shares");
    auto grouped = false;
    SUBCASE("ungrouped") {}
    SUBCASE("grouped") {
      builder.set_grouping({1});
      grouped = true;
    }
    auto view = std::unique_ptr<TableView>(builder.make());
    view->show();
    auto& visible = *view->get_body().get_table();
    for(auto& row : std::vector<std::vector<std::any>>{
        {1, 2, 3, 4}, {1}, {}}) {
      source->push(row);
      QApplication::processEvents();
      REQUIRE(visible.get_row_size() == 0);
      REQUIRE(visible.get_column_size() == 2);
      REQUIRE(source->get_row_size() == 1);
      source->remove(0);
      source->push({QString("A"), 10});
      QApplication::processEvents();
      auto leaf = static_cast<int>(grouped);
      REQUIRE(visible.get_row_size() == leaf + 1);
      REQUIRE(visible.get<int>(leaf, 1) == 10);
      REQUIRE(view->get_body().find_item(TableIndex(leaf, 1)));
      source->remove(0);
    }
  }

  UI_TEST_CASE("header_sort_after_programmatic_sort") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("Long"), QString("A"), 20});
    source->push({QString("Long"), QString("B"), 10});
    source->push({QString("Long"), QString("C"), 20});
    auto view = std::unique_ptr<TableView>(GroupedTableViewBuilder(source).
      add_column("Side").add_column("Account").add_column("Shares").
      set_grouping({0}).make());
    view->resize(500, 200);
    view->show();
    auto& table = *view->get_table();
    auto& header = view->get_header();
    header.get_items()->move(2, 1);
    view->set_column_order({{1, SortedTableModel::Ordering::DESCENDING}});
    QApplication::processEvents();
    REQUIRE(header.get_items()->get(2).m_order ==
      TableHeaderItem::Order::DESCENDING);
    REQUIRE(table.get<QString>(1, 1) == "C");
    auto shares = header.get_item(1);
    move_cursor(*shares, shares->rect().center());
    press_mouse(*shares, Qt::LeftButton);
    release_mouse(*shares, Qt::LeftButton);
    REQUIRE(header.get_items()->get(1).m_order ==
      TableHeaderItem::Order::ASCENDING);
    REQUIRE(header.get_items()->get(2).m_order == TableHeaderItem::Order::NONE);
    REQUIRE(table.get<QString>(1, 1) == "B");
    REQUIRE(table.get<QString>(2, 1) == "C");
    REQUIRE(table.get<QString>(3, 1) == "A");
    view->set_column_order({});
    REQUIRE(header.get_items()->get(1).m_order == TableHeaderItem::Order::NONE);
    REQUIRE(table.get<QString>(1, 1) == "A");
  }

  UI_TEST_CASE("grouped_comparator") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), QString("CAD")});
    source->push({QString("A"), QString("USD")});
    source->push({QString("B"), QString("CAD")});
    auto builder = GroupedTableViewBuilder(source);
    builder.add_column("Account").add_column("Currency").
      set_order(TableHeaderItem::Order::ASCENDING).set_grouping({0});
    auto reverse = [] (const AnyRef& left, const AnyRef& right) {
      if(left.get_type() == typeid(QString)) {
        return any_cast<QString>(left) > any_cast<QString>(right);
      }
      return any_cast<std::int64_t>(left) > any_cast<std::int64_t>(right);
    };
    SUBCASE("values") {
      builder.set_comparator(reverse);
    }
    SUBCASE("rows") {
      builder.set_comparator([&] (const AnyRef& left, int left_row,
          const AnyRef& right, int right_row, int column) {
        REQUIRE(left_row != right_row);
        REQUIRE(column == 1);
        return reverse(left, right);
      });
    }
    auto view = std::unique_ptr<TableView>(builder.make());
    auto& table = *view->get_table();
    REQUIRE(table.get<QString>(0, 0) == "A (2)");
    REQUIRE(table.get<QString>(0, 1) == "(2)");
    REQUIRE(table.get<QString>(1, 1) == "USD");
    REQUIRE(table.get<QString>(2, 1) == "CAD");
  }

  UI_TEST_CASE("selection_across_groups") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), QString("CAD"), 10});
    source->push({QString("B"), QString("USD"), 20});
    source->push({QString("A"), QString("CAD"), 30});
    auto view = std::unique_ptr<TableView>(GroupedTableViewBuilder(source).
      add_column("Account").add_column("Currency").add_column("Shares").
      set_grouping({0, 1}).make());
    auto table = std::static_pointer_cast<GroupedTableModel>(view->get_table());
    SUBCASE("source_order") {}
    SUBCASE("sorted") {
      table->set_column_order({{2, SortedTableModel::Ordering::DESCENDING}});
    }
    auto& rows = *view->get_body().get_selection()->get_row_selection();
    auto& current = *view->get_body().get_current();
    auto& visible = *view->get_body().get_table();
    auto row = table->index_from_source(0);
    REQUIRE(rows.push(row) == QValidator::Acceptable);
    REQUIRE(current.set(TableIndex(row, 2)) == QValidator::Acceptable);
    auto check_selection = [&] {
      auto row = table->index_from_source(0);
      REQUIRE(rows.get_size() == 1);
      REQUIRE(rows.get(0) == row);
      REQUIRE((current.get() == TableIndex(row, 2)));
      REQUIRE(visible.get<int>(row, 2) == 10);
    };
    source->set(0, 0, QString("B"));
    check_selection();
    source->set(0, 1, QString("USD"));
    check_selection();
    source->set(0, 0, QString("A"));
    check_selection();
    source->remove(0);
    REQUIRE(rows.get_size() == 0);
  }

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
    auto leaf = body.find_item(TableIndex(2, 0));
    REQUIRE(leaf);
    REQUIRE(leaf->get_body().layout()->itemAt(0)->widget()->isHidden());
    REQUIRE(view->get_header().get_items()->get(0).m_name == "Account (+1)");
    REQUIRE(view->get_header().get_item(1)->isHidden());
    auto ranges = std::vector<std::pair<int, int>>();
    view->get_header().get_filter()->connect_rows_signal(
      [&] (int first, int end) {
        ranges.emplace_back(first, end);
      });
    toggle(1, 0, "CAD");
    REQUIRE(visible.get_row_size() == 7);
    REQUIRE(visible.get<QString>(2, 0) == "USD (1)");
    toggle(0, 0, "A");
    REQUIRE(ranges == std::vector{std::pair(2, 3), std::pair(1, 5)});
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
      toggle(0, 0, "CAD");
      REQUIRE(visible.get_row_size() == 3);
      REQUIRE(visible.get<QString>(1, 1) == "USD (1)");
      table->set_grouping({});
      REQUIRE(visible.get_row_size() == 4);
      REQUIRE(visible.get<int>(0, 2) == 12);
    }
  }

  UI_TEST_CASE("drag_across_hidden_column") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), QString("CAD"), QString("Long"), 10});
    auto view = std::unique_ptr<TableView>(GroupedTableViewBuilder(source).
      add_column("Account").add_column("Currency").add_column("Side").
      add_column("Shares").set_grouping({0, 1}).make());
    auto controller = TableColumnReorderController(*view);
    auto& header = view->get_header();
    auto& widths = *header.get_widths();
    widths.set(0, scale_width(80));
    widths.set(1, scale_width(480));
    widths.set(2, scale_width(160));
    widths.set(3, scale_width(100));
    view->resize(scale(400, 200));
    view->show();
    QApplication::processEvents();
    auto account = header.get_item(0);
    auto currency = header.get_item(1);
    REQUIRE(currency->isHidden());
    move_cursor(*account, account->rect().center());
    auto receiver = QApplication::widgetAt(QCursor::pos());
    REQUIRE(receiver);
    press_mouse(*receiver, Qt::LeftButton);
    auto positions = std::vector<int>();
    auto drag_to = [&] (int x) {
      move_cursor(header, QPoint(scale_width(x), header.rect().center().y()));
      auto position = QCursor::pos();
      auto move = QMouseEvent(QEvent::MouseMove,
        receiver->mapFromGlobal(position), position, Qt::NoButton,
        Qt::LeftButton, Qt::NoModifier);
      QApplication::sendEvent(receiver, &move);
      positions.push_back(header.get_index(account).value());
    };
    drag_to(100);
    drag_to(110);
    drag_to(280);
    drag_to(60);
    release_mouse(*receiver, Qt::LeftButton);
    REQUIRE(positions == std::vector{2, 2, 3, 1});
    REQUIRE(currency->isHidden());
    REQUIRE(header.get_column_order()->get(1) == 0);
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
    auto group = header.get_item(0);
    move_cursor(*group, group->rect().center());
    auto position = QCursor::pos();
    auto move = QMouseEvent(QEvent::MouseMove,
      receiver->mapFromGlobal(position), position, Qt::NoButton,
      Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(receiver, &move);
    release_mouse(*receiver, Qt::LeftButton);
    QApplication::processEvents();
    REQUIRE(header.get_item(0) == shares);
    REQUIRE(header.get_column_order()->get(0) == 2);
    REQUIRE(rows.get<QString>(0, 0) == "A (2)");
    move_cursor(*shares, shares->rect().center());
    press_mouse(*shares, Qt::LeftButton);
    release_mouse(*shares, Qt::LeftButton);
    REQUIRE(header.get_items()->get(0).m_order ==
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
    table->set_grouping({1});
    REQUIRE(header.get_items()->get(1).m_name == "Currency");
    REQUIRE(header.get_items()->get(2).m_name == "Account");
    REQUIRE_FALSE(header.get_item(1)->isHidden());
    table->set_grouping({});
    REQUIRE(header.get_items()->get(0).m_name == "Shares");
    REQUIRE(header.get_items()->get(1).m_name == "Currency");
  }

  UI_TEST_CASE("aggregate_row_style") {
    auto source = std::make_shared<ArrayTableModel>();
    for(auto i = 0; i != 32; ++i) {
      source->push({QString::number(i), QString("CAD"), i});
    }
    auto view = std::unique_ptr<TableView>(GroupedTableViewBuilder(source).
      add_column("Account").add_column("Currency").add_column("Shares").
      set_grouping({0, 1}).make());
    view->resize(scale(500, 185));
    view->show();
    QApplication::processEvents();
    auto& body = view->get_body();
    auto row = [&] (int index) {
      auto item = body.find_item(TableIndex(index, 0));
      REQUIRE(item);
      return item->parentWidget();
    };
    auto check = [&] (int index, bool aggregate) {
      REQUIRE(is_match(*row(index), AggregateRow()) == aggregate);
    };
    check(0, true);
    check(1, true);
    check(2, false);
    REQUIRE((find<BackgroundColor>(get_computed_block(*row(1))) ==
      BackgroundColor(QColor(0, 0, 0, 16))));
    auto fill = QColor(0xB9DAFF);
    auto selected = QColor(0xE7CAFF);
    auto grid = QColor(0x37618C);
    update_style(*view, [&] (auto& style) {
      auto body = Any() > is_a<TableBody>();
      style.get(body > AggregateRow()).
        set(BackgroundColor(fill)).
        set(HorizontalGridColor(grid));
      style.get(body > (AggregateRow() && Selected())).
        set(BackgroundColor(selected));
    });
    auto color = [&] (int index) {
      QApplication::processEvents();
      auto widget = row(index);
      auto point = QPoint(widget->width() - scale_width(5),
        widget->height() / 2);
      auto area = QRect(widget->mapTo(&body, point), QSize(1, 1));
      return body.grab(area).toImage().pixelColor(0, 0);
    };
    REQUIRE(color(0) == fill);
    REQUIRE(color(1) == fill);
    REQUIRE((find<HorizontalGridColor>(get_computed_block(*row(1))) ==
      HorizontalGridColor(grid)));
    REQUIRE(color(2) != fill);
    view->get_selection()->get_row_selection()->push(1);
    REQUIRE(color(1) == selected);
    auto toggle = dynamic_cast<ToggleButton*>(
      body.find_item(TableIndex(0, 0))->get_body().focusProxy());
    REQUIRE(toggle);
    toggle->get_current()->set(false);
    QApplication::processEvents();
    check(1, true);
    check(2, true);
    check(3, false);
    auto& scroll = view->get_scroll_box().get_vertical_scroll_bar();
    scroll.set_position(scroll.get_range().m_end);
    QApplication::processEvents();
    auto last = body.get_table()->get_row_size() - 1;
    check(last - 2, true);
    check(last - 1, true);
    check(last, false);
    auto table = std::static_pointer_cast<GroupedTableModel>(view->get_table());
    table->set_grouping({});
    scroll.set_position(0);
    QApplication::processEvents();
    check(0, false);
    check(1, false);
    REQUIRE(color(0) != fill);
  }

  UI_TEST_CASE("grouped_column_visibility") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), QString("Long")});
    source->push({QString("B"), QString("Short")});
    auto view = std::unique_ptr<TableView>(GroupedTableViewBuilder(source).
      add_column("Account", "Acct").add_column("Side").make());
    view->resize(400, 300);
    view->show();
    QApplication::processEvents();
    auto table = std::static_pointer_cast<GroupedTableModel>(view->get_table());
    auto& header = view->get_header();
    auto& items = *header.get_items();
    auto leaf = [&] (int column) {
      QApplication::processEvents();
      auto item = view->get_body().find_item(
        TableIndex(table->index_from_source(0), column));
      REQUIRE(item);
      return item;
    };
    auto label = [&] (int column) {
      return leaf(column)->get_body().layout()->itemAt(0)->widget();
    };
    table->set_grouping({1});
    REQUIRE(items.get(0).m_name == "Side");
    REQUIRE(header.get_column_order()->get(0) == 1);
    REQUIRE(table->get_group_name(0) == "Long");
    REQUIRE(label(0)->isHidden());
    REQUIRE(label(1)->isVisible());
    items.move(0, 1);
    table->set_grouping({1, 0});
    REQUIRE(header.get_item(0)->isHidden());
    REQUIRE_FALSE(header.get_item(1)->isHidden());
    REQUIRE(leaf(0)->width() == 0);
    REQUIRE(table->get_group_name(1) == "A");
    table->set_grouping({0, 1});
    REQUIRE(header.get_item(0)->isHidden());
    REQUIRE_FALSE(header.get_item(1)->isHidden());
    REQUIRE(items.get(1).m_name == "Account (+1)");
    REQUIRE(items.get(1).m_short_name == "Acct (+1)");
    table->set_grouping({});
    REQUIRE(items.get(0).m_name == "Side");
    REQUIRE(items.get(1).m_name == "Account");
    REQUIRE(items.get(1).m_short_name == "Acct");
    REQUIRE_FALSE(header.get_item(1)->isHidden());
    REQUIRE(label(0)->isVisible());
    REQUIRE(label(1)->isVisible());
    REQUIRE(leaf(0)->width() > 0);
    view->hide_column(1);
    table->set_grouping({0});
    REQUIRE(header.get_column_order()->get(0) == 0);
    REQUIRE_FALSE(header.get_item(0)->isHidden());
    table->set_grouping({});
    REQUIRE(header.get_item(0)->isHidden());
  }

  UI_TEST_CASE("grouped_header_rename") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), QString("CAD"), 10});
    auto view = std::unique_ptr<TableView>(GroupedTableViewBuilder(source).
      add_column("Account", "Acct").add_column("Currency", "Ccy").
      add_column("Shares").set_grouping({0, 1}).make());
    auto table = std::static_pointer_cast<GroupedTableModel>(view->get_table());
    auto& items = *view->get_header().get_items();
    auto account = items.get(0);
    account.m_name = "Desk (+1)";
    items.set(0, account);
    auto currency = items.get(1);
    currency.m_short_name = "FX";
    items.set(1, currency);
    table->set_grouping({0, 1, 2});
    REQUIRE(items.get(0).m_name == "Desk (+1) (+2)");
    REQUIRE(items.get(0).m_short_name == "Acct (+2)");
    table->set_grouping({1, 0});
    REQUIRE(items.get(0).m_short_name == "FX (+1)");
    auto renamed = items.get(0);
    renamed.m_short_name.clear();
    items.set(0, renamed);
    table->set_grouping({});
    REQUIRE(items.get(0).m_name == "Currency");
    REQUIRE(items.get(0).m_short_name.isEmpty());
    REQUIRE(items.get(1).m_name == "Desk (+1)");
    REQUIRE(items.get(1).m_short_name == "Acct");
  }

  UI_TEST_CASE("heading_edits") {
    struct Editor : TextBox {
      std::shared_ptr<TableModel> m_table;

      Editor(const std::shared_ptr<TableModel>& table, int row, int column)
        : TextBox(make_table_value_model<QString>(table, row, column)),
          m_table(table) {}
    };
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("Account"), QString("CAD")});
    auto view = std::unique_ptr<TableView>(GroupedTableViewBuilder(source).
      add_column("Account").add_column("Currency").set_grouping({0}).
      set_item_builder([] (const auto& table, int row, int column) {
        return new Editor(table, row, column);
      }).make());
    view->show();
    QApplication::processEvents();
    auto editor = [&] (int row, int column, int index) {
      auto item = view->get_body().find_item(TableIndex(row, column));
      REQUIRE(item);
      auto widget = item->get_body().layout()->itemAt(index)->widget();
      auto editor = dynamic_cast<Editor*>(widget);
      REQUIRE(editor);
      return editor;
    };
    auto heading = editor(0, 0, 1);
    auto table = heading->m_table;
    auto key = heading->get_current();
    REQUIRE(key->set(QString("Other")) == QValidator::Invalid);
    REQUIRE(table->remove(0) == QValidator::Invalid);
    REQUIRE(source->get<QString>(0, 0) == "Account");
    auto leaf = editor(1, 1, 0);
    REQUIRE(leaf->m_table == source);
    auto currency = make_table_value_model<QString>(table, 0, 1);
    auto updates = 0;
    auto connection = currency->connect_update_signal([&] (const QString&) {
      ++updates;
    });
    REQUIRE(
      leaf->get_current()->set(QString("USD")) == QValidator::Acceptable);
    REQUIRE(currency->get() == "USD");
    REQUIRE(updates == 1);
  }

  UI_TEST_CASE("heading_item_builder") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), QString("CA"), 10});
    source->push({QString("B"), QString("US"), 20});
    source->push({QString("A"), QString("CA"), 30});
    auto view = std::unique_ptr<TableView>(GroupedTableViewBuilder(source).
      add_column("Account").add_column("Country").add_column("Shares").
      set_item_builder([=] (const std::shared_ptr<TableModel>& table, int row,
          int column) -> QWidget* {
        if(column != 1) {
          return TableView::default_item_builder(table, row, column);
        }
        REQUIRE(table->get<QString>(row, column) ==
          source->get<QString>(row, column));
        auto flag = new Icon(QImage());
        flag->setAccessibleName(table->get<QString>(row, column));
        flag->setToolTip(QString::number(table->get<int>(row, 2)));
        flag->setFixedSize(scale(16, 10));
        return flag;
      }).set_grouping({0, 1}).make());
    view->get_header().get_widths()->set(0, scale_width(180));
    view->resize(scale(500, 350));
    view->show();
    QApplication::processEvents();
    auto heading = [&] (int row) {
      auto item = view->get_body().find_item(TableIndex(row, 0));
      REQUIRE(item);
      return item->get_body().layout();
    };
    auto flag = [&] (int row) {
      auto icon = dynamic_cast<Icon*>(heading(row)->itemAt(1)->widget());
      REQUIRE(icon);
      return icon;
    };
    auto count = [&] (int row) {
      auto label = dynamic_cast<TextBox*>(heading(row)->itemAt(3)->widget());
      REQUIRE(label);
      return label->get_current()->get();
    };
    auto account = dynamic_cast<TextBox*>(heading(0)->itemAt(1)->widget());
    REQUIRE(account);
    REQUIRE(account->get_current()->get() == "A");
    REQUIRE(count(0) == "(2)");
    auto first = flag(1);
    REQUIRE(first->accessibleName() == "CA");
    REQUIRE(first->toolTip() == "10");
    REQUIRE(first->isVisible());
    auto count_label = heading(1)->itemAt(3)->widget();
    REQUIRE(count_label->x() == first->x() + first->width() + scale_width(4));
    REQUIRE(count(1) == "(2)");
    auto toggle =
      dynamic_cast<ToggleButton*>(heading(1)->itemAt(0)->widget());
    REQUIRE(toggle);
    toggle->get_current()->set(false);
    SUBCASE("insert_first_leaf") {
      source->insert({QString("A"), QString("CA"), 5}, 0);
      REQUIRE(flag(1)->toolTip() == "5");
      REQUIRE(count(1) == "(3)");
    }
    SUBCASE("sort_first_leaf") {
      auto table =
        std::static_pointer_cast<GroupedTableModel>(view->get_table());
      table->set_column_order({{2, SortedTableModel::Ordering::DESCENDING}});
      REQUIRE(flag(1)->toolTip() == "30");
      REQUIRE(count(1) == "(2)");
      source->set(0, 2, 50);
      REQUIRE(flag(1)->toolTip() == "50");
    }
    SUBCASE("remove_first_leaf") {
      source->push({QString("A"), QString("CA"), 40});
      REQUIRE(flag(1) == first);
      REQUIRE(count(0) == "(3)");
      REQUIRE(count(1) == "(3)");
      source->remove(0);
      REQUIRE(flag(1)->accessibleName() == "CA");
      REQUIRE(flag(1)->toolTip() == "30");
      REQUIRE(flag(1)->isVisible());
      REQUIRE(count(1) == "(2)");
      source->set(1, 1, QString("US"));
      REQUIRE(flag(1)->toolTip() == "40");
      REQUIRE(count(1) == "(1)");
      REQUIRE(count(0) == "(2)");
    }
  }

  UI_TEST_CASE("item_builders") {
    using Highlight = StateSelector<void, struct HighlightTag>;
    struct LeafBuilder {
      std::shared_ptr<TableModel> m_source;
      int& m_reused;

      std::optional<Selector> get_row_selector(int row) const {
        if(row == 2) {
          return Highlight();
        }
        return std::nullopt;
      }

      QWidget* mount(
          const std::shared_ptr<TableModel>& table, int row, int column) {
        auto label = new QLabel();
        bind(*label, table, row, column);
        return label;
      }

      void reset(QWidget& widget, const std::shared_ptr<TableModel>& table,
          int row, int column) {
        ++m_reused;
        bind(static_cast<QLabel&>(widget), table, row, column);
      }

      void unmount(QWidget* widget) {
        delete widget;
      }

      void bind(QLabel& label, const std::shared_ptr<TableModel>& table,
          int row, int column) {
        if(column != 0) {
          REQUIRE(table == m_source);
        }
        if(column == 2) {
          label.setNum(table->get<int>(row, column));
        } else {
          label.setText(table->get<QString>(row, column));
        }
      }
    };
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), QString("CAD"), 10});
    source->push({QString("B"), QString("USD"), 40});
    source->push({QString("A"), QString("USD"), 20});
    auto reused = 0;
    auto leaves = RecycledTableViewItemBuilder(LeafBuilder(source, reused));
    auto builder = GroupedTableViewBuilder(source).
      add_column("Account").add_column("Currency").add_column("Shares").
      set_item_builder(std::move(leaves)).set_grouping({0});
    auto custom = false;
    SUBCASE("default_aggregates") {}
    SUBCASE("custom_aggregates") {
      custom = true;
      builder.set_aggregate_item_builder(
        [=] (const std::shared_ptr<TableModel>& table, int row, int column) {
          auto grouped = std::dynamic_pointer_cast<GroupedTableModel>(table);
          REQUIRE(grouped);
          REQUIRE(grouped->get_source() == source);
          REQUIRE(grouped->index_to_source(row) == -1);
          REQUIRE(column != 0);
          auto item = TableView::default_item_builder(table, row, column);
          item->setToolTip(grouped->get_group_name(row));
          return item;
        });
    }
    auto view = std::unique_ptr<TableView>(builder.make());
    view->resize(500, 350);
    view->show();
    QApplication::processEvents();
    auto& body = view->get_body();
    auto cell = [&] (int row, int column) {
      auto item = body.find_item(TableIndex(row, column));
      REQUIRE(item);
      return item->get_body().layout()->itemAt(0)->widget();
    };
    auto text = [&] (int row, int column) {
      auto widget = cell(row, column);
      if(auto label = dynamic_cast<QLabel*>(widget)) {
        return label->text();
      }
      auto label = dynamic_cast<TextBox*>(widget);
      REQUIRE(label);
      return label->get_current()->get();
    };
    REQUIRE(text(2, 2) == "20");
    auto leaf = body.find_item(TableIndex(2, 2));
    REQUIRE(is_match(*leaf->parentWidget(), Highlight()));
    REQUIRE(text(0, 1) == "(2)");
    REQUIRE(text(0, 2) == "30");
    if(custom) {
      REQUIRE(cell(0, 2)->toolTip() == "A");
    }
    auto toggle = dynamic_cast<ToggleButton*>(cell(0, 0));
    REQUIRE(toggle);
    auto expanded = toggle->get_current();
    expanded->set(false);
    QApplication::processEvents();
    REQUIRE(text(2, 2) == "40");
    REQUIRE(text(1, 2) == "40");
    if(custom) {
      REQUIRE(cell(1, 2)->toolTip() == "B");
    }
    auto shares = cell(0, 2);
    source->set(2, 2, 25);
    REQUIRE(cell(0, 2) == shares);
    REQUIRE(text(0, 2) == "35");
    source->set(2, 1, QString("CAD"));
    REQUIRE(text(0, 1) == "(1)");
    expanded->set(true);
    QApplication::processEvents();
    REQUIRE(reused > 0);
    REQUIRE(text(2, 2) == "25");
    auto grouped =
      std::static_pointer_cast<GroupedTableModel>(view->get_table());
    grouped->set_column_order({{2, SortedTableModel::Ordering::DESCENDING}});
    QApplication::processEvents();
    REQUIRE(text(0, 2) == "40");
    if(custom) {
      REQUIRE(cell(0, 2)->toolTip() == "B");
    }
    source->set(1, 2, 45);
    REQUIRE(text(0, 2) == "45");
    grouped->set_column_order({});
    grouped->set_grouping({});
    QApplication::processEvents();
    REQUIRE(text(1, 0) == "B");
    REQUIRE(text(2, 2) == "25");
  }

  UI_TEST_CASE("column_order_settings") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({QString("A"), 3});
    source->push({QString("B"), 8});
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
    source->push({QString("A"), 10, 5});
    source->push({QString("B"), 30, 9});
    source->push({QString("A"), 20, 2});
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
    auto enabled = button->grab().toImage();
    bar->setDisabled(true);
    REQUIRE(button->grab().toImage() != enabled);
    bar->setEnabled(true);
    REQUIRE(button->grab().toImage() == enabled);
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
    select(2);
    auto& headers = *view->get_header().get_items();
    REQUIRE(headers.get(0).m_name == "Account (+2)");
    REQUIRE(headers.get(0).m_short_name == "Acct (+2)");
    select(2);
    REQUIRE(headers.get(0).m_name == "Account (+1)");
    remove(0);
    REQUIRE(headers.get(0).m_name == "Currency");
    REQUIRE(headers.get(0).m_short_name.isEmpty());
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
