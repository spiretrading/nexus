#include "Spire/Blotter/BlotterExecutionsView.hpp"
#include "Spire/Ui/ArrayTableModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto to_table(std::shared_ptr<OrderListModel> orders) {
    return std::make_shared<ArrayTableModel>();
  }
}

BlotterExecutionsView::BlotterExecutionsView(
    std::shared_ptr<OrderListModel> orders, QWidget* parent)
    : QWidget(parent),
      m_orders(std::move(orders)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table_view_builder = TableViewBuilder(to_table(m_orders));
  table_view_builder.add_header_item(tr("Time"));
  table_view_builder.add_header_item(tr("ID"));
  table_view_builder.add_header_item(tr("Side"));
  table_view_builder.add_header_item(tr("Security"));
  table_view_builder.add_header_item(tr("Status"));
  table_view_builder.add_header_item(tr("Quantity"), tr("Qty"));
  table_view_builder.add_header_item(tr("Last Quantity"), tr("Last Qty"));
  table_view_builder.add_header_item(tr("Price"), tr("Px"));
  table_view_builder.add_header_item(tr("Last Price"), tr("Last Px"));
  table_view_builder.add_header_item(tr("Profit and Loss"), tr("P/L"));
  table_view_builder.add_header_item(tr("Market"), tr("Mkt"));
  table_view_builder.add_header_item(tr("Flag"));
  table_view_builder.add_header_item(tr("Execution Fee"), tr("Exec Fee"));
  table_view_builder.add_header_item(tr("Processing Fee"), tr("Proc Fee"));
  table_view_builder.add_header_item(tr("Miscellaneous Fee"), tr("Misc Fee"));
  table_view_builder.add_header_item(tr("Message"));
  auto table = table_view_builder.make();
  table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *table);
}
