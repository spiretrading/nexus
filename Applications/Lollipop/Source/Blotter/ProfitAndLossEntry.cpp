#include "Spire/Blotter/ProfitAndLossEntry.hpp"
#include <QBoxLayout>
#include <QStandardItemModel>
#include <QTableView>
#include "Spire/Blotter/ProfitAndLossEntryModel.hpp"
#include "Spire/Blotter/ProfitAndLossHeader.hpp"
#include "Spire/Blotter/ProfitAndLossTable.hpp"
#include "Spire/UI/ExpandButton.hpp"
#include "ui_ProfitAndLossHeader.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

ProfitAndLossEntry::ProfitAndLossEntry(Ref<UserProfile> userProfile,
    Ref<ProfitAndLossEntryModel> model, QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_model(model.Get()) {
  m_header = new ProfitAndLossHeader(Ref(*m_model), this);
  m_table = new ProfitAndLossTable(Ref(userProfile), Ref(*m_model), this);
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(6);
  setLayout(layout);
  layout->addWidget(m_header, 0);
  layout->addStretch(0);
  layout->addWidget(m_table, 1);
  m_table->setVisible(false);
  m_expandedConnection = m_header->m_ui->m_expandButton->ConnectExpandedSignal(
    std::bind(&ProfitAndLossEntry::OnTableExpanded, this));
  m_collapsedConnection =
    m_header->m_ui->m_expandButton->ConnectCollapsedSignal(
    std::bind(&ProfitAndLossEntry::OnTableCollapsed, this));
}

ProfitAndLossEntry::~ProfitAndLossEntry() {}

const ProfitAndLossEntryModel& ProfitAndLossEntry::GetModel() const {
  return *m_model;
}

ProfitAndLossEntryModel& ProfitAndLossEntry::GetModel() {
  return *m_model;
}

void ProfitAndLossEntry::OnTableExpanded() {
  m_table->show();
}

void ProfitAndLossEntry::OnTableCollapsed() {
  m_table->hide();
}
