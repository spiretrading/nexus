#include "Spire/Blotter/BlotterTabView.hpp"
#include "Spire/Blotter/BlotterExecutionsView.hpp"
#include "Spire/Blotter/BlotterOrderLogView.hpp"
#include "Spire/Blotter/BlotterPositionsView.hpp"
#include "Spire/Blotter/BlotterProfitLossView.hpp"
#include "Spire/Blotter/BlotterTaskView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TabView.hpp"

using namespace Spire;

BlotterTabView::BlotterTabView(QWidget* parent)
    : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto tab_view = new TabView();
  tab_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  tab_view->add(
    std::vector{tr("Positions"), tr("Pos")}, *new BlotterPositionsView());
  tab_view->add(
    std::vector{tr("Profit/Loss"), tr("P/L")}, *new BlotterProfitLossView());
  auto orders = std::make_shared<ArrayListModel<OrdersModel::Type>>();
  tab_view->add(std::vector{tr("Order Log"), tr("Orders")},
    *new BlotterOrderLogView(orders));
  tab_view->add(
    std::vector{tr("Executions"), tr("Exec")}, *new BlotterExecutionsView());
  tab_view->add(tr("Task"), *new BlotterTaskView());
  enclose(*this, *tab_view);
  setFocusProxy(tab_view);
}
