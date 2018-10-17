#include "Spire/Dashboard/DashboardWidgetWindowSettings.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

DashboardWidgetWindowSettings::DashboardWidgetWindowSettings() {}

DashboardWidgetWindowSettings::DashboardWidgetWindowSettings(
    const DashboardWidget& widget)
    : m_sortOrder(widget.m_columnSortOrder),
      m_rendererSettings(widget.m_renderer->GetSettings()) {}

DashboardWidgetWindowSettings::~DashboardWidgetWindowSettings() {}

string DashboardWidgetWindowSettings::GetName() const {
  return "Dashboard";
}

QWidget* DashboardWidgetWindowSettings::Reopen(
    Ref<UserProfile> userProfile) const {
  assert(false);
  return nullptr;
}

void DashboardWidgetWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  auto& dashboard = dynamic_cast<DashboardWidget&>(*widget);
  dashboard.m_columnSortOrder = m_sortOrder;
  dashboard.m_renderer->Apply(m_rendererSettings);
}
