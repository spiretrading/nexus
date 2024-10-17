#include "Spire/TimeAndSales/LegacyTimeAndSalesWindowSettings.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
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

std::string LegacyTimeAndSalesWindowSettings::GetName() const {
  return m_name;
}

QWidget* LegacyTimeAndSalesWindowSettings::Reopen(
    Ref<UserProfile> user_profile) const {
  auto window = new TimeAndSalesWindow(user_profile->GetSecurityQueryModel(),
    user_profile->GetTimeAndSalesPropertiesWindowFactory(),
    user_profile->GetTimeAndSalesModelBuilder(), m_identifier);
  window->setAttribute(Qt::WA_DeleteOnClose);
  Apply(Ref(user_profile), Store(*window));
  return window;
}

void LegacyTimeAndSalesWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  auto& window = dynamic_cast<TimeAndSalesWindow&>(*widget);
  restore_geometry(window, m_geometry);
  auto frame_height = get_frame_height();
  window.move(window.x(), window.y() - frame_height);
  window.resize(window.width(), window.height() + frame_height);
  if(m_security != Security()) {
    window.m_security_view->get_current()->set(m_security);
  }
  window.m_link_identifier = m_link_identifier;
}
