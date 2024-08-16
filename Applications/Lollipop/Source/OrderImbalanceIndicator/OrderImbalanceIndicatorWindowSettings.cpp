#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindowSettings.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindow.hpp"
#include "Spire/UI/CollapsibleWidget.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_OrderImbalanceIndicatorWindow.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

OrderImbalanceIndicatorWindowSettings::
    OrderImbalanceIndicatorWindowSettings() {}

OrderImbalanceIndicatorWindowSettings::OrderImbalanceIndicatorWindowSettings(
    const OrderImbalanceIndicatorWindow& window,
    Ref<UserProfile> userProfile)
    : m_properties(window.GetModel()->GetProperties()),
      m_geometry(window.saveGeometry()),
      m_tableHeaderGeometry(window.m_ui->m_orderImbalanceIndicatorTableView->
        horizontalHeader()->saveGeometry()),
      m_tableHeaderState(window.m_ui->m_orderImbalanceIndicatorTableView->
        horizontalHeader()->saveState()),
      m_timeRangeSettings(
        window.m_ui->m_timeRangeParametersWidget->GetWindowSettings()),
      m_marketsSettings(
        window.m_ui->m_marketSelectionWidget->GetWindowSettings()) {}

OrderImbalanceIndicatorWindowSettings::
    ~OrderImbalanceIndicatorWindowSettings() {}

string OrderImbalanceIndicatorWindowSettings::GetName() const {
  return "Order Imbalance Indicator";
}

QWidget* OrderImbalanceIndicatorWindowSettings::Reopen(
    Ref<UserProfile> userProfile) const {
  std::shared_ptr<OrderImbalanceIndicatorModel> model =
    std::make_shared<OrderImbalanceIndicatorModel>(Ref(userProfile),
    m_properties);
  OrderImbalanceIndicatorWindow* window = new OrderImbalanceIndicatorWindow(
    Ref(userProfile), model);
  window->setAttribute(Qt::WA_DeleteOnClose);
  Apply(Ref(userProfile), Store(*window));
  return window;
}

void OrderImbalanceIndicatorWindowSettings::Apply(
    Ref<UserProfile> userProfile, Out<QWidget> widget) const {
  OrderImbalanceIndicatorWindow& window =
    dynamic_cast<OrderImbalanceIndicatorWindow&>(*widget);
  window.restoreGeometry(m_geometry);
  window.m_ui->m_orderImbalanceIndicatorTableView->horizontalHeader()->
    restoreGeometry(m_tableHeaderGeometry);
  window.m_ui->m_orderImbalanceIndicatorTableView->horizontalHeader()->
    restoreState(m_tableHeaderState);
  m_timeRangeSettings->Apply(Ref(userProfile),
    Store(*window.m_ui->m_timeRangeParametersWidget));
  m_marketsSettings->Apply(Ref(userProfile),
    Store(*window.m_ui->m_marketSelectionWidget));
}
