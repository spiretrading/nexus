#include "Spire/AccountViewer/TraderProfileWindow.hpp"
#include <QMessageBox>
#include "Spire/AccountViewer/AccountEntitlementModel.hpp"
#include "Spire/AccountViewer/AccountInfoModel.hpp"
#include "Spire/AccountViewer/ComplianceModel.hpp"
#include "Spire/AccountViewer/RiskModel.hpp"
#include "Spire/AccountViewer/TraderProfileViewWidget.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_TraderProfileWindow.h"

using namespace Beam;
using namespace Spire;

TraderProfileWindow::TraderProfileWindow(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : QFrame{parent, flags},
      m_ui{std::make_unique<Ui_TraderProfileWindow>()},
      m_userProfile{userProfile.get()} {
  m_ui->setupUi(this);
}

TraderProfileWindow::~TraderProfileWindow() {}

void TraderProfileWindow::Load(const DirectoryEntry& account) {
  m_infoModel = std::make_shared<AccountInfoModel>(Ref(*m_userProfile),
    account);
  try {
    m_infoModel->Load();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to load the account info: ") + e.what());
  }
  m_entitlementModel = std::make_shared<AccountEntitlementModel>(
    Ref(*m_userProfile), account);
  try {
    m_entitlementModel->Load();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to load the account's entitlements: ") + e.what());
  }
  m_riskModel = std::make_shared<RiskModel>(Ref(*m_userProfile), account);
  try {
    m_riskModel->Load();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to load the risk parameters: ") + e.what());
  }
  m_complianceModel = std::make_shared<ComplianceModel>(Ref(*m_userProfile),
    account);
  try {
    m_complianceModel->Load();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to load the compliance rules: ") + e.what());
  }
  m_ui->m_profileView->Initialize(Ref(*m_userProfile), true, false, m_infoModel,
    m_entitlementModel, m_riskModel, m_complianceModel);
  setWindowTitle("Profile (" + QString::fromStdString(account.m_name) +
    ") - Read only");
}
