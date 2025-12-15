#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindow.hpp"
#include "Spire/InputWidgets/TimeRangeInputWidget.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorVenueSelectionWidget.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindowSettings.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "ui_OrderImbalanceIndicatorWindow.h"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  auto DEFAULT_COLUMN_SIZES =
    std::array<int, OrderImbalanceIndicatorModel::COLUMN_COUNT>{
      65, 70, 60, 100, 90, 120, 120};
}

OrderImbalanceIndicatorWindow::OrderImbalanceIndicatorWindow(
    Ref<UserProfile> userProfile,
    const std::shared_ptr<OrderImbalanceIndicatorModel>& model, QWidget* parent,
    Qt::WindowFlags flags)
    : QFrame(parent, flags),
      m_ui(std::make_unique<Ui_OrderImbalanceIndicatorWindow>()),
      m_userProfile(userProfile.get()) {
  m_ui->setupUi(this);
  resize(scale(size()));
  m_ui->m_orderImbalanceIndicatorTableView->setItemDelegate(
    new CustomVariantItemDelegate(Ref(*m_userProfile)));
  for(auto i = 0; i < OrderImbalanceIndicatorModel::COLUMN_COUNT; ++i) {
    m_ui->m_orderImbalanceIndicatorTableView->setColumnWidth(
      i, DEFAULT_COLUMN_SIZES[i]);
  }
  auto metrics = QFontMetrics(m_ui->m_orderImbalanceIndicatorTableView->font());
  m_ui->m_orderImbalanceIndicatorTableView->verticalHeader()->
    setDefaultSectionSize(metrics.height());
  SetModel(model);
}

const std::shared_ptr<OrderImbalanceIndicatorModel>&
    OrderImbalanceIndicatorWindow::GetModel() const {
  return m_model;
}

void OrderImbalanceIndicatorWindow::SetModel(
    const std::shared_ptr<OrderImbalanceIndicatorModel>& model) {
  auto previousModel = m_model;
  auto previousProxyModel = std::move(m_proxyModel);
  m_model = model;
  m_proxyModel =
    std::make_unique<CustomVariantSortFilterProxyModel>(Ref(*m_userProfile));
  m_proxyModel->setSourceModel(m_model.get());
  m_ui->m_orderImbalanceIndicatorTableView->setModel(m_proxyModel.get());
  auto isTimeRangeExpanded = [&] {
    if(!previousModel) {
      return false;
    } else {
      return m_ui->m_timeRangeParametersWidget->IsExpanded();
    }
  }();
  auto timeRangeInputWidget = new TimeRangeInputWidget();
  timeRangeInputWidget->SetTimeRange(
    m_model->GetProperties().m_startTime, m_model->GetProperties().m_endTime);
  m_timeRangeConnection = timeRangeInputWidget->ConnectTimeRangeUpdatedSignal(
    std::bind_front(&OrderImbalanceIndicatorWindow::OnTimeRangeUpdated, this));
  m_ui->m_timeRangeParametersWidget->Initialize(
    "Time Range", timeRangeInputWidget, isTimeRangeExpanded);
  auto isMarketSelectionExpanded = [&] {
    if(!previousModel) {
      return false;
    } else {
      return m_ui->m_marketSelectionWidget->IsExpanded();
    }
  }();
  m_ui->m_marketSelectionWidget->Initialize(
    "Markets", new OrderImbalanceIndicatorVenueSelectionWidget(Ref(*m_model)),
    isMarketSelectionExpanded);
}

std::unique_ptr<WindowSettings>
    OrderImbalanceIndicatorWindow::GetWindowSettings() const {
  return std::make_unique<OrderImbalanceIndicatorWindowSettings>(
    *this, Ref(*m_userProfile));
}

void OrderImbalanceIndicatorWindow::closeEvent(QCloseEvent* event) {
  auto settings = std::make_shared<OrderImbalanceIndicatorWindowSettings>(
    *this, Ref(*m_userProfile));
  m_userProfile->SetInitialOrderImbalanceIndicatorWindowSettings(*settings);
  m_userProfile->SetDefaultOrderImbalanceIndicatorProperties(
    m_model->GetProperties());
  m_userProfile->GetRecentlyClosedWindows()->push(std::move(settings));
  QFrame::closeEvent(event);
}

void OrderImbalanceIndicatorWindow::OnTimeRangeUpdated(
    const TimeRangeParameter& startTime, const TimeRangeParameter& endTime) {
  if(m_model) {
    m_model->UpdateTimeRange(startTime, endTime);
  }
}
