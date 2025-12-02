#include "Spire/Blotter/BlotterMonitorsDialog.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterTaskMonitor.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/UI/CanvasWindow.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_BlotterMonitorsDialog.h"

using namespace Beam;
using namespace Spire;
using namespace Spire::UI;

BlotterMonitorsDialog::BlotterMonitorsDialog(Ref<UserProfile> userProfile,
    const BlotterTaskProperties& properties, Ref<BlotterModel> model,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_BlotterMonitorsDialog>()),
      m_userProfile(userProfile.get()),
      m_propeties(properties),
      m_model(model.get()) {
  m_ui->setupUi(this);
  m_ui->m_monitorTable->setHorizontalHeaderLabels(QStringList("Monitors"));
  m_ui->m_monitorTable->setMinimumWidth(1);
  m_ui->m_monitorTable->adjustSize();
  m_ui->m_splitter->setStretchFactor(0, 0);
  m_ui->m_splitter->setStretchFactor(1, 1);
  m_ui->m_splitter->setSizes(QList<int>() <<
    m_ui->m_monitorTable->minimumWidth() <<
    m_ui->m_splitter->widget(1)->width());
  m_ui->m_monitorTable->setRowCount(m_propeties.GetMonitors().size());
  auto metrics = QFontMetrics(m_ui->m_monitorTable->font());
  m_ui->m_monitorTable->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
  for(auto i = m_propeties.GetMonitors().begin();
      i != m_propeties.GetMonitors().end(); ++i) {
    auto monitorItem =
      new QTableWidgetItem(QString::fromStdString(i->GetName()));
    m_ui->m_monitorTable->setItem(
      std::distance(m_propeties.GetMonitors().begin(), i), 0, monitorItem);
  }
  connect(m_ui->m_monitorTable, &QTableWidget::cellChanged, this,
    &BlotterMonitorsDialog::OnMonitorItemDataChanged);
  connect(m_ui->m_monitorTable, &QTableWidget::currentCellChanged, this,
    &BlotterMonitorsDialog::OnMonitorItemSelectionChanged);
  connect(m_ui->m_addButton, &QPushButton::clicked, this,
    &BlotterMonitorsDialog::OnAddMonitor);
  connect(m_ui->m_deleteButton, &QPushButton::clicked, this,
    &BlotterMonitorsDialog::OnDeleteMonitor);
  connect(m_ui->m_loadDefaultButton, &QPushButton::clicked, this,
    &BlotterMonitorsDialog::OnLoadDefault);
  connect(m_ui->m_saveAsDefaultButton, &QPushButton::clicked, this,
    &BlotterMonitorsDialog::OnSaveAsDefault);
  connect(m_ui->m_resetDefaultButton, &QPushButton::clicked, this,
    &BlotterMonitorsDialog::OnResetDefault);
  connect(m_ui->m_okButton, &QPushButton::clicked, this,
    &BlotterMonitorsDialog::OnAccept);
  connect(m_ui->m_cancelButton, &QPushButton::clicked, this,
    &BlotterMonitorsDialog::reject);
  connect(m_ui->m_applyButton, &QPushButton::clicked, this,
    &BlotterMonitorsDialog::OnApply);
  connect(m_ui->m_applyToAllButton, &QPushButton::clicked, this,
    &BlotterMonitorsDialog::OnApplyToAll);
}

BlotterMonitorsDialog::~BlotterMonitorsDialog() = default;

const BlotterTaskProperties& BlotterMonitorsDialog::GetProperties() const {
  return m_propeties;
}

void BlotterMonitorsDialog::Commit() {
  if(m_currentRow < 0 ||
      m_currentRow >= static_cast<int>(m_propeties.GetMonitors().size())) {
    return;
  }
  auto canvasWindow =
    dynamic_cast<CanvasWindow*>(m_ui->m_canvasScrollArea->widget());
  if(!canvasWindow) {
    return;
  }
  auto roots = canvasWindow->GetCanvasNodeModel().GetRoots();
  for(auto& root : roots) {
    if(dynamic_cast<const BlotterTaskMonitorNode*>(root)) {
      auto monitor = BlotterTaskMonitor(
        m_propeties.GetMonitors()[m_currentRow].GetName(), *root);
      m_propeties.Replace(m_currentRow, monitor);
      return;
    }
  }
}

