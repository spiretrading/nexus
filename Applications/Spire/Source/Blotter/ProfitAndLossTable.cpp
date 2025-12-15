#include "Spire/Blotter/ProfitAndLossTable.hpp"
#include "Spire/Blotter/ProfitAndLossEntryModel.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "ui_ProfitAndLossTable.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

ProfitAndLossTable::ProfitAndLossTable(Ref<UserProfile> userProfile,
    Ref<ProfitAndLossEntryModel> model, QWidget* parent)
    : QWidget(parent),
      m_ui(std::make_unique<Ui_ProfitAndLossTable>()),
      m_model(model.get()),
      m_proxyModel(new CustomVariantSortFilterProxyModel(
        Ref(userProfile))) {
  m_ui->setupUi(this);
  m_proxyModel->setSourceModel(m_model);
  m_ui->m_tableView->setModel(m_proxyModel);
  m_ui->m_tableView->setItemDelegate(new CustomVariantItemDelegate(
    Ref(userProfile)));
  m_ui->m_tableView->horizontalHeader()->setFont(QFont("Arial", 9));
  m_ui->m_tableView->setStyleSheet("\
    QHeaderView::section {\
      background-color: rgb(235, 234, 219);\
      border: 1px solid #6c6c6c;\
    }");
  OnHeaderResized(0, 0, 0);
  connect(m_ui->m_tableView->horizontalHeader(), &QHeaderView::sectionResized,
    this, &ProfitAndLossTable::OnHeaderResized);
  connect(m_proxyModel, &CustomVariantSortFilterProxyModel::rowsRemoved, this,
    &ProfitAndLossTable::OnRowCountChanged);
  connect(m_proxyModel, &CustomVariantSortFilterProxyModel::rowsInserted, this,
    &ProfitAndLossTable::OnRowCountChanged);
  QFontMetrics metrics(m_ui->m_tableView->font());
  m_ui->m_tableView->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
}

ProfitAndLossTable::~ProfitAndLossTable() {}

void ProfitAndLossTable::OnHeaderResized(int logicalIndex, int oldSize,
    int newSize) {
  m_ui->m_tableView->setFixedWidth(
    m_ui->m_tableView->horizontalHeader()->length());
}

void ProfitAndLossTable::OnRowCountChanged(const QModelIndex& parent, int start,
    int end) {
  m_ui->m_tableView->setMaximumHeight(
    m_ui->m_tableView->verticalHeader()->defaultSectionSize() *
    m_proxyModel->rowCount(QModelIndex()) +
    m_ui->m_tableView->horizontalHeader()->height());
}
