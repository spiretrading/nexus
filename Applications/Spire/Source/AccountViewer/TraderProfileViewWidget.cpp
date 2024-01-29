#include "Spire/AccountViewer/TraderProfileViewWidget.hpp"
#include <QMessageBox>
#include "Spire/AccountViewer/AccountEntitlementModel.hpp"
#include "Spire/AccountViewer/AccountInfoModel.hpp"
#include "Spire/AccountViewer/ComplianceModel.hpp"
#include "Spire/AccountViewer/RiskModel.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_TraderProfileViewWidget.h"

using namespace Beam;
using namespace Spire;

TraderProfileViewWidget::TraderProfileViewWidget(QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_TraderProfileViewWidget>()) {
  m_ui->setupUi(this);
}

TraderProfileViewWidget::~TraderProfileViewWidget() {}

void TraderProfileViewWidget::Initialize(Ref<UserProfile> userProfile,
    bool isReadOnly, bool isPasswordReadOnly,
    std::shared_ptr<AccountInfoModel> accountInfoModel,
    std::shared_ptr<AccountEntitlementModel> accountEntitlementModel,
    std::shared_ptr<RiskModel> riskModel,
    std::shared_ptr<ComplianceModel> complianceModel) {
  m_ui->m_accountInfoTab->Initialize(Ref(userProfile), isReadOnly,
    isPasswordReadOnly);
  m_ui->m_accountInfoTab->SetModel(accountInfoModel);
  m_ui->m_entitlementsTab->Initialize(Ref(userProfile), isReadOnly);
  m_ui->m_entitlementsTab->SetModel(accountEntitlementModel);
  m_ui->m_riskTab->Initialize(Ref(userProfile), isReadOnly);
  m_ui->m_riskTab->SetModel(riskModel);
  m_ui->m_complianceTab->Initialize(Ref(userProfile), isReadOnly);
  m_ui->m_complianceTab->SetModel(complianceModel);
  m_ui->m_activityReportTab->Initialize(Ref(userProfile),
    accountInfoModel->GetAccount());
}

void TraderProfileViewWidget::Commit() {
  try {
    m_ui->m_accountInfoTab->Commit();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to save account info: ") + e.what());
  }
  try {
    m_ui->m_entitlementsTab->GetModel().Commit();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to save entitlements: ") + e.what());
  }
  try {
    m_ui->m_riskTab->Commit();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to save risk parameters: ") + e.what());
  }
  try {
    m_ui->m_complianceTab->Commit();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to save compliance parameters: ") + e.what());
  }
}

void TraderProfileViewWidget::Reload() {
  try {
    m_ui->m_accountInfoTab->GetModel().Load();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to load account info: ") + e.what());
  }
  try {
    m_ui->m_entitlementsTab->GetModel().Load();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to load the account's entitlements: ") + e.what());
  }
  try {
    m_ui->m_riskTab->GetModel().Load();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to load the risk parameters: ") + e.what());
  }
  try {
    m_ui->m_complianceTab->GetModel().Load();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to load the compliance parameters: ") + e.what());
  }
}
