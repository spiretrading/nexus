#include "Spire/AccountViewer/TraderItemWidget.hpp"
#include "Spire/AccountViewer/AccountInfoModel.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_TraderItemWidget.h"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

TraderItemWidget::TraderItemWidget(Ref<UserProfile> userProfile,
    std::shared_ptr<AccountInfoModel> infoModel,
    std::shared_ptr<AccountEntitlementModel> entitlementModel,
    std::shared_ptr<RiskModel> riskModel,
    std::shared_ptr<ComplianceModel> complianceModel,
    QWidget* parent, Qt::WindowFlags flags)
    : QWidget{parent, flags},
      m_ui{std::make_unique<Ui_TraderItemWidget>()},
      m_userProfile{userProfile.get()},
      m_infoModel{std::move(infoModel)},
      m_entitlementModel{std::move(entitlementModel)},
      m_riskModel{std::move(riskModel)},
      m_complianceModel{std::move(complianceModel)} {
  m_ui->setupUi(this);
  m_ui->m_traderProfileView->Initialize(Ref(*m_userProfile),
    !m_userProfile->IsAdministrator(), false, m_infoModel, m_entitlementModel,
    m_riskModel, m_complianceModel);
  connect(m_ui->m_saveButton, &QPushButton::clicked, this,
    &TraderItemWidget::OnApply);
  connect(m_ui->m_reloadButton, &QPushButton::clicked, this,
    &TraderItemWidget::OnReload);
  connect(m_ui->m_openBlotterButton, &QPushButton::clicked, this,
    &TraderItemWidget::OnOpenBlotter);
}

TraderItemWidget::~TraderItemWidget() {}

void TraderItemWidget::OnApply() {
  m_ui->m_traderProfileView->Commit();
}

void TraderItemWidget::OnReload() {
  m_ui->m_traderProfileView->Reload();
}

void TraderItemWidget::OnOpenBlotter() {
  auto& consolidatedBlotter =
    m_userProfile->GetBlotterSettings().GetConsolidatedBlotter(
    m_infoModel->GetAccount());
  auto& window = BlotterWindow::GetBlotterWindow(Ref(*m_userProfile),
    Ref(consolidatedBlotter));
  window.show();
}
