#include "Spire/Blotter/BlotterOrderLogView.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto to_table(std::shared_ptr<OrderListModel> orders) {
    return std::make_shared<ArrayTableModel>();
  }
}

BlotterOrderLogView::BlotterOrderLogView(
    std::shared_ptr<OrderListModel> orders, QWidget* parent)
    : QWidget(parent),
      m_orders(std::move(orders)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table_view_builder = TableViewBuilder(to_table(m_orders));
  table_view_builder.add_header_item(tr("Time"));
  table_view_builder.add_header_item(tr("ID"));
  table_view_builder.add_header_item(tr("Status"));
  table_view_builder.add_header_item(tr("Security"));
  table_view_builder.add_header_item(tr("Currency"), tr("Curr"));
  table_view_builder.add_header_item(tr("Type"));
  table_view_builder.add_header_item(tr("Side"));
  table_view_builder.add_header_item(tr("Destination"), tr("Dest"));
  table_view_builder.add_header_item(tr("Quantity"), tr("Qty"));
  table_view_builder.add_header_item(tr("Price"), tr("Px"));
  table_view_builder.add_header_item(tr("Time in Force"), tr("TIF"));
  auto table = table_view_builder.make();
  table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto scroll_box = new ScrollBox(table);
  scroll_box->set(
    ScrollBox::DisplayPolicy::ON_OVERFLOW, ScrollBox::DisplayPolicy::NEVER);
  enclose(*this, *scroll_box);
}

const std::shared_ptr<OrderListModel>& BlotterOrderLogView::get_orders() const {
  return m_orders;
}