void BlotterMonitorsDialog::OnMonitorItemDataChanged(int row, int column) {
  if(row < 0 || row >= static_cast<int>(m_propeties.GetMonitors().size())) {
    return;
  }
  Commit();
  auto monitor = BlotterTaskMonitor(m_ui->m_monitorTable->item(
    row, column)->data(Qt::DisplayRole).value<QString>().toStdString(),
    m_propeties.GetMonitors()[row].GetMonitor());
  m_propeties.Replace(row, monitor);
}

void BlotterMonitorsDialog::OnMonitorItemSelectionChanged(
    int currentRow, int currentColumn, int previousRow, int previousColumn) {
  Commit();
  m_currentRow = currentRow;
  auto canvas = new CanvasWindow(Ref(*m_userProfile));
  m_ui->m_canvasScrollArea->setWidget(canvas);
  if(m_currentRow == -1) {
    canvas->setEnabled(false);
  } else {
    canvas->GetCanvasNodeModel().Add(CanvasNodeModel::Coordinate(0, 0),
      m_propeties.GetMonitors()[currentRow].GetMonitor());
  }
}

void BlotterMonitorsDialog::OnAddMonitor() {
  Commit();
  auto newMonitor = BlotterTaskMonitor("New Monitor", BlotterTaskMonitorNode());
  auto monitorItem =
    new QTableWidgetItem(QString::fromStdString(newMonitor.GetName()));
  m_propeties.Add(newMonitor);
  m_ui->m_monitorTable->setRowCount(m_propeties.GetMonitors().size());
  m_ui->m_monitorTable->setItem(
    m_propeties.GetMonitors().size() - 1, 0, monitorItem);
  m_ui->m_monitorTable->setCurrentItem(monitorItem);
  monitorItem->setSelected(true);
  m_ui->m_monitorTable->setFocus();
}

void BlotterMonitorsDialog::OnDeleteMonitor() {
  Commit();
  auto selectedRow = m_ui->m_monitorTable->currentRow();
  if(selectedRow < 0 || selectedRow >=
      static_cast<int>(m_propeties.GetMonitors().size())) {
    return;
  }
  m_ui->m_monitorTable->removeRow(selectedRow);
  m_propeties.Remove(m_propeties.GetMonitors()[selectedRow].GetName());
}

void BlotterMonitorsDialog::OnLoadDefault() {
  Commit();
  while(m_ui->m_monitorTable->rowCount() > 0) {
    m_ui->m_monitorTable->removeRow(0);
  }
  m_propeties =
    m_userProfile->GetBlotterSettings().GetDefaultBlotterTaskProperties();
  m_ui->m_monitorTable->setRowCount(m_propeties.GetMonitors().size());
  for(auto i = m_propeties.GetMonitors().begin();
      i != m_propeties.GetMonitors().end(); ++i) {
    auto monitorItem =
      new QTableWidgetItem(QString::fromStdString(i->GetName()));
    m_ui->m_monitorTable->setItem(
      std::distance(m_propeties.GetMonitors().begin(), i), 0, monitorItem);
  }
  if(m_ui->m_monitorTable->rowCount() > 0) {
    auto selectedItem = m_ui->m_monitorTable->item(0, 0);
    m_ui->m_monitorTable->setCurrentItem(selectedItem);
    selectedItem->setSelected(true);
  }
}

void BlotterMonitorsDialog::OnSaveAsDefault() {
  Commit();
  m_userProfile->GetBlotterSettings().SetDefaultBlotterTaskProperties(
    m_propeties);
}

void BlotterMonitorsDialog::OnResetDefault() {
  Commit();
  m_userProfile->GetBlotterSettings().SetDefaultBlotterTaskProperties(
    BlotterTaskProperties::GetDefault());
}

void BlotterMonitorsDialog::OnAccept() {
  Commit();
  Q_EMIT accept();
}

void BlotterMonitorsDialog::OnApply() {
  Commit();
  m_model->GetTasksModel().SetProperties(m_propeties);
}

void BlotterMonitorsDialog::OnApplyToAll() {
  Commit();
  for(auto& blotter : m_userProfile->GetBlotterSettings().GetAllBlotters()) {
    blotter->GetTasksModel().SetProperties(m_propeties);
  }
}
