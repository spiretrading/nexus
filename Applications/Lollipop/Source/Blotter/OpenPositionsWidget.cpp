#include "Spire/Blotter/OpenPositionsWidget.hpp"
#include <QAbstractItemModel>
#include <QFileDialog>
#include <QMenu>
#include <QStandardPaths>
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterTasksModel.hpp"
#include "Spire/Blotter/OpenPositionsModel.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderTaskNodes.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "Spire/Utilities/ExportModel.hpp"
#include "ui_OpenPositionsWidget.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  void FlattenPosition(BlotterModel& blotterModel,
      const OpenPositionsModel::Entry& position,
      const UserProfile& userProfile) {
    auto orderFields = make_market_order_fields(
      blotterModel.GetExecutingAccount(), position.m_key.m_security,
      position.m_key.m_currency,
      get_opposite(get_side(position.m_inventory.m_position)),
      userProfile.GetDestinationDatabase().get_preferred_destination(
      position.m_key.m_security.get_venue()).m_id,
      abs(position.m_inventory.m_position.m_quantity));
    auto orderNode = MakeOrderTaskNodeFromOrderFields(orderFields,
      userProfile);
    auto& entry = blotterModel.GetTasksModel().Add(*orderNode);
    entry.m_task->Execute();
  }
}

OpenPositionsWidget::OpenPositionsWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_OpenPositionsWidget>()),
      m_model(nullptr) {
  m_ui->setupUi(this);
  m_ui->m_flattenSelectionButton->setEnabled(false);
  connect(m_ui->m_openPositionsTable, &QTableView::customContextMenuRequested,
    this, &OpenPositionsWidget::OnContextMenu);
  connect(m_ui->m_flattenAllButton, &QPushButton::clicked, this,
    &OpenPositionsWidget::OnFlattenAll);
  connect(m_ui->m_flattenSelectionButton, &QPushButton::clicked, this,
    &OpenPositionsWidget::OnFlattenSelection);
}

OpenPositionsWidget::UIState OpenPositionsWidget::GetUIState() const {
  UIState state;
  state.m_tableGeometry =
    m_ui->m_openPositionsTable->horizontalHeader()->saveGeometry();
  state.m_tableState =
    m_ui->m_openPositionsTable->horizontalHeader()->saveState();
  return state;
}

void OpenPositionsWidget::SetUIState(const UIState& state) {
  m_ui->m_openPositionsTable->horizontalHeader()->restoreGeometry(
    state.m_tableGeometry);
  m_ui->m_openPositionsTable->horizontalHeader()->restoreState(
    state.m_tableState);
}

void OpenPositionsWidget::SetModel(Ref<UserProfile> userProfile,
    Ref<BlotterModel> model) {
  if(m_proxyModel != nullptr) {
    disconnect(m_selectionChangedConnection);
  }
  m_ui->m_openPositionsTable->reset();
  m_userProfile = userProfile.get();
  m_model = model.get();
  m_proxyModel = std::make_unique<CustomVariantSortFilterProxyModel>(
    Ref(userProfile));
  m_proxyModel->setSourceModel(&m_model->GetOpenPositionsModel());
  m_ui->m_openPositionsTable->setModel(m_proxyModel.get());
  m_ui->m_openPositionsTable->setItemDelegate(new CustomVariantItemDelegate(
    Ref(userProfile)));
  m_ui->m_openPositionsTable->horizontalHeader()->setSectionsMovable(true);
  QFontMetrics metrics(m_ui->m_openPositionsTable->font());
  m_ui->m_openPositionsTable->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
  m_selectionChangedConnection = connect(
    m_ui->m_openPositionsTable->selectionModel(),
    &QItemSelectionModel::selectionChanged, this,
    &OpenPositionsWidget::OnSelectionChanged);
}

void OpenPositionsWidget::OnContextMenu(const QPoint& position) {
  QMenu contextMenu;
  QAction exportAction(&contextMenu);
  exportAction.setText(tr("Export To File"));
  contextMenu.addAction(&exportAction);
  QAction* selectedAction = contextMenu.exec(
    static_cast<QWidget*>(sender())->mapToGlobal(position));
  if(selectedAction == &exportAction) {
    QString path = QFileDialog::getSaveFileName(this,
      tr("Select file to export to."), QStandardPaths::writableLocation(
      QStandardPaths::DocumentsLocation) + "/open_positions.csv",
      "CSV (*.csv)");
    if(path.isNull()) {
      return;
    }
    ofstream exportFile(path.toStdString());
    ExportModelAsCsv(*m_userProfile, m_model->GetOpenPositionsModel(),
      exportFile);
  }
}

void OpenPositionsWidget::OnFlattenAll() {
  vector<OpenPositionsModel::Entry> openPositions =
    m_model->GetOpenPositionsModel().GetOpenPositions();
  for(auto i = openPositions.begin(); i != openPositions.end(); ++i) {
    FlattenPosition(*m_model, *i, *m_userProfile);
  }
}

void OpenPositionsWidget::OnFlattenSelection() {
  QModelIndexList openPositionIndexes =
    m_ui->m_openPositionsTable->selectionModel()->selectedRows();
  vector<OpenPositionsModel::Entry> openPositions =
    m_model->GetOpenPositionsModel().GetOpenPositions();
  for(auto i = openPositionIndexes.begin();
      i != openPositionIndexes.end(); ++i) {
    int index = m_proxyModel->mapToSource(*i).row();
    OpenPositionsModel::Entry position = openPositions[index];
    FlattenPosition(*m_model, position, *m_userProfile);
  }
}

void OpenPositionsWidget::OnSelectionChanged(const QItemSelection& selected,
    const QItemSelection& deselected) {
  m_ui->m_flattenSelectionButton->setEnabled(!selected.isEmpty());
}
