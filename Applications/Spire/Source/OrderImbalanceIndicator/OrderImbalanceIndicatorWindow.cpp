#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindow.hpp"
#include "Spire/InputWidgets/TimeRangeInputWidget.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorMarketSelectionWidget.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindowSettings.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_OrderImbalanceIndicatorWindow.h"

using namespace Beam;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  std::array<int, OrderImbalanceIndicatorModel::COLUMN_COUNT>
    DEFAULT_COLUMN_SIZES = {65, 70, 60, 100, 90, 120, 120};
}

OrderImbalanceIndicatorWindow::OrderImbalanceIndicatorWindow(
    Ref<UserProfile> userProfile,
    const std::shared_ptr<OrderImbalanceIndicatorModel>& model, QWidget* parent,
    Qt::WindowFlags flags)
    : QFrame(parent, flags),
      m_ui(std::make_unique<Ui_OrderImbalanceIndicatorWindow>()),
      m_userProfile(userProfile.Get()) {
  m_ui->setupUi(this);
  m_ui->m_orderImbalanceIndicatorTableView->setItemDelegate(
    new CustomVariantItemDelegate(Ref(*m_userProfile)));
  for(int i = 0; i < OrderImbalanceIndicatorModel::COLUMN_COUNT; ++i) {
    m_ui->m_orderImbalanceIndicatorTableView->setColumnWidth(i,
      DEFAULT_COLUMN_SIZES[i]);
  }
  QFontMetrics metrics(m_ui->m_orderImbalanceIndicatorTableView->font());
  m_ui->m_orderImbalanceIndicatorTableView->verticalHeader()->
    setDefaultSectionSize(metrics.height());
  SetModel(model);
}

OrderImbalanceIndicatorWindow::~OrderImbalanceIndicatorWindow() {}

const std::shared_ptr<OrderImbalanceIndicatorModel>&
    OrderImbalanceIndicatorWindow::GetModel() const {
  return m_model;
}

void OrderImbalanceIndicatorWindow::SetModel(
    const std::shared_ptr<OrderImbalanceIndicatorModel>& model) {
  std::shared_ptr<OrderImbalanceIndicatorModel> previousModel = m_model;
  std::unique_ptr<CustomVariantSortFilterProxyModel> previousProxyModel =
    std::move(m_proxyModel);
  m_model = model;
  m_proxyModel = std::make_unique<CustomVariantSortFilterProxyModel>(
    Ref(*m_userProfile));
  m_proxyModel->setSourceModel(m_model.get());
  m_ui->m_orderImbalanceIndicatorTableView->setModel(m_proxyModel.get());
  bool isTimeRangeExpanded;
  if(previousModel == nullptr) {
    isTimeRangeExpanded = false;
  } else {
    isTimeRangeExpanded = m_ui->m_timeRangeParametersWidget->IsExpanded();
  }
  TimeRangeInputWidget* timeRangeInputWidget = new TimeRangeInputWidget();
  timeRangeInputWidget->SetTimeRange(m_model->GetProperties().m_startTime,
    m_model->GetProperties().m_endTime);
  m_timeRangeConnection = timeRangeInputWidget->ConnectTimeRangeUpdatedSignal(
    std::bind(&OrderImbalanceIndicatorWindow::OnTimeRangeUpdated, this,
    std::placeholders::_1, std::placeholders::_2));
  m_ui->m_timeRangeParametersWidget->Initialize("Time Range",
    timeRangeInputWidget, isTimeRangeExpanded);
  bool isMarketSelectionExpanded;
  if(previousModel == nullptr) {
    isMarketSelectionExpanded = false;
  } else {
    isMarketSelectionExpanded = m_ui->m_marketSelectionWidget->IsExpanded();
  }
  m_ui->m_marketSelectionWidget->Initialize("Markets",
    new OrderImbalanceIndicatorMarketSelectionWidget(
    m_userProfile->GetMarketDatabase(), Ref(*m_model)),
    isMarketSelectionExpanded);
}

unique_ptr<WindowSettings> OrderImbalanceIndicatorWindow::
    GetWindowSettings() const {
  unique_ptr<WindowSettings> settings =
    std::make_unique<OrderImbalanceIndicatorWindowSettings>(*this,
    Ref(*m_userProfile));
  return settings;
}

void OrderImbalanceIndicatorWindow::closeEvent(QCloseEvent* event) {
  unique_ptr<OrderImbalanceIndicatorWindowSettings> settings =
    std::make_unique<OrderImbalanceIndicatorWindowSettings>(*this,
    Ref(*m_userProfile));
  m_userProfile->SetInitialOrderImbalanceIndicatorWindowSettings(*settings);
  m_userProfile->SetDefaultOrderImbalanceIndicatorProperties(
    m_model->GetProperties());
  m_userProfile->AddRecentlyClosedWindow(std::move(settings));
  QFrame::closeEvent(event);
}

void OrderImbalanceIndicatorWindow::OnTimeRangeUpdated(
    const TimeRangeParameter& startTime, const TimeRangeParameter& endTime) {
  if(m_model == nullptr) {
    return;
  }
  m_model->UpdateTimeRange(startTime, endTime);
}
