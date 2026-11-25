#include <QApplication>
#include <QLayout>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Ui/GroupedTableModel.hpp"
#include "Spire/Ui/GroupedTableView.hpp"
#include "Spire/Ui/TableItem.hpp"
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
    view->resize(500, 400);
    view->show();
    QApplication::processEvents();
    auto& body = view->get_body();
    auto& visible = *body.get_table();
    auto toggle = [&] (int row, int column) {
      QApplication::processEvents();
      auto item = body.find_item(TableIndex(row, column));
      REQUIRE(item);
      auto button = dynamic_cast<ToggleButton*>(item->get_body().focusProxy());
      REQUIRE(button);
      auto target = button->focusProxy();
      REQUIRE(target);
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
    toggle(1, 0);
    REQUIRE(visible.get_row_size() == 7);
    REQUIRE(visible.get<QString>(2, 0) == "USD (1)");
    toggle(0, 0);
    REQUIRE(visible.get_row_size() == 4);
    REQUIRE(visible.get<QString>(1, 0) == "B (1)");
    source->set(0, 2, 12);
    source->push({QString("A"), QString("CAD"), 5});
    REQUIRE(visible.get_row_size() == 4);
    REQUIRE(visible.get<QString>(0, 0) == "A (3)");
    REQUIRE(visible.get<std::int64_t>(0, 2) == 47);
    toggle(0, 0);
    REQUIRE(visible.get_row_size() == 7);
    REQUIRE(visible.get<QString>(1, 0) == "CAD (2)");
    REQUIRE(visible.get<std::int64_t>(1, 2) == 17);
    toggle(1, 0);
    REQUIRE(visible.get_row_size() == 9);
    REQUIRE(visible.get<int>(2, 2) == 12);
    REQUIRE(visible.get<int>(3, 2) == 5);
    toggle(0, 0);
    auto table = std::static_pointer_cast<GroupedTableModel>(view->get_table());
    table->set_grouping({0});
    REQUIRE(visible.get_row_size() == 3);
    table->set_grouping({1});
    REQUIRE(visible.get_row_size() == 6);
    toggle(0, 1);
    REQUIRE(visible.get_row_size() == 3);
    REQUIRE(visible.get<QString>(1, 1) == "USD (1)");
    table->set_grouping({});
    REQUIRE(visible.get_row_size() == 4);
    REQUIRE(visible.get<int>(0, 2) == 12);
  }
}
