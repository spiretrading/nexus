#include "Spire/BookView/LegacyBookViewWindowSettings.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Ui/SecurityView.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

namespace {
  auto get_frame_height() {
    return GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) +
      GetSystemMetrics(SM_CXPADDEDBORDER);
  }
}

std::string LegacyBookViewWindowSettings::GetName() const {
  return m_name;
}

QWidget* LegacyBookViewWindowSettings::Reopen(
    Ref<UserProfile> user_profile) const {
  auto window = new BookViewWindow(Ref(user_profile),
    user_profile->GetSecurityInfoQueryModel(),
    user_profile->GetKeyBindings(),
    user_profile->GetBookViewPropertiesWindowFactory(),
    user_profile->GetBookViewModelBuilder(), m_identifier);
  Apply(Ref(user_profile), out(*window));
  return window;
}

void LegacyBookViewWindowSettings::Apply(
    Ref<UserProfile> userProfile, Out<QWidget> widget) const {
  auto& window = dynamic_cast<BookViewWindow&>(*widget);
  restore_geometry(window, m_geometry);
  auto frame_height = get_frame_height();
  window.move(window.x(), window.y() - frame_height);
  window.resize(window.width(), window.height() + frame_height);
  if(m_security) {
    window.m_security_view->get_current()->set(m_security);
  }
  window.m_link_identifier = m_link_identifier;
}
