#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include <array>
#include <QEvent>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include "Spire/InputWidgets/SecurityInputDialog.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/LinkSecurityContextAction.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/LegacyUI/ValueLabel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesPropertiesDialog.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindowSettings.hpp"
#include "Spire/Utilities/ExportModel.hpp"
#include "Spire/Utilities/SecurityTechnicalsModel.hpp"
#include "ui_TimeAndSalesWindow.h"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  auto DEFAULT_COLUMN_SIZES =
    std::array<int, TimeAndSalesModel::COLUMN_COUNT>{60, 60, 40, 60, 60};
}

TimeAndSalesWindow::TimeAndSalesWindow(Ref<UserProfile> userProfile,
    const TimeAndSalesProperties& properties, const std::string& identifier,
    QWidget* parent, Qt::WindowFlags flags)
    : QFrame(parent, flags),
      SecurityContext(identifier),
      m_ui(std::make_unique<Ui_TimeAndSalesWindow>()),
      m_userProfile(userProfile.get()) {
  m_ui->setupUi(this);
  resize(scale(size()));
  m_ui->m_timeAndSalesView->setItemDelegate(
    new CustomVariantItemDelegate(Ref(*m_userProfile)));
  m_ui->m_snapshotView->setItemDelegate(
    new CustomVariantItemDelegate(Ref(*m_userProfile)));
  connect(m_ui->m_timeAndSalesView, &QTableView::customContextMenuRequested,
    this, &TimeAndSalesWindow::OnContextMenu);
  connect(m_ui->m_timeAndSalesView->horizontalHeader(),
    &QHeaderView::sectionMoved, this, &TimeAndSalesWindow::OnSectionMoved);
  connect(m_ui->m_snapshotView, &QTableView::customContextMenuRequested, this,
    &TimeAndSalesWindow::OnContextMenu);
  connect(m_ui->m_timeAndSalesView->horizontalHeader(),
    &QHeaderView::sectionResized, this, &TimeAndSalesWindow::OnSectionResized);
  m_model = std::make_unique<TimeAndSalesModel>(
    Ref(*m_userProfile), properties, Security());
  m_ui->m_timeAndSalesView->horizontalHeader()->setSectionsMovable(true);
  m_ui->m_timeAndSalesView->horizontalHeader()->setMinimumSectionSize(1);
  m_ui->m_timeAndSalesView->verticalHeader()->setMinimumSectionSize(0);
  m_ui->m_timeAndSalesView->setModel(m_model.get());
  m_ui->m_snapshotView->horizontalHeader()->setSectionsMovable(true);
  m_ui->m_snapshotView->horizontalHeader()->setMinimumSectionSize(1);
  m_ui->m_snapshotView->verticalHeader()->setMinimumSectionSize(0);
  m_ui->m_snapshotView->setModel(m_model.get());
  auto splitterSizes = m_ui->m_splitter->sizes();
  splitterSizes.back() = 0;
  m_ui->m_splitter->setSizes(splitterSizes);
  m_statusBar = new QStatusBar(this);
  auto sizePolicy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(m_statusBar->sizePolicy().hasHeightForWidth());
  m_statusBar->setSizePolicy(sizePolicy);
  m_ui->verticalLayout->addWidget(m_statusBar);
  m_volumeLabel = new ValueLabel("Volume", Ref(*m_userProfile));
  m_volumeLabel->AdjustSize(QVariant::fromValue(999999999));
  m_statusBar->setStyleSheet("QStatusBar::item { border: 0px solid black };");
  m_statusBar->addWidget(m_volumeLabel);
  setWindowTitle(tr("Time and Sales"));
  SetProperties(properties);
  for(auto i = 0; i < TimeAndSalesModel::COLUMN_COUNT; ++i) {
    m_ui->m_timeAndSalesView->setColumnWidth(i, DEFAULT_COLUMN_SIZES[i]);
  }
  SetupSecurityTechnicals();
}

const TimeAndSalesProperties& TimeAndSalesWindow::GetProperties() const {
  return m_properties;
}

