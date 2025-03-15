#include "Spire/BookView/BookViewWindowSettings.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

BookViewWindowSettings::BookViewWindowSettings(const BookViewWindow& window)
    : m_identifier(window.GetIdentifier()),
      m_link_identifier(window.m_link_identifier) {
  auto& security = window.GetDisplayedSecurity();
  if(security == Security()) {
    m_name = "Book View";
  } else {
    m_name = "Book View - " + to_text(security).toStdString();
  }
}

std::string BookViewWindowSettings::GetName() const {
  return m_name;
}

QWidget* BookViewWindowSettings::Reopen(Ref<UserProfile> user_profile) const {
  auto window = new BookViewWindow(Ref(user_profile),
    user_profile->GetSecurityInfoQueryModel(),
    user_profile->GetKeyBindings(),
    user_profile->GetMarketDatabase(),
    user_profile->GetBookViewPropertiesWindowFactory(),
    user_profile->GetBookViewModelBuilder(), m_identifier);
  Apply(Ref(user_profile), Store(*window));
  return window;
}

void BookViewWindowSettings::Apply(
    Ref<UserProfile> user_profile, Out<QWidget> widget) const {
  auto& window = dynamic_cast<BookViewWindow&>(*widget);
  restore_geometry(window, m_geometry);
  window.m_link_identifier = m_link_identifier;
  window.m_security_view->restore(m_security_view);
}
