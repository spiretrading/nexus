#include "Spire/Ui/TableView.hpp"
#include <QVBoxLayout>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/SortedTableModel.hpp"
#include "Spire/Ui/StandardTableFilter.hpp"
#include "Spire/Ui/TableBody.hpp"

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
}

QWidget* TableView::default_view_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  return TableBody::default_view_builder(table, row, column);
}

TableView::TableView(
    std::shared_ptr<TableModel> table, std::shared_ptr<HeaderModel> header,
    std::shared_ptr<TableFilter> filter, std::shared_ptr<CurrentModel> current,
    ViewBuilder view_builder, QWidget* parent)
    : QWidget(parent),
      m_table(std::move(table)),
      m_header(std::move(header)),
      m_filter(std::move(filter)) {
  auto box_body = new QWidget();
  auto box_body_layout = new QVBoxLayout(box_body);
  box_body_layout->setContentsMargins({});
  for(auto i = 0; i != m_header->get_size(); ++i) {
    auto& item = m_header->get(i);
    auto filter = m_filter->get_filter(i);
    if(filter != item.m_filter) {
      auto revised_item = item;
      revised_item.m_filter = filter;
      m_header->set(i, revised_item);
    }
  }
  auto header_view = new TableHeader(m_header);
  box_body_layout->addWidget(header_view);
  auto box = new Box(box_body);
  update_style(*box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  proxy_style(*this, *box);
  auto sorted_table = std::make_shared<SortedTableModel>(m_table);
  m_body = new TableBody(std::move(sorted_table), std::move(current),
    header_view->get_widths(), std::move(view_builder));
  auto layout = new QVBoxLayout(this);
  layout->addWidget(box);
  layout->addWidget(m_body);
  header_view->connect_sort_signal(
    std::bind_front(&TableView::on_order_update, this));
  header_view->connect_filter_signal(
    std::bind_front(&TableView::on_filter, this));
}

const std::shared_ptr<TableModel>& TableView::get_table() const {
  return m_table;
}

const std::shared_ptr<TableView::CurrentModel>& TableView::get_current() const {
  return m_body->get_current();
}

connection TableView::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

void TableView::on_order_update(int index, TableHeaderItem::Order order) {
  auto header_item = m_header->get(index);
  header_item.m_order = order;
  auto sorted_table =
    std::static_pointer_cast<SortedTableModel>(m_body->get_table());
  auto column_order = sorted_table->get_column_order();
  adjust(
    SortedTableModel::ColumnOrder(index, to_table_order(order)), column_order);
  sorted_table->set_column_order(column_order);
  for(auto i = 0; i != m_header->get_size(); ++i) {
    if(i == index) {
      m_header->set(i, header_item);
    } else {
      auto item = m_header->get(i);
      item.m_order = TableHeaderItem::Order::NONE;
      m_header->set(i, item);
    }
  }
  m_sort_signal(index, order);
}

void TableView::on_filter(int index) {
  auto widget = m_filter->make_filter_widget(index, *this);
  widget->show();
}

TableViewBuilder::TableViewBuilder(
  std::shared_ptr<TableModel> table, QWidget* parent)
  : m_table(std::move(table)),
    m_parent(parent),
    m_filter(std::make_shared<EmptyTableFilter>()),
    m_current(std::make_shared<LocalValueModel<optional<TableBody::Index>>>()),
    m_view_builder(&TableView::default_view_builder) {}

TableViewBuilder& TableViewBuilder::set_header(
    const std::shared_ptr<TableView::HeaderModel>& header) {
  m_header = header;
  return *this;
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
    types.push_back(m_table->at(0, i).type());
  }
  return set_filter(std::make_shared<StandardTableFilter>(std::move(types)));
}

TableViewBuilder& TableViewBuilder::set_current(
    const std::shared_ptr<TableView::CurrentModel>& current) {
  m_current = current;
  return *this;
}

TableViewBuilder& TableViewBuilder::set_view_builder(
    const TableView::ViewBuilder& view_builder) {
  m_view_builder = view_builder;
  return *this;
}

TableView* TableViewBuilder::make() const {
  return new TableView(
    m_table, m_header, m_filter, m_current, m_view_builder, m_parent);
}
