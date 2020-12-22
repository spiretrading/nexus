#include "Spire/Charting/ChartWindow.hpp"
#include <QKeyEvent>
#include <QLineEdit>
#include <QToolButton>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/Types/DurationType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Charting/ChartIntervalComboBox.hpp"
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/Charting/ChartWindowSettings.hpp"
#include "Spire/Charting/SecurityTimePriceChartPlotSeries.hpp"
#include "Spire/InputWidgets/SecurityInputDialog.hpp"
#include "Spire/UI/LinkSecurityContextAction.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_ChartWindow.h"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::TechnicalAnalysis;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  const unsigned int UPDATE_INTERVAL = 100;
}

ChartWindow::ChartWindow(Ref<UserProfile> userProfile,
    const string& identifier, QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      SecurityContext(identifier),
      m_ui(std::make_unique<Ui_ChartWindow>()),
      m_userProfile(userProfile.Get()),
      m_interactionMode(ChartInteractionMode::NONE) {
  m_ui->setupUi(this);
  m_intervalComboBox = new ChartIntervalComboBox(this);
  m_ui->m_toolBar->insertWidget(m_ui->m_panAction, m_intervalComboBox);
  m_ui->m_toolBar->insertSeparator(m_ui->m_panAction);
  m_ui->m_verticalScrollBar->SetOrientation(Qt::Vertical);
  m_ui->m_verticalScrollBar->SetInverted(true);
  m_ui->m_verticalScrollBar->SetSingleStep(2);
  m_ui->m_horizontalScrollBar->SetOrientation(Qt::Horizontal);
  m_ui->m_horizontalScrollBar->SetSingleStep(2);
  m_ui->m_chart->Initialize(Ref(*m_userProfile),
    ChartPlotView::Properties::GetDefault());
  auto currentTime =
    m_userProfile->GetServiceClients().GetTimeClient().GetTime();
  ChartPlotView::AxisParameters xAxisParameters(DateTimeType::GetInstance(),
    ChartValue(currentTime - hours(1)), ChartValue(currentTime + hours(1)),
    ChartValue(minutes(1)));
  m_ui->m_chart->SetXAxisParameters(xAxisParameters);
  ChartPlotView::AxisParameters yAxisParameters(MoneyType::GetInstance(),
    ChartValue(Money::ONE), ChartValue(Money::ONE + 10 * Money::CENT),
    ChartValue(Money::CENT));
  m_ui->m_chart->SetYAxisParameters(yAxisParameters);
  connect(&m_updateTimer, &QTimer::timeout, this, &ChartWindow::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
  m_controller.emplace();
  m_controller->SetView(Ref(*m_ui->m_chart));
  m_verticalSliderConnection =
    m_ui->m_verticalScrollBar->ConnectSliderChangedSignal(
    std::bind(&ChartWindow::OnVerticalSliderChanged, this,
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
    std::placeholders::_4));
  m_ui->m_verticalScrollBar->SetSliderRange(33, 66);
  m_horizontalSliderConnection =
    m_ui->m_horizontalScrollBar->ConnectSliderChangedSignal(
    std::bind(&ChartWindow::OnHorizontalSliderChanged, this,
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
    std::placeholders::_4));
  m_ui->m_horizontalScrollBar->SetSliderRange(33, 66);
  m_beginPanConnection = m_ui->m_chart->ConnectBeginPanSignal(std::bind(
    &ChartWindow::OnBeginPan, this));
  m_endPanConnection = m_ui->m_chart->ConnectEndPanSignal(std::bind(
    &ChartWindow::OnEndPan, this));
  m_linkMenu = new QMenu("Links", this);
  m_ui->m_linkAction->setMenu(m_linkMenu);
  auto linkMenuButton = dynamic_cast<QToolButton*>(
    m_ui->m_toolBar->widgetForAction(m_ui->m_linkAction));
  linkMenuButton->setStyleSheet(
    "QToolButton::menu-indicator { image: none; }");
  connect(m_ui->m_panAction, &QAction::toggled, this,
    &ChartWindow::OnPanActionToggled);
  connect(m_ui->m_zoomAction, &QAction::toggled, this,
    &ChartWindow::OnZoomActionToggled);
  connect(m_ui->m_autoScaleAction, &QAction::toggled, this,
    &ChartWindow::OnAutoScaleActionToggled);
  connect(m_ui->m_lockGridAction, &QAction::toggled, this,
    &ChartWindow::OnLockGridActionToggled);
  connect(m_ui->m_linkAction, &QAction::triggered, this,
    &ChartWindow::OnLinkMenuActionTriggered);
  connect(m_linkMenu, &QMenu::triggered, this,
    &ChartWindow::OnLinkActionTriggered);
  m_ui->m_chart->setFocus();
  m_intervalChangedConnection =
    m_intervalComboBox->ConnectIntervalChangedSignal(std::bind(
    &ChartWindow::OnIntervalChanged, this, std::placeholders::_1,
    std::placeholders::_2));
  OnIntervalChanged(m_intervalComboBox->GetType(),
    m_intervalComboBox->GetValue());
  SetInteractionMode(ChartInteractionMode::PAN);
  SetAutoScale(true);
  SetLockGrid(true);
}

ChartWindow::~ChartWindow() {}

ChartInteractionMode ChartWindow::GetInteractionMode() const {
  return m_interactionMode;
}

void ChartWindow::SetInteractionMode(ChartInteractionMode interactionMode) {
  if(m_interactionMode == interactionMode) {
    return;
  }
  m_interactionMode = interactionMode;
  m_ui->m_chart->SetInteractionMode(m_interactionMode);
  UpdateInteractionMode();
}

bool ChartWindow::IsAutoScaleEnabled() const {
  return m_controller->IsAutoScaleEnabled();
}

void ChartWindow::SetAutoScale(bool autoScale) {
  m_controller->SetAutoScale(autoScale);
  m_ui->m_autoScaleAction->setChecked(m_controller->IsAutoScaleEnabled());
}

bool ChartWindow::IsLockGridEnabled() const {
  return m_controller->IsLockGridEnabled();
}

void ChartWindow::SetLockGrid(bool lockGrid) {
  m_controller->SetLockGrid(lockGrid);
  m_ui->m_lockGridAction->setChecked(m_controller->IsLockGridEnabled());
}

void ChartWindow::DisplaySecurity(const Security& security) {
  m_security = security;
  if(m_security == Security()) {
    setWindowTitle(tr("Chart - Spire"));
  } else {
    setWindowTitle(QString::fromStdString(
      ToString(m_security, m_userProfile->GetMarketDatabase())) +
      tr(" - Chart"));
    OnIntervalChanged(m_intervalComboBox->GetType(),
      m_intervalComboBox->GetValue());
  }
  SetDisplayedSecurity(m_security);
}

std::unique_ptr<WindowSettings> ChartWindow::GetWindowSettings() const {
  return std::make_unique<ChartWindowSettings>(*this, Ref(*m_userProfile));
}

void ChartWindow::showEvent(QShowEvent* event) {
  auto context = SecurityContext::FindSecurityContext(m_linkIdentifier);
  if(context.is_initialized()) {
    Link(*context);
  } else {
    m_linkConnection.disconnect();
    m_linkIdentifier.clear();
  }
  QMainWindow::showEvent(event);
}

void ChartWindow::closeEvent(QCloseEvent* event) {
  if(m_security != Security()) {
    auto settings = GetWindowSettings();
    m_userProfile->AddRecentlyClosedWindow(std::move(settings));
  }
  QMainWindow::closeEvent(event);
}

void ChartWindow::keyPressEvent(QKeyEvent* event) {
  auto key = event->key();
  if(key == Qt::Key_PageUp) {
    m_securityViewStack.PushUp(m_security,
      [&] (const Security& security) {
        DisplaySecurity(security);
      });
    return;
  } else if(key == Qt::Key_PageDown) {
    m_securityViewStack.PushDown(m_security,
      [&] (const Security& security) {
        this->DisplaySecurity(security);
      });
    return;
  }
  auto text = event->text();
  if(text.isEmpty() || !text[0].isLetterOrNumber()) {
    return;
  }
  SecurityInputDialog dialog(Ref(*m_userProfile), text.toStdString(), this);
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto security = dialog.GetSecurity();
  if(security == Security() || security == m_security) {
    return;
  }
  m_securityViewStack.Push(m_security);
  DisplaySecurity(security);
}

void ChartWindow::HandleLink(SecurityContext& context) {
  m_linkIdentifier = context.GetIdentifier();
  m_linkConnection = context.ConnectSecurityDisplaySignal(
    std::bind(&ChartWindow::DisplaySecurity, this, std::placeholders::_1));
  DisplaySecurity(context.GetDisplayedSecurity());
}

void ChartWindow::HandleUnlink() {
  m_linkConnection.disconnect();
  m_linkIdentifier.clear();
}

void ChartWindow::AdjustSlider(int previousMinimum, int previousMaximum,
    int minimum, int maximum, ScalableScrollBar* scrollBar) {
  const ChartPlotView::AxisParameters* axisParameters;
  if(scrollBar == m_ui->m_horizontalScrollBar) {
    axisParameters = &m_ui->m_chart->GetXAxisParameters();
  } else {
    axisParameters = &m_ui->m_chart->GetYAxisParameters();
  }
  auto updatedAxisParameters = *axisParameters;
  auto scrollRange = scrollBar->GetMaximum() - scrollBar->GetMinimum();
  auto chartRange = axisParameters->m_max - axisParameters->m_min;
  auto minScrollDelta = minimum - previousMinimum;
  auto minChartDelta = (minScrollDelta * chartRange) / scrollRange;
  updatedAxisParameters.m_min = axisParameters->m_min + minChartDelta;
  auto maxScrollDelta = maximum - previousMaximum;
  auto maxChartDelta = (maxScrollDelta * chartRange) / scrollRange;
  updatedAxisParameters.m_max = axisParameters->m_max + maxChartDelta;
  if(scrollBar == m_ui->m_horizontalScrollBar) {
    m_ui->m_chart->SetXAxisParameters(updatedAxisParameters);
  } else {
    m_ui->m_chart->SetYAxisParameters(updatedAxisParameters);
  }
  auto isSliderAtMaximum = maximum >= scrollBar->GetMaximum();
  auto isSliderAtMinimum = minimum <= scrollBar->GetMinimum();
  if(isSliderAtMaximum || isSliderAtMinimum) {
    auto sliderDelta = maximum - minimum;
    int sliderMinimum;
    int sliderMaximum;
    if(isSliderAtMaximum) {
      if(previousMinimum == minimum) {
        sliderMaximum = 34;
      } else {
        sliderMaximum = sliderDelta + 1;
      }
      sliderMinimum = 1;
    } else {
      if(previousMaximum == maximum) {
        sliderMinimum = 66;
      } else {
        sliderMinimum = scrollBar->GetMaximum() - sliderDelta - 1;
      }
      sliderMaximum = scrollBar->GetMaximum() - 1;
    }
    scoped_connection* blockedConnection;
    if(scrollBar == m_ui->m_horizontalScrollBar) {
      blockedConnection = &m_horizontalSliderConnection;
    } else {
      blockedConnection = &m_verticalSliderConnection;
    }
    shared_connection_block block(*blockedConnection);
    scrollBar->SetSliderRange(sliderMinimum, sliderMaximum);
  }
}

void ChartWindow::UpdateInteractionMode() {
  m_ui->m_panAction->setChecked(m_interactionMode == ChartInteractionMode::PAN);
  m_ui->m_zoomAction->setChecked(m_interactionMode ==
    ChartInteractionMode::ZOOM);
}

void ChartWindow::OnUpdateTimer() {
  HandleTasks(m_slotHandler);
}

void ChartWindow::OnVerticalSliderChanged(int previousMinimum,
    int previousMaximum, int minimum, int maximum) {
  SetLockGrid(false);
  AdjustSlider(previousMinimum, previousMaximum, minimum, maximum,
    m_ui->m_verticalScrollBar);
}

void ChartWindow::OnHorizontalSliderChanged(int previousMinimum,
    int previousMaximum, int minimum, int maximum) {
  SetLockGrid(false);
  AdjustSlider(previousMinimum, previousMaximum, minimum, maximum,
    m_ui->m_horizontalScrollBar);
}

void ChartWindow::OnIntervalChanged(const std::shared_ptr<NativeType>& type,
    ChartValue value) {
  m_ui->m_chart->setFocus(Qt::ActiveWindowFocusReason);
  if(m_security == Security()) {
    return;
  }
  m_controller->Clear();
  auto axisParameters = m_ui->m_chart->GetXAxisParameters();
  double scaleFactor;
  if(axisParameters.m_increment != ChartValue() &&
      axisParameters.m_type->GetCompatibility(DateTimeType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL && type->GetCompatibility(
      DurationType::GetInstance()) == CanvasType::Compatibility::EQUAL) {
    scaleFactor = value / axisParameters.m_increment;
  } else {
    scaleFactor = 1.0;
  }
  axisParameters.m_increment = value;
  if(scaleFactor != 1.0) {
    auto range = axisParameters.m_max - axisParameters.m_min;
    auto newRange = scaleFactor * range;
    axisParameters.m_min = axisParameters.m_max - newRange;
  }
  m_ui->m_chart->SetXAxisParameters(axisParameters);
  std::shared_ptr<ChartPlotSeries> chartPlotSeries;
  if(type->GetCompatibility(DurationType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL) {
    chartPlotSeries = std::make_shared<SecurityTimePriceChartPlotSeries>(
      Ref(*m_userProfile), m_security, value.ToTimeDuration());
  }
  if(chartPlotSeries != nullptr) {
    m_controller->Add(chartPlotSeries);
  }
}

void ChartWindow::OnBeginPan() {
  m_xPan = m_ui->m_chart->GetXAxisParameters().m_max;
  m_yPan = m_ui->m_chart->GetYAxisParameters().m_max;
  SetLockGrid(false);
}

void ChartWindow::OnEndPan() {
  const auto& xParameters = m_ui->m_chart->GetXAxisParameters();
  const auto& yParameters = m_ui->m_chart->GetYAxisParameters();
  auto ChartValueAbs =
    [] (ChartValue value) {
      if(value >= ChartValue()) {
        return value;
      }
      return ChartValue() - value;
    };
  if(ChartValueAbs(m_xPan - xParameters.m_max) >= xParameters.m_increment ||
      ChartValueAbs(m_yPan - yParameters.m_max) >= yParameters.m_increment) {
    SetLockGrid(false);
  } else {
    SetLockGrid(true);
  }
}

void ChartWindow::OnPanActionToggled(bool toggled) {
  if(toggled) {
    SetInteractionMode(ChartInteractionMode::PAN);
  } else if(m_interactionMode == ChartInteractionMode::PAN) {
    SetInteractionMode(ChartInteractionMode::NONE);
  }
}

void ChartWindow::OnZoomActionToggled(bool toggled) {
  if(toggled) {
    SetInteractionMode(ChartInteractionMode::ZOOM);
  } else if(m_interactionMode == ChartInteractionMode::ZOOM) {
    SetInteractionMode(ChartInteractionMode::NONE);
  }
}

void ChartWindow::OnAutoScaleActionToggled(bool toggled) {
  SetAutoScale(toggled);
}

void ChartWindow::OnLockGridActionToggled(bool toggled) {
  SetLockGrid(toggled);
}

void ChartWindow::OnLinkMenuActionTriggered(bool triggered) {
  auto actions = m_linkMenu->actions();
  for(auto& i : actions) {
    std::unique_ptr<QAction> action(i);
    m_linkMenu->removeAction(action.get());
  }
  m_linkMenu->clear();
  auto linkActions = LinkSecurityContextAction::MakeActions(this,
    m_linkIdentifier, m_linkMenu, *m_userProfile);
  for(const auto& linkAction : linkActions) {
    m_linkMenu->addAction(linkAction.get());
  }
  if(m_linkMenu->isEmpty()) {
    auto disabledAction = m_linkMenu->addAction(tr("No links available."));
    disabledAction->setEnabled(false);
  }
  auto linkButton = dynamic_cast<QToolButton*>(
    m_ui->m_toolBar->widgetForAction(m_ui->m_linkAction));
  linkButton->showMenu();
}

void ChartWindow::OnLinkActionTriggered(QAction* action) {
  auto linkAction = dynamic_cast<LinkSecurityContextAction*>(action);
  if(linkAction == nullptr) {
    return;
  }
  linkAction->Execute(Store(*this));
}
