#include "Spire/Ui/TableView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FilteredTableModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/StandardTableFilter.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableItem.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  SortedTableModel::Ordering to_table_order(TableHeaderItem::Order order) {
    if(order == TableHeaderItem::Order::ASCENDING) {
      return SortedTableModel::Ordering::ASCENDING;
    } else if(order == TableHeaderItem::Order::DESCENDING) {
      return SortedTableModel::Ordering::DESCENDING;
    }
    return SortedTableModel::Ordering::NONE;
  }

  auto make_column_order(const TableView::HeaderModel& header) {
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    for(auto i = 0; i != header.get_size(); ++i) {
      auto& item = header.get(i);
      if(item.m_order != TableHeaderItem::Order::NONE &&
          item.m_order != TableHeaderItem::Order::UNORDERED) {
        order.push_back(
          SortedTableModel::ColumnOrder(i, to_table_order(item.m_order)));
      }
    }
    return order;
  }
}

QWidget* TableView::default_view_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  return TableBody::default_view_builder(table, row, column);
}

TableView::TableView(
    std::shared_ptr<TableModel> table, std::shared_ptr<HeaderModel> header,
    std::shared_ptr<TableFilter> filter, std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection, ViewBuilder view_builder,
    Comparator comparator, QWidget* parent)
    : QWidget(parent),
      m_table(std::move(table)),
      m_header(std::move(header)),
      m_filter(std::move(filter)),
      m_horizontal_spacing(0),
      m_vertical_spacing(0) {
  for(auto i = 0; i != m_header->get_size(); ++i) {
    auto& item = m_header->get(i);
    auto filter = m_filter->get_filter(i);
    if(filter != item.m_filter) {
      auto revised_item = item;
      revised_item.m_filter = filter;
      m_header->set(i, revised_item);
    }
  }
  m_header_view = new TableHeader(m_header);
  m_header_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_header_view->setContentsMargins({scale_width(1), 0, 0, 0});
  link(*this, *m_header_view);
  auto box_body = new QWidget();
  enclose(*box_body, *m_header_view);
  auto box = new Box(box_body);
  box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  link(*box, *m_header_view);
  update_style(*box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  proxy_style(*this, *box);
  m_filtered_table = std::make_shared<FilteredTableModel>(
    m_table, std::bind_front(&TableView::is_filtered, this));
  if(comparator) {
    m_sorted_table = std::make_shared<SortedTableModel>(
      m_filtered_table, make_column_order(*m_header), std::move(comparator));
  } else {
    m_sorted_table = std::make_shared<SortedTableModel>(
      m_filtered_table, make_column_order(*m_header));
  }
  m_body = new TableBody(m_sorted_table, std::move(current),
    std::move(selection), m_header_view->get_widths(), std::move(view_builder));
  m_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  link(*this, *m_body);
  m_scroll_box = new ScrollBox(m_body);
  m_scroll_box->set(ScrollBox::DisplayPolicy::ON_ENGAGE);
  auto layout = make_vbox_layout(this);
  layout->addWidget(box);
  layout->addWidget(m_scroll_box);
  layout->addStretch(1);
  m_header_view->connect_sort_signal(
    std::bind_front(&TableView::on_order_update, this));
  m_header_view->connect_filter_signal(
    std::bind_front(&TableView::on_filter_clicked, this));
  m_filter_connection = m_filter->connect_filter_signal(
    std::bind_front(&TableView::on_filter, this));
  m_current_connection = m_body->get_current()->connect_update_signal(
    std::bind_front(&TableView::on_current, this));
  m_body_style_connection = connect_style_signal(
    *m_body, std::bind_front(&TableView::on_body_style, this));
  setFocusProxy(m_body);
  on_body_style();
}

const std::shared_ptr<TableModel>& TableView::get_table() const {
  return m_table;
}

const std::shared_ptr<TableView::CurrentModel>& TableView::get_current() const {
  return m_body->get_current();
}

const std::shared_ptr<TableView::SelectionModel>&
    TableView::get_selection() const {
  return m_body->get_selection();
}

TableHeader& TableView::get_header() {
  return *m_header_view;
}

TableBody& TableView::get_body() {
  return *m_body;
}

connection TableView::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

bool TableView::is_filtered(const TableModel& model, int row) {
  return m_filter->is_filtered(model, row);
}

void TableView::on_order_update(int index, TableHeaderItem::Order order) {
  auto header_item = m_header->get(index);
  header_item.m_order = order;
  auto column_order = m_sorted_table->get_column_order();
  adjust(
    SortedTableModel::ColumnOrder(index, to_table_order(order)), column_order);
  m_sorted_table->set_column_order(column_order);
  for(auto i = 0; i != m_header->get_size(); ++i) {
    if(i == index) {
      m_header->set(i, header_item);
    } else {
      auto item = m_header->get(i);
      if(item.m_order != TableHeaderItem::Order::UNORDERED) {
        item.m_order = TableHeaderItem::Order::NONE;
        m_header->set(i, item);
      }
    }
  }
  m_sort_signal(index, order);
}

void TableView::on_filter_clicked(int index) {
  auto& filter_button = m_header_view->get_filter_button(index);
  auto widget = m_filter->make_filter_widget(index, filter_button);
  widget->show();
}

void TableView::on_filter(int column, TableFilter::Filter filter) {
  auto& item = m_header->get(column);
  if(item.m_filter != filter) {
    auto revised_item = item;
    revised_item.m_filter = filter;
    m_header->set(column, revised_item);
  }
  m_filtered_table->set_filter(std::bind_front(&TableView::is_filtered, this));
}

void TableView::on_current(const optional<Index>& current) {
  if(!current) {
    return;
  }
  if(auto item = m_body->get_item(*current)) {
    auto& horizontal_scroll_bar = m_scroll_box->get_horizontal_scroll_bar();
    auto old_x = horizontal_scroll_bar.get_position();
    auto& vertical_scroll_bar = m_scroll_box->get_vertical_scroll_bar();
    auto old_y = vertical_scroll_bar.get_position();
    m_scroll_box->scroll_to(*item);
    auto x = horizontal_scroll_bar.get_position();
    if(x > old_x) {
      horizontal_scroll_bar.set_position(x + m_horizontal_spacing);
    } else if(x < old_x) {
      horizontal_scroll_bar.set_position(x - m_horizontal_spacing);
    }
    auto y = vertical_scroll_bar.get_position();
    if(y > old_y) {
      vertical_scroll_bar.set_position(y + m_vertical_spacing);
    } else if(y < old_y) {
      vertical_scroll_bar.set_position(y - m_vertical_spacing);
    }
  }
}

void TableView::on_body_style() {
  auto& stylist = find_stylist(*m_body);
  m_horizontal_spacing = 0;
  m_vertical_spacing = 0;
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const HorizontalSpacing& spacing) {
        stylist.evaluate(spacing, [=] (auto spacing) {
          m_horizontal_spacing = spacing;
          update();
        });
      },
      [&] (const VerticalSpacing& spacing) {
        stylist.evaluate(spacing, [=] (auto spacing) {
          m_vertical_spacing = spacing;
          update();
        });
      });
  }
  update();
}

