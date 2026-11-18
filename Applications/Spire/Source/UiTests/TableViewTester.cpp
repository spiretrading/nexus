#include <QApplication>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/Spire/TableCurrentIndexModel.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/MultiSelectionModel.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Tests;

namespace {
  template<typename T>
  class CountingSelection : public ArrayListModel<T> {
    public:
      mutable int m_previous_reads = 0;

      const T& get(int index) const override {
        if(index < this->get_size() - 1) {
          ++m_previous_reads;
        }
        return ArrayListModel<T>::get(index);
      }
  };

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

    connection connect_filter_signal(
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
    auto selection = std::make_shared<TableSelectionModel>(
      std::make_shared<ArrayListModel<TableIndex>>(),
      std::make_shared<ArrayListModel<int>>(),
      std::make_shared<ArrayListModel<int>>());
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("Value").set_filter(filter).
      set_selection(selection).make());
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
    table->insert({5}, 0);
    auto rows = displayed->get_row_selection();
    REQUIRE(rows->get(0) == 1);
    REQUIRE(rows->push(0) == QValidator::Acceptable);
    REQUIRE(rows->get_size() == 2);
    REQUIRE(rows->get(0) == 1);
    REQUIRE(rows->get(1) == 0);
    auto items = displayed->get_item_selection();
    auto columns = displayed->get_column_selection();
    REQUIRE(items->push(TableIndex(1, 0)) == QValidator::Acceptable);
    SUBCASE("retained_lists") {
      displayed.reset();
    }
    table->remove(3);
    REQUIRE(source->get_row_selection()->get_size() == 1);
    REQUIRE(rows->get_size() == 1);
    REQUIRE(rows->get(0) == 0);
    REQUIRE(source->get_item_selection()->get_size() == 0);
    REQUIRE(items->get_size() == 0);
    rows.reset();
    items.reset();
    displayed.reset();
    REQUIRE(weak_filter.expired());
    source.reset();
    selection.reset();
    REQUIRE(columns->push(0) == QValidator::Acceptable);
  }

  UI_TEST_CASE("append_selection") {
    auto table = std::make_shared<ArrayTableModel>();
    const auto SIZE = 64;
    for(auto i = 0; i != SIZE; ++i) {
      table->push({i});
    }
    auto rows = std::make_shared<CountingSelection<int>>();
    auto selection = std::make_shared<TableSelectionModel>(
      std::make_shared<ArrayListModel<TableIndex>>(), rows,
      std::make_shared<ArrayListModel<int>>());
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("Value", TableHeaderItem::Order::DESCENDING).
      set_selection(selection).make());
    auto& displayed = *view->get_body().get_selection()->get_row_selection();
    auto append = [&] {
      for(auto i = 0; i != SIZE; ++i) {
        rows->push(i);
        REQUIRE(displayed.get_size() == i + 1);
        REQUIRE(displayed.get(i) == SIZE - i - 1);
      }
    };
    SUBCASE("individual") {
      append();
    }
    SUBCASE("transaction") {
      rows->transact(append);
    }
    REQUIRE(rows->m_previous_reads == 0);
  }

  UI_TEST_CASE("clear_selection_on_click") {
    const auto SIZE = 64;
    auto table = std::make_shared<ArrayTableModel>();
    for(auto i = 0; i <= SIZE; ++i) {
      table->push({i});
    }
    auto rows = std::make_shared<CountingSelection<int>>();
    auto cells = std::make_shared<CountingSelection<TableIndex>>();
    auto selection = std::make_shared<TableSelectionModel>(
      cells, rows, std::make_shared<ArrayListModel<int>>());
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("Value", TableHeaderItem::Order::DESCENDING).
      set_selection(selection).make());
    view->resize(320, 240);
    view->show();
    QApplication::processEvents();
    for(auto i = 0; i < SIZE; ++i) {
      rows->push(i);
      cells->push(TableIndex(i, 0));
    }
    rows->m_previous_reads = 0;
    cells->m_previous_reads = 0;
    auto& body = view->get_body();
    auto item = body.find_item(TableIndex(0, 0));
    REQUIRE(item);
    move_cursor(*item, item->rect().center());
    press_mouse(*item, Qt::LeftButton);
    release_mouse(*item, Qt::LeftButton);
    REQUIRE(rows->get_size() == 1);
    REQUIRE(rows->get(0) == SIZE);
    REQUIRE(cells->get_size() == 1);
    REQUIRE(cells->get(0) == TableIndex(SIZE, 0));
    auto& displayed = *body.get_selection();
    REQUIRE(displayed.get_row_selection()->get_size() == 1);
    REQUIRE(displayed.get_row_selection()->get(0) == 0);
    REQUIRE(displayed.get_item_selection()->get_size() == 1);
    REQUIRE(displayed.get_item_selection()->get(0) == TableIndex(0, 0));
    REQUIRE(rows->m_previous_reads == 0);
    REQUIRE(cells->m_previous_reads == 0);
  }

  UI_TEST_CASE("selection_on_row_insert") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({10});
    table->push({20});
    auto rows = std::make_shared<CountingSelection<int>>();
    auto cells = std::make_shared<CountingSelection<TableIndex>>();
    auto selection = std::make_shared<TableSelectionModel>(
      cells, rows, std::make_shared<ArrayListModel<int>>());
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("Value", TableHeaderItem::Order::DESCENDING).
      set_filter(std::make_shared<MinimumFilter>()).
      set_selection(selection).make());
    for(auto row : {0, 1}) {
      rows->push(row);
      cells->push(TableIndex(row, 0));
    }
    auto& displayed = *view->get_body().get_selection();
    rows->m_previous_reads = 0;
    cells->m_previous_reads = 0;
    table->push({0});
    REQUIRE(displayed.get_row_selection()->get(0) == 1);
    REQUIRE(displayed.get_item_selection()->get(1) == TableIndex(0, 0));
    auto row_reads = rows->m_previous_reads;
    auto cell_reads = cells->m_previous_reads;
    rows->m_previous_reads = 0;
    cells->m_previous_reads = 0;
    table->push({30});
    REQUIRE(displayed.get_row_selection()->get(0) == 2);
    REQUIRE(displayed.get_item_selection()->get(1) == TableIndex(1, 0));
    REQUIRE(rows->m_previous_reads == row_reads);
    REQUIRE(cells->m_previous_reads == cell_reads);
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
    auto shift_click = [&] (int row) {
      QApplication::processEvents();
      press_key(body, Qt::Key_Shift);
      click(*body.find_item(TableIndex(row, 0)));
      release_key(body, Qt::Key_Shift);
    };
    click(*body.find_item(TableIndex(0, 0)));
    SUBCASE("insertion_inside_range") {
      auto end = 4;
      SUBCASE("forward") {
        shift_click(2);
        table->insert({15}, 1);
      }
      SUBCASE("backward") {
        click(*body.find_item(TableIndex(3, 0)));
        shift_click(1);
        table->insert({25}, 2);
        end = 0;
      }
      auto& cells = std::as_const(*selection->get_item_selection());
      REQUIRE(rows->get_size() == 3);
      shift_click(end);
      REQUIRE(rows->get_size() == 5);
      for(auto row = 0; row != 5; ++row) {
        REQUIRE(std::ranges::contains(std::as_const(*rows), row));
        REQUIRE(std::ranges::contains(cells, TableIndex(row, 0)));
      }
    }
    SUBCASE("reordered_range") {
      table->push({50});
      auto& cells = *selection->get_item_selection();
      rows->push(4);
      cells.push(TableIndex(4, 0));
      shift_click(2);
      REQUIRE(rows->get_size() == 4);
      table->set(1, 0, 5);
      REQUIRE(body.get_table()->get<int>(0, 0) == 5);
      SUBCASE("extend") {
        shift_click(3);
      }
      SUBCASE("fill_gap") {
        table->set(3, 0, 25);
        REQUIRE(body.get_table()->get<int>(2, 0) == 25);
        shift_click(3);
      }
      REQUIRE(rows->get_size() == 4);
      REQUIRE(rows->get(0) == 0);
      REQUIRE(rows->get(1) == 4);
      REQUIRE(rows->get(2) == 2);
      REQUIRE(rows->get(3) == 3);
      for(auto row : {0, 2, 3, 4}) {
        REQUIRE(std::find(cells.begin(), cells.end(), TableIndex(row, 0)) !=
          cells.end());
      }
      for(auto& cell : std::as_const(cells)) {
        REQUIRE(cell.m_row != 1);
      }
    }
    SUBCASE("header_sort") {
      click(*view->get_header().get_item(0));
      QApplication::processEvents();
      REQUIRE(body.get_table()->get<int>(3, 0) == 10);
      REQUIRE(rows->get_size() == 1);
      REQUIRE(rows->get(0) == 0);
      SUBCASE("retained_anchor") {
        shift_click(1);
        REQUIRE(rows->get_size() == 3);
        REQUIRE(rows->get(0) == 0);
        REQUIRE(rows->get(1) == 1);
        REQUIRE(rows->get(2) == 2);
      }
      SUBCASE("deselected_anchor") {
        rows->remove(0);
        shift_click(1);
        REQUIRE(rows->get_size() == 1);
        REQUIRE(rows->get(0) == 2);
      }
      auto current = view->get_current()->get();
      REQUIRE(current.has_value());
      REQUIRE(*current == TableIndex(2, 0));
    }
  }

  UI_TEST_CASE("injected_current") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({10});
    table->push({20});
    auto current = make_proxy_value_model(
      std::make_shared<LocalValueModel<optional<TableIndex>>>());
    auto builder = TableViewBuilder(table).set_current(current);
    auto make_view = [&] {
      SUBCASE("source_order") {
        builder.add_header_item("Value");
      }
      SUBCASE("sorted_and_filtered") {
        builder.add_header_item("Value", TableHeaderItem::Order::DESCENDING).
          set_filter(std::make_shared<MinimumFilter>());
      }
      return std::unique_ptr<TableView>(builder.make());
    };
    auto view = std::unique_ptr<TableView>();
    SUBCASE("current_before_view") {
      current->set_source(std::make_shared<TableCurrentIndexModel>(table));
      view = make_view();
    }
    SUBCASE("current_after_view") {
      view = make_view();
      current->set_source(std::make_shared<TableCurrentIndexModel>(table));
    }
    current->set(TableIndex(0, 0));
    auto& body = view->get_body();
    auto require_current = [&] (int source_row) {
      auto displayed = body.get_current()->get();
      REQUIRE(current->get().has_value());
      REQUIRE(current->get()->m_row == source_row);
      REQUIRE(displayed.has_value());
      REQUIRE(body.get_table()->get<int>(displayed->m_row, 0) == 10);
    };
    require_current(0);
    auto updates = std::vector<TableIndex>();
    auto connection = scoped_connection(
      body.get_current()->connect_update_signal([&] (const auto& index) {
        REQUIRE(index.has_value());
        updates.push_back(*index);
      }));
    auto displayed = *body.get_current()->get();
    table->move(0, 1);
    require_current(1);
    REQUIRE(*body.get_current()->get() == displayed);
    REQUIRE(updates.empty());
    table->transact([&] {
      table->move(1, 0);
      REQUIRE(current->get().has_value());
      REQUIRE(*current->get() == TableIndex(0, 0));
      current->set(TableIndex(1, 0));
      REQUIRE(updates.empty());
    });
    REQUIRE(updates.size() == 1);
    REQUIRE(body.get_table()->get<int>(updates.back().m_row, 0) == 20);
    current->set(TableIndex(0, 0));
    REQUIRE(updates.size() == 2);
    REQUIRE(updates.back() == displayed);
    updates.clear();
    table->insert({5}, 0);
    require_current(1);
    REQUIRE(*body.get_current()->get() == TableIndex(1, 0));
    if(displayed.m_row == 0) {
      REQUIRE(updates == std::vector{TableIndex(1, 0)});
    } else {
      REQUIRE(updates.empty());
    }
    connection.disconnect();
    table->transact([&] {
      table->insert({0}, 0);
      REQUIRE(current->get().has_value());
      REQUIRE(*current->get() == TableIndex(2, 0));
      table->remove(0);
      REQUIRE(current->get().has_value());
      REQUIRE(*current->get() == TableIndex(1, 0));
      table->remove(0);
    });
    require_current(0);
    table->remove(0);
    REQUIRE(!current->get().has_value());
    REQUIRE(!body.get_current()->get().has_value());
  }

  UI_TEST_CASE("current_on_batched_removals") {
    auto table = std::make_shared<ArrayTableModel>();
    for(auto value : {10, 20, 30}) {
      table->push({value});
    }
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("Value", TableHeaderItem::Order::ASCENDING).make());
    auto& current = *view->get_current();
    auto& displayed = *view->get_body().get_current();
    current.set(TableIndex(1, 0));
    auto updates = std::vector<optional<TableIndex>>();
    auto connection = scoped_connection(displayed.connect_update_signal(
      [&] (const auto& index) { updates.push_back(index); }));
    SUBCASE("current_survives") {
      auto expected = 20;
      table->transact([&] {
        table->remove(0);
        SUBCASE("same_record") {
          table->remove(1);
        }
        SUBCASE("new_current") {
          current.set(TableIndex(1, 0));
          table->remove(0);
          expected = 30;
        }
        REQUIRE(updates.empty());
      });
      REQUIRE(table->get<int>(0, 0) == expected);
      REQUIRE(current.get().has_value());
      REQUIRE(*current.get() == TableIndex(0, 0));
      REQUIRE(displayed.get().has_value());
      REQUIRE(*displayed.get() == TableIndex(0, 0));
    }
    SUBCASE("current_removed") {
      table->transact([&] {
        table->remove(0);
        table->remove(0);
        REQUIRE(updates.empty());
      });
      REQUIRE(!current.get());
      REQUIRE(!displayed.get());
    }
    REQUIRE(updates.size() == 1);
    REQUIRE((updates.front() == displayed.get()));
  }

  UI_TEST_CASE("fixed_current_on_move") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({20});
    table->push({10});
    auto current =
      std::make_shared<LocalValueModel<optional<TableIndex>>>(
        TableIndex(0, 0));
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("Value", TableHeaderItem::Order::ASCENDING).
      set_current(current).make());
    auto& body = view->get_body();
    REQUIRE(body.get_current()->get().has_value());
    REQUIRE(*body.get_current()->get() == TableIndex(1, 0));
    auto updates = std::vector<TableIndex>();
    auto connection = scoped_connection(
      body.get_current()->connect_update_signal([&] (const auto& index) {
        REQUIRE(index.has_value());
        updates.push_back(*index);
      }));
    table->move(0, 1);
    REQUIRE(updates == std::vector{TableIndex(0, 0)});
    REQUIRE(*body.get_current()->get() == TableIndex(0, 0));
    updates.clear();
    table->transact([&] {
      table->move(1, 0);
      table->transact([&] { table->move(0, 1); });
      REQUIRE(updates.empty());
    });
    REQUIRE(updates.empty());
    table->transact([&] {
      table->move(1, 0);
      table->move(0, 1);
      table->move(1, 0);
      REQUIRE(updates.empty());
    });
    REQUIRE(updates == std::vector{TableIndex(1, 0)});
    REQUIRE(*body.get_current()->get() == TableIndex(1, 0));
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

  UI_TEST_CASE("track_multi_selection") {
    auto table = std::make_shared<ArrayTableModel>();
    for(auto i : {10, 20, 30}) {
      table->push({i, i});
    }
    auto rows = std::make_shared<ListMultiSelectionModel>();
    auto cells = std::make_shared<TableMultiSelectionModel>();
    auto selection = std::make_shared<TableSelectionModel>(
      cells, rows, std::make_shared<ArrayListModel<int>>());
    auto view = std::unique_ptr<TableView>(TableViewBuilder(table).
      add_header_item("A").add_header_item("B").set_selection(selection).
      make());
    for(auto row : {2, 0, 1}) {
      rows->push(row);
      cells->push(TableIndex(row, 1));
    }
    cells->push(TableIndex(1, 0));
    auto expected = std::vector<int>();
    SUBCASE("remove") {
      table->remove(0);
      expected = {1, 0};
    }
    SUBCASE("insert") {
      table->insert({5, 5}, 0);
      expected = {3, 1, 2};
    }
    SUBCASE("move_down") {
      table->move(0, 2);
      expected = {1, 2, 0};
    }
    SUBCASE("move_up") {
      table->move(2, 0);
      expected = {0, 1, 2};
    }
    REQUIRE(rows->get_size() == std::ssize(expected));
    REQUIRE(cells->get_size() == rows->get_size() + 1);
    for(auto i = 0; i != rows->get_size(); ++i) {
      REQUIRE(rows->get(i) == expected[i]);
      REQUIRE(cells->get(i) == TableIndex(expected[i], 1));
    }
    REQUIRE(cells->get(rows->get_size()) == TableIndex(expected.back(), 0));
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
    SUBCASE("edits_after_rebinding") {
      auto previous_columns =
        first_view->get_body().get_selection()->get_column_selection();
      first->push({15});
      auto second = std::make_shared<ArrayTableModel>();
      second->push({20});
      second->push({25});
      auto view = std::unique_ptr<TableView>(TableViewBuilder(second).
        add_header_item("Value").set_selection(selection).make());
      auto& body = first_view->get_body();
      auto& previous = *body.get_selection();
      auto& columns = *selection->get_column_selection();
      SUBCASE("mouse") {
        first_view->resize(320, 200);
        first_view->show();
        QApplication::processEvents();
        auto cell = body.find_item(TableIndex(1, 0));
        REQUIRE(cell);
        move_cursor(*cell, cell->rect().center());
        press_mouse(*cell, Qt::LeftButton);
        release_mouse(*cell, Qt::LeftButton);
      }
      SUBCASE("select_all") {
        press_key(body, Qt::Key_A, Qt::ControlModifier);
      }
      SUBCASE("displayed_lists") {
        auto check = [] (auto& list, const auto& value) {
          REQUIRE(list.set(0, value) == QValidator::Invalid);
          REQUIRE(list.push(value) == QValidator::Invalid);
          REQUIRE(list.insert(value, 0) == QValidator::Invalid);
          REQUIRE(list.move(0, 0) == QValidator::Invalid);
          REQUIRE(list.remove(0) == QValidator::Invalid);
        };
        check(*previous.get_row_selection(), 1);
        check(*previous.get_item_selection(), TableIndex(1, 0));
        REQUIRE(previous_columns->push(0) == QValidator::Invalid);
        REQUIRE(columns.get_size() == 0);
        auto& active =
          *view->get_body().get_selection()->get_column_selection();
        REQUIRE(active.push(0) == QValidator::Acceptable);
        check(*previous_columns, 0);
        REQUIRE(columns.get_size() == 1);
        REQUIRE(columns.get(0) == 0);
        REQUIRE(active.remove(0) == QValidator::Acceptable);
      }
      REQUIRE(rows->get_size() == 1);
      REQUIRE(rows->get(0) == 0);
      REQUIRE(cells->get_size() == 1);
      REQUIRE(cells->get(0) == TableIndex(0, 0));
      REQUIRE(columns.get_size() == 0);
      REQUIRE(view->get_body().get_selection()->get_row_selection()->push(1) ==
        QValidator::Acceptable);
      REQUIRE(rows->get(1) == 1);
    }
    SUBCASE("same_table") {
      auto view = std::unique_ptr<TableView>(TableViewBuilder(first).
        add_header_item("Value").set_selection(selection).make());
      first->insert({5}, 0);
      REQUIRE(rows->get(0) == 1);
      REQUIRE(cells->get(0) == TableIndex(1, 0));
      auto& displayed = *first_view->get_body().get_selection();
      REQUIRE(displayed.get_row_selection()->push(0) == QValidator::Acceptable);
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
    auto& displayed_columns = *displayed.get_column_selection();
    columns->push(0);
    REQUIRE(displayed_columns.get_size() == 1);
    REQUIRE(displayed_columns.get(0) == 0);
    REQUIRE(displayed_columns.remove(0) == QValidator::Acceptable);
    REQUIRE(columns->get_size() == 0);
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
