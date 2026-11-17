#include "Spire/Blotter/BlotterTabView.hpp"
#include "Spire/Blotter/BlotterExecutionsView.hpp"
#include "Spire/Blotter/BlotterPositionsView.hpp"
#include "Spire/Blotter/BlotterProfitLossView.hpp"
#include "Spire/Blotter/BlotterTaskView.hpp"
#include "Spire/Blotter/OrderLogView.hpp"
#include "Spire/Blotter/TaskOrdersModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TabView.hpp"

using namespace Spire;

BlotterTabView::BlotterTabView(
    std::shared_ptr<TasksModel> tasks, QWidget* parent)
    : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto tab_view = new TabView();
  tab_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  tab_view->add(
    std::vector{tr("Positions"), tr("Pos")}, *new BlotterPositionsView());
  tab_view->add(
    std::vector{tr("Profit/Loss"), tr("P/L")}, *new BlotterProfitLossView());
  tab_view->add(std::vector{tr("Order Log"), tr("Orders")},
    *new OrderLogView(std::make_shared<TaskOrdersModel>(std::move(tasks))));
  tab_view->add(
    std::vector{tr("Executions"), tr("Exec")}, *new BlotterExecutionsView());
  tab_view->add(tr("Task"), *new BlotterTaskView());
  enclose(*this, *tab_view);
  setFocusProxy(tab_view);
}
