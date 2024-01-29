#include "Spire/AccountViewer/ServiceItemWidget.hpp"
#include <QMessageBox>
#include "Spire/AccountViewer/AccountEntitlementModel.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_ServiceItemWidget.h"

using namespace Beam;
using namespace Spire;
using namespace std;

ServiceItemWidget::ServiceItemWidget(Ref<UserProfile> userProfile,
    const std::shared_ptr<AccountEntitlementModel>& entitlementModel,
    QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_ServiceItemWidget>()),
      m_entitlementModel(entitlementModel) {
  m_ui->setupUi(this);
  m_ui->m_entitlementsTab->Initialize(Ref(userProfile), false);
  m_ui->m_entitlementsTab->SetModel(m_entitlementModel);
  connect(m_ui->m_applyButton, &QPushButton::clicked, this,
    &ServiceItemWidget::OnApply);
  connect(m_ui->m_revertButton, &QPushButton::clicked, this,
    &ServiceItemWidget::OnRevert);
}

ServiceItemWidget::~ServiceItemWidget() {}

void ServiceItemWidget::OnApply() {
  try {
    m_entitlementModel->Commit();
  } catch(std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to save entitlements."));
  }
}

void ServiceItemWidget::OnRevert() {
  try {
    m_entitlementModel->Load();
  } catch(std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to load the service's entitlements."));
  }
}