void TimeAndSalesWindow::SetProperties(
    const TimeAndSalesProperties& properties) {
  m_properties = properties;
  if(m_properties.IsHorizontalScrollBarVisible()) {
    m_ui->m_timeAndSalesView->setHorizontalScrollBarPolicy(
      Qt::ScrollBarAsNeeded);
    m_ui->m_snapshotView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  } else {
    m_ui->m_timeAndSalesView->setHorizontalScrollBarPolicy(
      Qt::ScrollBarAlwaysOff);
    m_ui->m_snapshotView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  }
  if(m_properties.IsVerticalScrollBarVisible()) {
    m_ui->m_timeAndSalesView->setVerticalScrollBarPolicy(
      Qt::ScrollBarAlwaysOn);
    m_ui->m_snapshotView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  } else {
    m_ui->m_timeAndSalesView->setVerticalScrollBarPolicy(
      Qt::ScrollBarAlwaysOff);
    m_ui->m_snapshotView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  }
  for(auto i = 0; i < TimeAndSalesProperties::COLUMN_COUNT; ++i) {
    if(m_ui->m_timeAndSalesView->isColumnHidden(i) &&
        m_properties.GetVisibleColumns()[i]) {
      m_ui->m_timeAndSalesView->setColumnWidth(i,
        std::max(DEFAULT_COLUMN_SIZES[i],
        m_ui->m_timeAndSalesView->columnWidth(i)));
    }
    m_ui->m_timeAndSalesView->setColumnHidden(
      i, !m_properties.GetVisibleColumns()[i]);
    m_ui->m_snapshotView->setColumnHidden(
      i, !m_properties.GetVisibleColumns()[i]);
    m_ui->m_snapshotView->setColumnWidth(
      i, m_ui->m_timeAndSalesView->columnWidth(i));
  }
  m_ui->m_timeAndSalesView->setFont(m_properties.GetFont());
  m_ui->m_snapshotView->setFont(m_properties.GetFont());
  auto metrics = QFontMetrics(m_properties.GetFont());
  m_ui->m_timeAndSalesView->verticalHeader()->setDefaultSectionSize(
    metrics.height());
  m_ui->m_snapshotView->verticalHeader()->setDefaultSectionSize(
    metrics.height());
  m_model->SetProperties(m_properties);
  m_ui->m_timeAndSalesView->setShowGrid(m_properties.GetShowGridLines());
  m_ui->m_snapshotView->setShowGrid(m_properties.GetShowGridLines());
}

void TimeAndSalesWindow::DisplaySecurity(const Security& security) {
  m_security = security;
  setWindowTitle(QString::fromStdString(to_string(m_security)) +
    tr(" - Time and Sales"));
  auto widths = std::vector<int>();
  for(auto i = 0; i < TimeAndSalesProperties::COLUMN_COUNT; ++i) {
    widths.push_back(m_ui->m_timeAndSalesView->columnWidth(i));
  }
  auto newModel = std::make_unique<TimeAndSalesModel>(
    Ref(*m_userProfile), m_properties, m_security);
  m_ui->m_timeAndSalesView->setModel(newModel.get());
  m_model = std::move(newModel);
  m_ui->m_timeAndSalesView->setModel(m_model.get());
  m_ui->m_snapshotView->setModel(m_model.get());
  for(auto i = 0; i < TimeAndSalesProperties::COLUMN_COUNT; ++i) {
    m_ui->m_timeAndSalesView->setColumnWidth(i, widths[i]);
  }
  SetupSecurityTechnicals();
  SetDisplayedSecurity(m_security);
}

std::unique_ptr<WindowSettings> TimeAndSalesWindow::GetWindowSettings() const {
  return std::make_unique<TimeAndSalesWindowSettings>(
    *this, Ref(*m_userProfile));
}

void TimeAndSalesWindow::showEvent(QShowEvent* event) {
  if(auto context = SecurityContext::FindSecurityContext(m_linkIdentifier)) {
    Link(*context);
  } else {
    m_linkConnection.disconnect();
    m_linkIdentifier.clear();
  }
  QFrame::showEvent(event);
  m_ui->m_timeAndSalesView->horizontalHeader()->setStretchLastSection(true);
  m_ui->m_snapshotView->horizontalHeader()->setStretchLastSection(true);
}

void TimeAndSalesWindow::closeEvent(QCloseEvent* event) {
  if(m_security != Security()) {
    auto window =
      std::make_shared<TimeAndSalesWindowSettings>(*this, Ref(*m_userProfile));
    m_userProfile->GetRecentlyClosedWindows()->push(std::move(window));
  }
  QFrame::closeEvent(event);
}

