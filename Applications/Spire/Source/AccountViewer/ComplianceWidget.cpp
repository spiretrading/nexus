#include "Spire/AccountViewer/ComplianceWidget.hpp"
#include <QBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include "Spire/AccountViewer/ComplianceModel.hpp"
#include "Spire/AccountViewer/ComplianceRuleEntryWidget.hpp"
#include "ui_ComplianceWidget.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

ComplianceWidget::ComplianceWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget{parent, flags},
      m_ui{std::make_unique<Ui_ComplianceWidget>()},
      m_isReadOnly{true} {
  m_ui->setupUi(this);
  auto layout = static_cast<QBoxLayout*>(m_ui->m_scrollLayout->layout());
  layout->setStretch(0, 1);
  connect(m_ui->m_newRuleComboBox,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this,
    &ComplianceWidget::OnNewRuleActivated);
  connect(m_ui->m_deleteRuleButton, &QToolButton::pressed, this,
    &ComplianceWidget::OnDeleteRules);
}

ComplianceWidget::~ComplianceWidget() {}

void ComplianceWidget::Initialize(Ref<UserProfile> userProfile,
    bool isReadOnly) {
  m_isReadOnly = isReadOnly;
  m_userProfile = userProfile.get();
  if(m_isReadOnly) {
    m_ui->m_toolbarWidget->hide();
  }
}

const ComplianceModel& ComplianceWidget::GetModel() const {
  return *m_model;
}

ComplianceModel& ComplianceWidget::GetModel() {
  return *m_model;
}

void ComplianceWidget::SetModel(const std::shared_ptr<ComplianceModel>& model) {
  m_model = model;
  m_ui->m_newRuleComboBox->clear();
  auto layout = static_cast<QBoxLayout*>(m_ui->m_scrollLayout->layout());
  while(layout->count() > 1) {
    auto item = layout->takeAt(0);
    delete item;
  }
  if(!m_isReadOnly) {
    m_ui->m_newRuleComboBox->setEditable(true);
    m_ui->m_newRuleComboBox->lineEdit()->setReadOnly(true);
    auto font = m_ui->m_newRuleComboBox->lineEdit()->font();
    font.setItalic(true);
    m_ui->m_newRuleComboBox->lineEdit()->setFont(font);
    for(auto& schema : m_model->GetSchemas()) {
      auto name = GetUnwrappedName(schema);
      m_ui->m_newRuleComboBox->addItem(name);
    }
    m_ui->m_newRuleComboBox->lineEdit()->setText(tr("Assign New Rule"));
  }
  for(auto& entry : m_model->GetEntries()) {
    OnEntryAdded(entry);
  }
  m_entryAddedConnection = m_model->ConnectComplianceRuleEntryAddedSignal(
    std::bind(&ComplianceWidget::OnEntryAdded, this, std::placeholders::_1));
  m_entryRemovedConnection = m_model->ConnectComplianceRuleEntryRemovedSignal(
    std::bind(&ComplianceWidget::OnEntryRemoved, this, std::placeholders::_1));
}

void ComplianceWidget::Commit() {
  auto layout = static_cast<QBoxLayout*>(m_ui->m_scrollLayout->layout());
  for(auto i = 0; i < layout->count() - 1; ++i) {
    auto entryWidget = static_cast<ComplianceRuleEntryWidget*>(
      layout->itemAt(i)->widget());
    entryWidget->Commit();
  }
  try {
    m_model->Commit();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to save compliance rules: ") + e.what());
  }
}

void ComplianceWidget::OnNewRuleActivated(int index) {
  m_ui->m_newRuleComboBox->lineEdit()->setText(tr("Assign New Rule"));
  auto& schema = m_model->GetSchemas()[index];
  m_model->Add(schema);
}

void ComplianceWidget::OnEntryAdded(const ComplianceRuleEntry& entry) {
  auto entryWidget = new ComplianceRuleEntryWidget{Ref(*m_userProfile),
    m_isReadOnly, entry, m_model, this};
  auto layout = static_cast<QBoxLayout*>(m_ui->m_scrollLayout->layout());
  layout->insertWidget(layout->count() - 1, entryWidget);
}

void ComplianceWidget::OnEntryRemoved(const ComplianceRuleEntry& entry) {
  auto layout = static_cast<QBoxLayout*>(m_ui->m_scrollLayout->layout());
  auto i = 0;
  while(i < layout->count() - 1) {
    auto entryWidget = static_cast<ComplianceRuleEntryWidget*>(
      layout->itemAt(i)->widget());
    if(entryWidget->GetEntry().get_id() == entry.get_id()) {
      layout->takeAt(i);
      delete entryWidget;
    } else {
      ++i;
    }
  }
}

void ComplianceWidget::OnDeleteRules() {
  auto layout = static_cast<QBoxLayout*>(m_ui->m_scrollLayout->layout());
  for(auto i = 0; i < layout->count() - 1; ++i) {
    auto entryWidget = static_cast<ComplianceRuleEntryWidget*>(
      layout->itemAt(i)->widget());
    if(entryWidget->IsSelected()) {
      m_model->Remove(entryWidget->GetEntry().get_id());
    }
  }
}
