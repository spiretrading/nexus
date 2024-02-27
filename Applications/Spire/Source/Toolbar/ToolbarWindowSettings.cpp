#include "Spire/Toolbar/ToolbarWindowSettings.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Toolbar/ToolbarWindow.hpp"

using namespace Beam;
using namespace Spire;

ToolbarWindowSettings::ToolbarWindowSettings()
  : m_position(0, 0) {}

ToolbarWindowSettings::ToolbarWindowSettings(const ToolbarWindow& toolbar)
  : m_position(toolbar.pos()) {}

std::string ToolbarWindowSettings::GetName() const {
  return "Toolbar";
}

QWidget* ToolbarWindowSettings::Reopen(Ref<UserProfile> profile) const {
  auto account =
    profile->GetServiceClients().GetServiceLocatorClient().GetAccount();
  auto roles = profile->
    GetServiceClients().GetAdministrationClient().LoadAccountRoles(account);
  auto pinned_blotters = std::make_shared<ArrayListModel<BlotterModel*>>();
  for(auto& blotter : profile->GetBlotterSettings().GetAllBlotters()) {
    pinned_blotters->push(&*blotter);
  }
  auto window = new ToolbarWindow(account, roles,
    profile->GetRecentlyClosedWindows(), pinned_blotters);
  Apply(Ref(profile), Store(*window));
  return window;
}

void ToolbarWindowSettings::Apply(
    Ref<UserProfile> userProfile, Out<QWidget> widget) const {
  auto& window = dynamic_cast<ToolbarWindow&>(*widget);
  window.move(m_position);
}
