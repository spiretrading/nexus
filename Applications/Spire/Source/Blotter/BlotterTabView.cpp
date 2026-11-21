#include "Spire/Blotter/BlotterTabView.hpp"
#include "Spire/Blotter/BlotterExecutionsView.hpp"
#include "Spire/Blotter/BlotterProfitLossView.hpp"
#include "Spire/Blotter/BlotterTaskView.hpp"
#include "Spire/Blotter/OrderExecutionReportsModel.hpp"
#include "Spire/Blotter/TaskOrdersModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TabView.hpp"

using namespace boost::signals2;
using namespace Spire;

BlotterTabView::BlotterTabView(std::shared_ptr<BlotterModel> model,
    std::shared_ptr<TableView::CurrentModel> current,
    std::shared_ptr<CancelKeyBindingsModel> cancel_bindings,
    std::shared_ptr<PositionKeyBindingsModel> position_bindings,
    QWidget* parent)
    : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto tab_view = new TabView();
  tab_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_positions = new BlotterPositionsView(
    model->get_portfolio(), std::move(position_bindings));
  tab_view->add(std::vector{tr("Positions"), tr("Pos")}, *m_positions);
  tab_view->add(
    std::vector{tr("Profit/Loss"), tr("P/L")}, *new BlotterProfitLossView());
  auto orders = std::make_shared<TaskOrdersModel>(model->get_tasks());
  m_order_log = new OrderLogView(orders, std::move(cancel_bindings));
  tab_view->add(std::vector{tr("Order Log"), tr("Orders")}, *m_order_log);
  tab_view->add(
    std::vector{tr("Executions"), tr("Exec")}, *new BlotterExecutionsView(
      std::make_shared<OrderExecutionReportsModel>(orders)));
  tab_view->add(
    tr("Task"), *new BlotterTaskView(model->get_tasks(), std::move(current)));
  enclose(*this, *tab_view);
  setFocusProxy(tab_view);
}

connection BlotterTabView::connect_cancel_orders_signal(
    const OrderLogView::CancelSignal::slot_type& slot) const {
  return m_order_log->connect_cancel_signal(slot);
}

connection BlotterTabView::connect_cancel_order_operation_signal(
    const OrderLogView::CancelOperationSignal::slot_type& slot) const {
  return m_order_log->connect_cancel_operation_signal(slot);
}

connection BlotterTabView::connect_flatten_positions_signal(
    const BlotterPositionsView::FlattenSignal::slot_type& slot) const {
  return m_positions->connect_flatten_signal(slot);
}
