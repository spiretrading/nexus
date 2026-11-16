#include "Spire/Toolbar/ToolbarWindowSettings.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Dimensions.hpp"

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
    profile->GetClients().get_service_locator_client().get_account();
  auto roles =
    profile->GetClients().get_administration_client().load_account_roles(
      account);
  auto window = new ToolbarWindow(account, roles,
    profile->GetAccountQueryModel(), profile->GetRecentlyClosedWindows());
  Apply(Ref(profile), out(*window));
  return window;
}

void ToolbarWindowSettings::Apply(
    Ref<UserProfile> userProfile, Out<QWidget> widget) const {
  auto& window = dynamic_cast<ToolbarWindow&>(*widget);
  window.move(m_position);
}