void TimeAndSalesWindow::keyPressEvent(QKeyEvent* event) {
  auto key = event->key();
  if(key == Qt::Key_PageUp) {
    m_securityViewStack.PushUp(m_security, [&] (const auto& security) {
      DisplaySecurity(security);
    });
    return;
  } else if(key == Qt::Key_PageDown) {
    m_securityViewStack.PushDown(m_security, [&] (const auto& security) {
      DisplaySecurity(security);
    });
    return;
  }
  auto text = event->text();
  if(text.isEmpty() || !text[0].isLetterOrNumber()) {
    return;
  }
  ShowSecurityInputDialog(Ref(*m_userProfile), text.toStdString(), this,
    [=] (auto security) {
      if(!security || security == Security() || security == m_security) {
        return;
      }
      m_securityViewStack.Push(m_security);
      DisplaySecurity(*security);
    });
}

void TimeAndSalesWindow::HandleLink(SecurityContext& context) {
  m_linkIdentifier = context.GetIdentifier();
  m_linkConnection = context.ConnectSecurityDisplaySignal(
    std::bind_front(&TimeAndSalesWindow::DisplaySecurity, this));
  DisplaySecurity(context.GetDisplayedSecurity());
}

void TimeAndSalesWindow::HandleUnlink() {
  m_linkConnection.disconnect();
  m_linkIdentifier.clear();
}

void TimeAndSalesWindow::SetupSecurityTechnicals() {
  m_volumeLabel->Reset();
  m_volumeConnection.disconnect();
  m_securityTechnicalsModel =
    SecurityTechnicalsModel::GetModel(Ref(*m_userProfile), m_security);
  m_volumeConnection = m_securityTechnicalsModel->ConnectVolumeSignal(
    std::bind_front(&TimeAndSalesWindow::OnVolumeUpdate, this));
}

void TimeAndSalesWindow::OnVolumeUpdate(Quantity volume) {
  m_volumeLabel->SetValue(static_cast<int>(volume));
}

void TimeAndSalesWindow::OnContextMenu(const QPoint& position) {
  auto contextMenu = QMenu();
  auto propertiesAction = QAction(&contextMenu);
  propertiesAction.setText(tr("Properties"));
  propertiesAction.setToolTip(tr("Opens the Time and Sales properties."));
  contextMenu.addAction(&propertiesAction);
  auto linkMenu = QMenu("Links");
  auto linkActions = LinkSecurityContextAction::MakeActions(
    this, m_linkIdentifier, &linkMenu, *m_userProfile);
  for(auto& action : linkActions) {
    linkMenu.addAction(action.get());
  }
  if(!linkMenu.isEmpty()) {
    contextMenu.addMenu(&linkMenu);
  }
  auto exportAction = QAction(&contextMenu);
  exportAction.setEnabled(m_model->rowCount(QModelIndex()) != 0);
  exportAction.setText(tr("Export To File"));
  contextMenu.addAction(&exportAction);
  auto selectedAction =
    contextMenu.exec(static_cast<QWidget*>(sender())->mapToGlobal(position));
  if(selectedAction == &propertiesAction) {
    auto propertiesWidget =
      TimeAndSalesPropertiesDialog(Ref(*m_userProfile), m_properties, this);
    if(propertiesWidget.exec() == QDialog::Rejected) {
      return;
    }
    SetProperties(propertiesWidget.GetProperties());
  } else if(selectedAction == &exportAction) {
    auto path = QFileDialog::getSaveFileName(
      this, tr("Select file to export to."), QStandardPaths::writableLocation(
        QStandardPaths::DocumentsLocation) + "/time_and_sales.csv",
      "CSV (*.csv)");
    if(path.isNull()) {
      return;
    }
    auto exportFile = std::ofstream(path.toStdString());
    ExportModelAsCsv(*m_userProfile, *m_model, exportFile);
  } else if(auto linkAction =
      dynamic_cast<LinkSecurityContextAction*>(selectedAction)) {
    linkAction->Execute(out(*this));
  }
}

void TimeAndSalesWindow::OnSectionMoved(
    int logicalIndex, int oldVisualIndex, int newVisualIndex) {
  m_ui->m_snapshotView->horizontalHeader()->moveSection(
    oldVisualIndex, newVisualIndex);
}

void TimeAndSalesWindow::OnSectionResized(
    int logicalIndex, int oldSize, int newSize) {
  m_ui->m_snapshotView->horizontalHeader()->resizeSection(
    logicalIndex, newSize);
}
