#include "Spire/AccountViewer/GroupDirectoryItemWidget.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_GroupDirectoryItemWidget.h"

using namespace Beam;
using namespace Spire;

GroupDirectoryItemWidget::GroupDirectoryItemWidget(
    Ref<UserProfile> userProfile, const DirectoryEntry& directoryEntry,
    std::shared_ptr<ComplianceModel> complianceModel, QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget{parent, flags},
      m_ui{std::make_unique<Ui_GroupDirectoryItemWidget>()} {
  m_ui->setupUi(this);
  m_ui->m_groupDirectoryView->Initialize(Ref(userProfile),
    !userProfile->IsAdministrator(), directoryEntry, complianceModel);
  connect(m_ui->m_saveButton, &QPushButton::clicked, this,
    &GroupDirectoryItemWidget::OnApply);
  connect(m_ui->m_reloadButton, &QPushButton::clicked, this,
    &GroupDirectoryItemWidget::OnReload);
}

GroupDirectoryItemWidget::~GroupDirectoryItemWidget() {}

void GroupDirectoryItemWidget::RemoveActivityReportTab() {
  m_ui->m_groupDirectoryView->RemoveActivityReportTab();
}

void GroupDirectoryItemWidget::OnApply() {
  m_ui->m_groupDirectoryView->Commit();
}

void GroupDirectoryItemWidget::OnReload() {
  m_ui->m_groupDirectoryView->Reload();
}
