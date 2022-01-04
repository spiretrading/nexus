#include "Spire/Ui/TableView.hpp"
#include <QVBoxLayout>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/SortedTableModel.hpp"
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

TableView::TableView(std::shared_ptr<TableModel> table,
  std::shared_ptr<HeaderModel> header, QWidget* parent)
  : TableView(std::move(table), std::move(header),
      std::make_shared<LocalValueModel<optional<Index>>>(),
      &default_view_builder, parent) {}

TableView::TableView(std::shared_ptr<TableModel> table,
  std::shared_ptr<HeaderModel> header, std::shared_ptr<CurrentModel> current,
  QWidget* parent)
  : TableView(std::move(table), std::move(header), std::move(current),
      &default_view_builder, parent) {}

TableView::TableView(std::shared_ptr<TableModel> table,
  std::shared_ptr<HeaderModel> header, ViewBuilder view_builder,
  QWidget* parent)
  : TableView(std::move(table), std::move(header),
      std::make_shared<LocalValueModel<optional<Index>>>(),
      std::move(view_builder), parent) {}

TableView::TableView(std::shared_ptr<TableModel> table,
    std::shared_ptr<HeaderModel> header, std::shared_ptr<CurrentModel> current,
    ViewBuilder view_builder, QWidget* parent)
    : QWidget(parent),
      m_table(std::move(table)),
      m_header(std::move(header)) {
  auto box_body = new QWidget();
  auto box_body_layout = new QVBoxLayout(box_body);
  box_body_layout->setContentsMargins({});
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
