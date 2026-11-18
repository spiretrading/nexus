#include <QApplication>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/TableCurrentIndexModel.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;
using namespace Spire::Tests;

namespace {
  template<typename T>
  struct FixedSelection : ArrayListModel<T> {
    using ArrayListModel<T>::ArrayListModel;

    QValidator::State remove(int index) override {
      return QValidator::Invalid;
    }
  };

  struct MinimumFilter : EmptyTableFilter {
    int m_minimum = 1;
    mutable FilterSignal m_filter_signal;

    bool is_filtered(const TableModel& table, int row) const override {
      return table.get<int>(row, 0) < m_minimum;
    }

    boost::signals2::connection connect_filter_signal(
        const FilterSignal::slot_type& slot) const override {
      return m_filter_signal.connect(slot);
    }

    void set_minimum(int minimum) {
      m_minimum = minimum;
      m_filter_signal(0, Filter::FILTERED);
    }
  };
}

TEST_SUITE("TableView") {
  UI_TEST_CASE("retained_body_selection") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({0});
    auto filter = std::make_shared<MinimumFilter>();
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("Value").set_filter(filter).make());
    SUBCASE("initial_filter") {}
    SUBCASE("updated_filter") {
      filter->set_minimum(2);
    }
    auto source = view->get_selection();
    auto displayed = view->get_body().get_selection();
    auto weak_filter = std::weak_ptr(filter);
    filter.reset();
    view.reset();
    REQUIRE(!weak_filter.expired());
    table->push({0});
    table->push({5});
    REQUIRE(displayed->get_row_selection()->push(0) == QValidator::Acceptable);
    REQUIRE(source->get_row_selection()->get(0) == 2);
    displayed.reset();
    REQUIRE(weak_filter.expired());
  }

  UI_TEST_CASE("select_sorted_row") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({30});
    table->push({10});
    auto view =
      std::unique_ptr<TableView>(TableViewBuilder(table).add_header_item(
        "Value", TableHeaderItem::Order::ASCENDING).make());
    view->resize(320, 200);
    view->show();
    QApplication::processEvents();
    auto cell = view->get_body().find_item(TableIndex(0, 0));
    REQUIRE(cell);
    move_cursor(*cell, cell->rect().center());
    press_mouse(*cell, Qt::LeftButton);
    release_mouse(*cell, Qt::LeftButton);
    auto& rows = *view->get_selection()->get_row_selection();
    REQUIRE(rows.get_size() == 1);
    REQUIRE(rows.get(0) == 1);
    auto current = view->get_current()->get();
    REQUIRE(current.has_value());
    REQUIRE(*current == TableIndex(1, 0));
  }

  UI_TEST_CASE("sorted_range_selection") {
    auto table = std::make_shared<ArrayTableModel>();
    for(auto value : {10, 20, 30, 40}) {
      table->push({value});
    }
    auto rows = std::make_shared<ArrayListModel<int>>();
    auto selection = std::make_shared<TableSelectionModel>(
      std::make_shared<ArrayListModel<TableIndex>>(), rows,
      std::make_shared<ArrayListModel<int>>());
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("Value", TableHeaderItem::Order::ASCENDING).
      set_selection(selection).make());
    view->resize(320, 240);
    view->show();
    QApplication::processEvents();
    auto click = [&] (QWidget& widget) {
      move_cursor(widget, widget.rect().center());
      press_mouse(widget, Qt::LeftButton);
      release_mouse(widget, Qt::LeftButton);
    };
    auto& body = view->get_body();
    click(*body.find_item(TableIndex(0, 0)));
    click(*view->get_header().get_item(0));
    QApplication::processEvents();
    REQUIRE(body.get_table()->get<int>(3, 0) == 10);
    REQUIRE(rows->get_size() == 1);
    REQUIRE(rows->get(0) == 0);
    auto shift_click = [&] {
      press_key(body, Qt::Key_Shift);
      click(*body.find_item(TableIndex(1, 0)));
      release_key(body, Qt::Key_Shift);
      auto current = view->get_current()->get();
      REQUIRE(current.has_value());
      REQUIRE(*current == TableIndex(2, 0));
    };
    SUBCASE("retained_anchor") {
      shift_click();
      REQUIRE(rows->get_size() == 3);
      REQUIRE(rows->get(0) == 0);
      REQUIRE(rows->get(1) == 1);
      REQUIRE(rows->get(2) == 2);
    }
    SUBCASE("deselected_anchor") {
      rows->remove(0);
      shift_click();
      REQUIRE(rows->get_size() == 1);
      REQUIRE(rows->get(0) == 2);
    }
  }

  UI_TEST_CASE("injected_current") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({10});
    table->push({20});
    auto current = std::make_shared<TableCurrentIndexModel>(table);
    auto builder = TableViewBuilder(table);
    SUBCASE("source_order") {
      builder.add_header_item("Value");
    }
    SUBCASE("sorted_and_filtered") {
      builder.add_header_item("Value", TableHeaderItem::Order::DESCENDING).
        set_filter(std::make_shared<MinimumFilter>());
    }
    auto view = std::unique_ptr<TableView>(builder.set_current(current).make());
    current->set(TableIndex(0, 0));
    auto& body = view->get_body();
    auto require_current = [&] (int source_row) {
      auto displayed = body.get_current()->get();
      REQUIRE(current->get().has_value());
      REQUIRE(current->get()->m_row == source_row);
      REQUIRE(displayed.has_value());
      REQUIRE(body.get_table()->get<int>(displayed->m_row, 0) == 10);
    };
    table->insert({5}, 0);
    require_current(1);
    table->transact([&] {
      table->insert({0}, 0);
      require_current(2);
      table->remove(0);
      require_current(1);
      table->remove(0);
      require_current(0);
    });
    table->remove(0);
    REQUIRE(!current->get().has_value());
    REQUIRE(!body.get_current()->get().has_value());
  }

  UI_TEST_CASE("filter_current_and_selection") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({10});
    table->push({30});
    table->push({20});
    auto filter = std::make_shared<MinimumFilter>();
    auto rows = std::make_shared<ArrayListModel<int>>();
    auto cells = std::make_shared<ArrayListModel<TableIndex>>();
    auto selection = std::make_shared<TableSelectionModel>(
      cells, rows, std::make_shared<ArrayListModel<int>>());
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("Value", TableHeaderItem::Order::ASCENDING).
      set_filter(filter).set_selection(selection).make());
    rows->push(1);
    rows->push(2);
    cells->push(TableIndex(2, 0));
    view->get_current()->set(TableIndex(2, 0));
    filter->set_minimum(15);
    auto current = view->get_body().get_current()->get();
    REQUIRE(current.has_value());
    REQUIRE(*current == TableIndex(0, 0));
    filter->set_minimum(1);
    current = view->get_body().get_current()->get();
    REQUIRE(current.has_value());
    REQUIRE(*current == TableIndex(1, 0));
    filter->set_minimum(25);
    REQUIRE(!view->get_current()->get().has_value());
    REQUIRE(!view->get_body().get_current()->get().has_value());
    REQUIRE(rows->get_size() == 1);
    REQUIRE(rows->get(0) == 1);
    REQUIRE(cells->get_size() == 0);
    filter->set_minimum(1);
    REQUIRE(!view->get_current()->get().has_value());
    REQUIRE(rows->get_size() == 1);
    REQUIRE(cells->get_size() == 0);
  }

  UI_TEST_CASE("filter_fixed_selection") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({10});
    table->push({20});
    auto filter = std::make_shared<MinimumFilter>();
    auto selection = std::make_shared<TableSelectionModel>(
      std::make_shared<ArrayListModel<TableIndex>>(),
      std::make_shared<FixedSelection<int>>(std::vector{0, 1}),
      std::make_shared<ArrayListModel<int>>());
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("Value").set_filter(filter).
      set_selection(selection).make());
    auto& rows = *selection->get_row_selection();
    REQUIRE(rows.get_size() == 2);
    filter->set_minimum(15);
    REQUIRE(rows.get_size() == 1);
    REQUIRE(rows.get(0) == 1);
    filter->set_minimum(1);
    REQUIRE(rows.get_size() == 1);
    filter->set_minimum(25);
    REQUIRE(rows.get_size() == 0);
    REQUIRE(view->get_body().get_selection()->get_row_selection()->
      get_size() == 0);
  }

  UI_TEST_CASE("remove_fixed_selection") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({10});
    table->push({20});
    table->push({30});
    auto selection = std::make_shared<TableSelectionModel>(
      std::make_shared<FixedSelection<TableIndex>>(
        std::vector{TableIndex(0, 0), TableIndex(2, 0)}),
      std::make_shared<FixedSelection<int>>(std::vector{0, 2}),
      std::make_shared<ArrayListModel<int>>());
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("Value").set_selection(selection).make());
    auto& displayed = *view->get_body().get_selection();
    REQUIRE(selection->get_row_selection()->remove(0) == QValidator::Invalid);
    table->remove(0);
    for(auto model : {selection.get(), &displayed}) {
      auto& rows = *model->get_row_selection();
      auto& items = *model->get_item_selection();
      REQUIRE(rows.get_size() == 1);
      REQUIRE(rows.get(0) == 1);
      REQUIRE(items.get_size() == 1);
      REQUIRE(items.get(0) == TableIndex(1, 0));
    }
    table->remove(1);
    table->push({40});
    REQUIRE(selection->get_row_selection()->get_size() == 0);
    REQUIRE(selection->get_item_selection()->get_size() == 0);
    REQUIRE(displayed.get_row_selection()->get_size() == 0);
    REQUIRE(displayed.get_item_selection()->get_size() == 0);
  }

  UI_TEST_CASE("reused_selection") {
    auto first = std::make_shared<ArrayTableModel>();
    first->push({10});
    auto rows = std::make_shared<ArrayListModel<int>>(std::vector{0});
    auto cells = std::make_shared<ArrayListModel<TableIndex>>(
      std::vector{TableIndex(0, 0)});
    auto selection = std::make_shared<TableSelectionModel>(
      cells, rows, std::make_shared<ArrayListModel<int>>());
    auto filter = std::make_shared<MinimumFilter>();
    auto first_view = std::unique_ptr<TableView>(TableViewBuilder(first).
      add_header_item("Value").set_filter(filter).
      set_selection(selection).make());
    auto& selected_rows = *selection->get_row_selection();
    SUBCASE("same_table") {
      auto view = std::unique_ptr<TableView>(TableViewBuilder(first).
        add_header_item("Value").set_selection(selection).make());
      first->insert({5}, 0);
      REQUIRE(rows->get(0) == 1);
      REQUIRE(cells->get(0) == TableIndex(1, 0));
      REQUIRE(selected_rows.push(0) == QValidator::Acceptable);
      filter->set_minimum(10);
      REQUIRE(rows->get_size() == 1);
      REQUIRE(rows->get(0) == 1);
      REQUIRE(selected_rows.push(0) == QValidator::Invalid);
    }
    SUBCASE("different_table") {
      SUBCASE("closed_view") {
        first_view.reset();
      }
      SUBCASE("open_view") {}
      auto second = std::make_shared<ArrayTableModel>();
      second->push({20});
      auto second_view = std::unique_ptr<TableView>(TableViewBuilder(second).
        add_header_item("Value").set_selection(selection).make());
      first->insert({5}, 0);
      REQUIRE(rows->get_size() == 1);
      REQUIRE(rows->get(0) == 0);
      REQUIRE(cells->get(0) == TableIndex(0, 0));
      filter->set_minimum(100);
      second->insert({15}, 0);
      REQUIRE(rows->get_size() == 1);
      REQUIRE(rows->get(0) == 1);
      REQUIRE(cells->get_size() == 1);
      REQUIRE(cells->get(0) == TableIndex(1, 0));
      auto& displayed = *second_view->get_body().get_selection();
      REQUIRE(displayed.get_row_selection()->get(0) == 1);
      REQUIRE(displayed.get_item_selection()->get(0) == TableIndex(1, 0));
      REQUIRE(selected_rows.push(0) == QValidator::Acceptable);
      REQUIRE(rows->get_size() == 2);
    }
  }

  UI_TEST_CASE("source_selection") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({30});
    table->push({0});
    table->push({10});
    auto rows = std::make_shared<ArrayListModel<int>>();
    auto cells = std::make_shared<ArrayListModel<TableIndex>>();
    auto columns = std::make_shared<ArrayListModel<int>>();
    auto selection =
      std::make_shared<TableSelectionModel>(cells, rows, columns);
    rows->push(0);
    cells->push(TableIndex(0, 0));
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("Value", TableHeaderItem::Order::ASCENDING).
      set_filter(std::make_shared<MinimumFilter>()).
      set_selection(selection).make());
    auto& displayed = *view->get_body().get_selection();
    REQUIRE(view->get_selection() == selection);
    REQUIRE(displayed.get_column_selection() == columns);
    REQUIRE(rows->get(0) == 0);
    REQUIRE(displayed.get_row_selection()->get(0) == 1);
    REQUIRE(displayed.get_item_selection()->get(0) == TableIndex(1, 0));
    REQUIRE(selection->get_row_selection()->push(1) == QValidator::Invalid);
    REQUIRE(selection->get_item_selection()->set(0, TableIndex(1, 0)) ==
      QValidator::Invalid);
    REQUIRE(rows->get_size() == 1);
    rows->push(1);
    REQUIRE(selection->get_row_selection()->get_size() == 1);
    rows->remove(1);
    rows->push(2);
    REQUIRE(displayed.get_row_selection()->get_size() == 2);
    REQUIRE(displayed.get_row_selection()->get(1) == 0);
    displayed.get_row_selection()->remove(1);
    REQUIRE(rows->get_size() == 1);
    REQUIRE(rows->get(0) == 0);
    displayed.get_row_selection()->set(0, 0);
    displayed.get_item_selection()->set(0, TableIndex(0, 0));
    REQUIRE(rows->get(0) == 2);
    REQUIRE(cells->get(0) == TableIndex(2, 0));
    table->set(2, 0, 40);
    REQUIRE(rows->get(0) == 2);
    REQUIRE(displayed.get_row_selection()->get(0) == 1);
    table->set(2, 0, -1);
    REQUIRE(displayed.get_row_selection()->get_size() == 0);
    REQUIRE(displayed.get_item_selection()->get_size() == 0);
    REQUIRE(rows->get_size() == 0);
    REQUIRE(cells->get_size() == 0);
    table->set(2, 0, 10);
    REQUIRE(displayed.get_row_selection()->get_size() == 0);
    REQUIRE(displayed.get_item_selection()->get_size() == 0);
    rows->push(2);
    cells->push(TableIndex(2, 0));
    table->insert({50}, 0);
    REQUIRE(rows->get(0) == 3);
    table->move(3, 0);
    REQUIRE(cells->get(0) == TableIndex(0, 0));
    table->remove(0);
    REQUIRE(rows->get_size() == 0);
    REQUIRE(cells->get_size() == 0);
    REQUIRE(displayed.get_row_selection()->get_size() == 0);
  }
}
