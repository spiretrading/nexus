#include "Spire/Blotter/ActivityLogWidget.hpp"
#include <QFileDialog>
#include <QMenu>
#include <QStandardPaths>
#include "Spire/Blotter/ActivityLogModel.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Utilities/ExportModel.hpp"
#include "ui_ActivityLogWidget.h"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

ActivityLogWidget::ActivityLogWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_ActivityLogWidget>()),
      m_userProfile(nullptr),
      m_model(nullptr) {
  m_ui->setupUi(this);
  connect(m_ui->m_activityLogTable, &QTableView::customContextMenuRequested,
    this, &ActivityLogWidget::OnContextMenu);
}

ActivityLogWidget::~ActivityLogWidget() {}

ActivityLogWidget::UIState ActivityLogWidget::GetUIState() const {
  UIState state;
  state.m_tableGeometry =
    m_ui->m_activityLogTable->horizontalHeader()->saveGeometry();
  state.m_tableState =
    m_ui->m_activityLogTable->horizontalHeader()->saveState();
  return state;
}

void ActivityLogWidget::SetUIState(const UIState& state) {
  restore_geometry(
    *m_ui->m_activityLogTable->horizontalHeader(), state.m_tableGeometry);
  m_ui->m_activityLogTable->horizontalHeader()->restoreState(
    state.m_tableState);
}

void ActivityLogWidget::SetModel(Ref<UserProfile> userProfile,
    Ref<BlotterModel> model) {
  m_userProfile = userProfile.Get();
  m_ui->m_activityLogTable->reset();
  m_model = model.Get();
  CustomVariantSortFilterProxyModel* proxyModel =
    new CustomVariantSortFilterProxyModel(Ref(*m_userProfile));
  proxyModel->setSourceModel(&m_model->GetActivityLogModel());
  int timeColumn = proxyModel->mapFromSource(
    m_model->GetActivityLogModel().index(
    0, ActivityLogModel::TIME_COLUMN)).column();
  proxyModel->sort(timeColumn, Qt::DescendingOrder);
  m_ui->m_activityLogTable->setModel(proxyModel);
  m_ui->m_activityLogTable->setItemDelegate(new CustomVariantItemDelegate(
    Ref(*m_userProfile)));
  m_ui->m_activityLogTable->horizontalHeader()->setSectionsMovable(true);
  QFontMetrics metrics(m_ui->m_activityLogTable->font());
  m_ui->m_activityLogTable->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
}

void ActivityLogWidget::OnContextMenu(const QPoint& position) {
  QMenu contextMenu;
  QAction exportAction(&contextMenu);
  exportAction.setText(tr("Export To File"));
  contextMenu.addAction(&exportAction);
  QAction* selectedAction = contextMenu.exec(
    static_cast<QWidget*>(sender())->mapToGlobal(position));
  if(selectedAction == &exportAction) {
    QString path = QFileDialog::getSaveFileName(this,
      tr("Select file to export to."), QStandardPaths::writableLocation(
      QStandardPaths::DocumentsLocation) + "/activity_log.csv", "CSV (*.csv)");
    if(path.isNull()) {
      return;
    }
    ofstream exportFile(path.toStdString());
    ExportModelAsCsv(*m_userProfile, m_model->GetActivityLogModel(),
      exportFile);
  }
}