TableViewBuilder::TableViewBuilder(
  std::shared_ptr<TableModel> table, QWidget* parent)
  : m_table(std::move(table)),
    m_parent(parent),
    m_header(std::make_shared<ArrayListModel<TableHeaderItem::Model>>()),
    m_filter(std::make_shared<EmptyTableFilter>()),
    m_current(std::make_shared<LocalValueModel<optional<TableIndex>>>()),
    m_selection(std::make_shared<TableSelectionModel>(
      std::make_shared<TableEmptySelectionModel>(),
      std::make_shared<ListSingleSelectionModel>(),
      std::make_shared<ListEmptySelectionModel>())),
    m_view_builder(&TableView::default_view_builder) {}

TableViewBuilder& TableViewBuilder::set_header(
    const std::shared_ptr<TableView::HeaderModel>& header) {
  m_header = header;
  return *this;
}

TableViewBuilder& TableViewBuilder::add_header_item(QString name) {
  return add_header_item(std::move(name), QString());
}

TableViewBuilder& TableViewBuilder::add_header_item(
    QString name, QString short_name) {
  return add_header_item(
    std::move(name), std::move(short_name), TableFilter::Filter::NONE);
}

TableViewBuilder& TableViewBuilder::add_header_item(
    QString name, QString short_name, TableFilter::Filter filter) {
  m_header->push(TableHeaderItem::Model(std::move(name), std::move(short_name),
    TableHeaderItem::Order::NONE, filter));
  return *this;
}

TableViewBuilder& TableViewBuilder::add_header_item(
    QString name, TableFilter::Filter filter) {
  return add_header_item(std::move(name), QString(), filter);
}

TableViewBuilder& TableViewBuilder::set_filter(
    const std::shared_ptr<TableFilter>& filter) {
  m_filter = filter;
  return *this;
}

TableViewBuilder& TableViewBuilder::set_standard_filter() {
  if(m_table->get_row_size() == 0) {
    return *this;
  }
  auto types = std::vector<std::type_index>();
  for(auto i = 0; i != m_table->get_column_size(); ++i) {
    types.push_back(m_table->at(0, i).get_type());
  }
  return set_filter(std::make_shared<StandardTableFilter>(std::move(types)));
}

TableViewBuilder& TableViewBuilder::set_current(
    const std::shared_ptr<TableView::CurrentModel>& current) {
  m_current = current;
  return *this;
}

TableViewBuilder& TableViewBuilder::set_selection(
    const std::shared_ptr<TableView::SelectionModel>& selection) {
  m_selection = selection;
  return *this;
}

TableViewBuilder& TableViewBuilder::set_view_builder(
    const TableView::ViewBuilder& view_builder) {
  m_view_builder = view_builder;
  return *this;
}

TableViewBuilder& TableViewBuilder::set_comparator(
    TableView::Comparator comparator) {
  m_comparator = comparator;
  return *this;
}

TableView* TableViewBuilder::make() const {
  return new TableView(m_table, m_header, m_filter, m_current, m_selection,
    m_view_builder, m_comparator, m_parent);
}
