#include "Spire/Blotter/BlotterOrderLogView.hpp"
#include <QKeyEvent>
#include "Spire/Blotter/OrdersToTableModel.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListSelectionValueModel.hpp"
#include "Spire/Ui/MultiSelectionModel.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::Styles;

BlotterOrderLogView::BlotterOrderLogView(
    std::shared_ptr<OrderListModel> orders, QWidget* parent)
    : QWidget(parent),
      m_orders(std::move(orders)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table_view_builder =
    TableViewBuilder(std::make_shared<OrdersToTableModel>(m_orders));
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
  auto selection = std::make_shared<TableSelectionModel>(
    std::make_shared<TableEmptySelectionModel>(),
    std::make_shared<ListMultiSelectionModel>(),
    std::make_shared<ListEmptySelectionModel>());
  table_view_builder.set_selection(std::move(selection));
  auto table = table_view_builder.make();
  m_selection = std::make_shared<ListSelectionValueModel<const Order*>>(
    m_orders, table->get_selection()->get_row_selection());
  table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto scroll_box = new ScrollBox(table);
  scroll_box->set(
    ScrollBox::DisplayPolicy::ON_OVERFLOW, ScrollBox::DisplayPolicy::NEVER);
  enclose(*this, *scroll_box);
}

const std::shared_ptr<OrderListModel>& BlotterOrderLogView::get_orders() const {
  return m_orders;
}

const std::shared_ptr<OrderListModel>&
    BlotterOrderLogView::get_selection() const {
  return m_selection;
}

connection BlotterOrderLogView::connect_cancel_signal(
    const CancelSignal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

void BlotterOrderLogView::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    auto orders = std::vector<const Order*>();
    for(auto i = 0; i != m_selection->get_size(); ++i) {
      auto order = m_selection->get(i);
      auto snapshot = order->GetPublisher().GetSnapshot();
      if(snapshot && !IsTerminal(snapshot->back().m_status)) {
        orders.push_back(m_selection->get(i));
      }
    }
    m_cancel_signal(orders);
    return;
  }
  QWidget::keyPressEvent(event);
}
