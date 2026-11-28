#include <doctest/doctest.h>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/EditableTableView.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/Filter.hpp"
#include "Spire/Ui/StandardTableFilter.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Tests;

namespace {
  struct Observer {
    std::vector<std::pair<int, TableFilter::Filter>> m_updates;

    void on_filter(int column, TableFilter::Filter filter) {
      m_updates.emplace_back(column, filter);
    }
  };

  class MinimumFilter : public Filter<int> {
    public:
      MinimumFilter() {
        get_current()->set(Range(10, none));
      }
  };
}

TEST_SUITE("StandardTableFilter") {
  TEST_CASE("column_filters") {
    auto source = ArrayTableModel();
    source.push({5, QString("Low"), 30});
    source.push({15, QString("Middle"), 20});
    source.push({25, QString("High"), 10});
    auto first = std::make_shared<LocalValueModel<Filter<int>::Range>>();
    auto last = std::make_shared<LocalValueModel<Filter<int>::Range>>();
    auto filters = std::vector<std::unique_ptr<ItemFilter>>(3);
    filters[0] = filter<int>(first);
    filters[2] = filter<int>(last);
    auto filter = StandardTableFilter(std::move(filters));
    auto observer = Observer();
    filter.connect_filter_signal(
      std::bind_front(&Observer::on_filter, &observer));
    REQUIRE(filter.get_filter(0) == TableFilter::Filter::UNFILTERED);
    REQUIRE(filter.get_filter(1) == TableFilter::Filter::NONE);
    REQUIRE(filter.get_filter(3) == TableFilter::Filter::NONE);
    REQUIRE(!filter.is_filtered(source, 0));
    first->set(Filter<int>::Range(10, none));
    REQUIRE(filter.is_filtered(source, 0));
    REQUIRE(!filter.is_filtered(source, 1));
    REQUIRE(!filter.is_filtered(source, 2));
    REQUIRE(observer.m_updates.size() == 1);
    REQUIRE(
      observer.m_updates.back() == std::pair(0, TableFilter::Filter::FILTERED));
    last->set(Filter<int>::Range(15, none));
    REQUIRE(filter.is_filtered(source, 0));
    REQUIRE(!filter.is_filtered(source, 1));
    REQUIRE(filter.is_filtered(source, 2));
    REQUIRE(
      observer.m_updates.back() == std::pair(2, TableFilter::Filter::FILTERED));
    last->set(Filter<int>::Range(25, none));
    REQUIRE(filter.is_filtered(source, 1));
    REQUIRE(observer.m_updates.size() == 3);
    last->set(Filter<int>::Range());
    REQUIRE(observer.m_updates.back() ==
      std::pair(2, TableFilter::Filter::UNFILTERED));
    REQUIRE(!filter.is_filtered(source, 1));
    REQUIRE(!filter.is_filtered(source, 2));
    first->set(Filter<int>::Range());
    REQUIRE(!filter.is_filtered(source, 0));
  }

  UI_TEST_CASE("widgets") {
    auto current = std::make_shared<LocalValueModel<Filter<int>::Range>>();
    auto filters = std::vector<std::unique_ptr<ItemFilter>>(2);
    filters[1] = filter<int>(current);
    auto filter = StandardTableFilter(std::move(filters));
    auto parent = QWidget();
    REQUIRE(!filter.make_filter_widget(0, parent));
    REQUIRE(!filter.make_filter_widget(2, parent));
    auto widget = filter.make_filter_widget(1, parent);
    REQUIRE(widget);
    REQUIRE(widget->parentWidget() == &parent);
    REQUIRE(filter.make_filter_widget(1, parent) == widget);
    current->set(Filter<int>::Range(10, 20));
    delete widget;
    auto panel =
      dynamic_cast<IntegerFilterPanel*>(filter.make_filter_widget(1, parent));
    REQUIRE(panel);
    REQUIRE(panel->get_minimum()->get().value_or(-1) == 10);
    REQUIRE(panel->get_maximum()->get().value_or(-1) == 20);
  }

  UI_TEST_CASE("builder_filters") {
    auto source = std::make_shared<ArrayTableModel>();
    auto filter = std::make_unique<MinimumFilter>();
    auto current = filter->get_current();
    auto view = std::unique_ptr<TableView>();
    auto offset = 0;
    SUBCASE("table") {
      view.reset(TableViewBuilder(source).add_header_item("Name").
        add_header_item("Quantity", "Qty", TableHeaderItem::Order::ASCENDING,
          std::move(filter)).make());
    }
    SUBCASE("editable_table") {
      view.reset(EditableTableViewBuilder(source).add_header_item("Name").
        add_header_item("Quantity", "Qty", TableHeaderItem::Order::ASCENDING,
          std::move(filter)).make());
      offset = 1;
    }
    auto& header = view->get_header();
    auto table = view->get_body().get_table();
    REQUIRE(table->get_row_size() == 0);
    REQUIRE(
      header.get_items()->get(offset).m_filter == TableFilter::Filter::NONE);
    REQUIRE(header.get_items()->get(offset + 1).m_filter ==
      TableFilter::Filter::FILTERED);
    source->push({QString("Low"), 5});
    source->push({QString("High"), 20});
    REQUIRE(table->get_row_size() == 1);
    REQUIRE(table->get<int>(0, offset + 1) == 20);
    source->set(0, 1, 15);
    REQUIRE(table->get_row_size() == 2);
    REQUIRE(table->get<int>(0, offset + 1) == 15);
    current->set(Filter<int>::Range(none, 15));
    REQUIRE(table->get_row_size() == 1);
    REQUIRE(table->get<int>(0, offset + 1) == 15);
    current->set(Filter<int>::Range());
    REQUIRE(table->get_row_size() == 2);
    REQUIRE(header.get_items()->get(offset + 1).m_filter ==
      TableFilter::Filter::UNFILTERED);
  }

  UI_TEST_CASE("table_filter_override") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({5});
    auto custom = std::make_shared<EmptyTableFilter>();
    auto view = std::unique_ptr<TableView>();
    SUBCASE("table") {
      view.reset(TableViewBuilder(source).set_filter(custom).
        add_header_item("Quantity", std::make_unique<MinimumFilter>()).make());
    }
    SUBCASE("editable_table") {
      view.reset(EditableTableViewBuilder(source).
        add_header_item("Quantity", std::make_unique<MinimumFilter>()).
        set_filter(custom).make());
    }
    REQUIRE(view->get_body().get_table()->get_row_size() == 1);
    REQUIRE(view->get_header().get_filter() == custom);
  }
}
