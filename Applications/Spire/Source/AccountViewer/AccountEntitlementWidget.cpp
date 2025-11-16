#include "Spire/AccountViewer/AccountEntitlementWidget.hpp"
#include "Spire/AccountViewer/AccountEntitlementModel.hpp"
#include "Spire/AccountViewer/EntitlementEntryWidget.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_AccountEntitlementWidget.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

AccountEntitlementWidget::AccountEntitlementWidget(QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_AccountEntitlementWidget>()) {
  m_ui->setupUi(this);
}

AccountEntitlementWidget::~AccountEntitlementWidget() {}

void AccountEntitlementWidget::Initialize(
    Ref<UserProfile> userProfile, bool isReadOnly) {
  m_userProfile = userProfile.get();
  m_isReadOnly = isReadOnly;
}

const AccountEntitlementModel& AccountEntitlementWidget::GetModel() const {
  return *m_model;
}

AccountEntitlementModel& AccountEntitlementWidget::GetModel() {
  return *m_model;
}

void AccountEntitlementWidget::SetModel(
    const std::shared_ptr<AccountEntitlementModel>& model) {
  QVBoxLayout* layout = static_cast<QVBoxLayout*>(
    m_ui->m_entitlementLayout->layout());
  while(layout->count() > 0) {
    QLayoutItem* item = layout->itemAt(0);
    layout->removeItem(item);
  }
  m_model = model;
  const EntitlementDatabase& entitlements =
    m_userProfile->GetEntitlementDatabase();
  auto entries = entitlements.get_entries();
  for(auto i = entries.begin(); i != entries.end(); ++i) {
    EntitlementEntryWidget* entitlementWidget = new EntitlementEntryWidget(
      Ref(*m_userProfile), m_isReadOnly, *i, m_model, this);
    layout->insertWidget(layout->count(), entitlementWidget);
    if(i + 1 != entries.end()) {
      QFrame* line = new QFrame();
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);
      layout->insertWidget(layout->count(), line);
    }
  }
}
