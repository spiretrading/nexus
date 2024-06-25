#include "Spire/Dashboard/DashboardWindowSettings.hpp"
#include "Spire/Dashboard/DashboardWindow.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_DashboardWindow.h"

using namespace Beam;
using namespace Spire;
using namespace std;

DashboardWindowSettings::DashboardWindowSettings()
    : m_schema{DashboardModelSchema::GetDefaultSchema()} {}

DashboardWindowSettings::DashboardWindowSettings(const DashboardWindow& window)
    : m_name(window.m_name),
      m_schema{*window.m_model, window.m_ui->m_dashboard->GetRowBuilder()},
      m_dashboardWidgetSettings{window.m_ui->m_dashboard->GetWindowSettings()},
      m_geometry{window.saveGeometry()} {}

DashboardWindowSettings::~DashboardWindowSettings() {}

string DashboardWindowSettings::GetName() const {
  if(m_name.empty()) {
    return "Dashboard";
  }
  return "Dashboard - " + m_name;
}

QWidget* DashboardWindowSettings::Reopen(
    Ref<UserProfile> userProfile) const {
  auto window = new DashboardWindow(m_name, m_schema, Ref(userProfile));
  window->setAttribute(Qt::WA_DeleteOnClose);
  Apply(Ref(userProfile), Store(*window));
  return window;
}

void DashboardWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  auto& window = dynamic_cast<DashboardWindow&>(*widget);
  restore_geometry(window, m_geometry);
  m_dashboardWidgetSettings->Apply(Ref(userProfile),
    Store(*window.m_ui->m_dashboard));
}
