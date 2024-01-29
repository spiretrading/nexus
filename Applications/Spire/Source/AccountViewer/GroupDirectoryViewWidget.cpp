#include "Spire/AccountViewer/GroupDirectoryViewWidget.hpp"
#include <QMessageBox>
#include "Spire/AccountViewer/ComplianceModel.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_GroupDirectoryViewWidget.h"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Spire;

GroupDirectoryViewWidget::GroupDirectoryViewWidget(QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget{parent, flags},
      m_ui{std::make_unique<Ui_GroupDirectoryViewWidget>()} {
  m_ui->setupUi(this);
}

GroupDirectoryViewWidget::~GroupDirectoryViewWidget() {}

void GroupDirectoryViewWidget::Initialize(Ref<UserProfile> userProfile,
    bool isReadOnly, const DirectoryEntry& directoryEntry,
    std::shared_ptr<ComplianceModel> complianceModel) {
  m_ui->m_complianceTab->Initialize(Ref(userProfile), isReadOnly);
  m_ui->m_complianceTab->SetModel(complianceModel);
  m_ui->m_groupProfitAndLossReportTab->Initialize(Ref(userProfile),
    directoryEntry);
}

void GroupDirectoryViewWidget::RemoveActivityReportTab() {
  m_ui->m_tabs->removeTab(1);
}

void GroupDirectoryViewWidget::Commit() {
  try {
    m_ui->m_complianceTab->Commit();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to save compliance parameters: ") + e.what());
  }
}

void GroupDirectoryViewWidget::Reload() {
  try {
    m_ui->m_complianceTab->GetModel().Load();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to load the compliance parameters: ") + e.what());
  }
}
