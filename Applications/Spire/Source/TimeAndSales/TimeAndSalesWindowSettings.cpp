#include "Spire/TimeAndSales/TimeAndSalesWindowSettings.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/SecurityView.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

TimeAndSalesWindowSettings::TimeAndSalesWindowSettings(
    const TimeAndSalesWindow& window)
    : m_security(window.GetDisplayedSecurity()),
      m_identifier(window.GetIdentifier()),
      m_link_identifier(window.m_link_identifier),
      m_geometry(window.saveGeometry()) {
  if(m_security == Security()) {
    m_name = "Time And Sales";
  } else {
    m_name = "Time And Sales - " + to_text(m_security).toStdString();
  }
}

std::string TimeAndSalesWindowSettings::GetName() const {
  return m_name;
}

QWidget* TimeAndSalesWindowSettings::Reopen(
    Ref<UserProfile> user_profile) const {
  auto window = new TimeAndSalesWindow(user_profile->GetSecurityQueryModel(),
    user_profile->GetTimeAndSalesPropertiesWindowFactory(),
    user_profile->GetTimeAndSalesModelBuilder(), m_identifier);
  window->setAttribute(Qt::WA_DeleteOnClose);
  Apply(Ref(user_profile), Store(*window));
  return window;
}

void TimeAndSalesWindowSettings::Apply(
    Ref<UserProfile> user_profile, Out<QWidget> widget) const {
  auto& window = dynamic_cast<TimeAndSalesWindow&>(*widget);
  restore_geometry(window, m_geometry);
  if(m_security != Security()) {
    window.m_security_view->get_current()->set(m_security);
  }
  window.m_link_identifier = m_link_identifier;
}
