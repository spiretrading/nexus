#include "Spire/BookView/BookViewWindowSettings.hpp"
#include <Beam/Utilities/ToString.hpp>
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

BookViewWindowSettings::BookViewWindowSettings(const BookViewWindow& window)
    : m_ticker_view(window.m_ticker_view->save_state()),
      m_geometry(window.saveGeometry()) {
  auto& ticker = window.get_current()->get();
  if(ticker) {
    m_name = "Book View - " + to_string(ticker);
  } else {
    m_name = "Book View";
  }
}

std::string BookViewWindowSettings::GetName() const {
  return m_name;
}

QWidget* BookViewWindowSettings::Reopen(Ref<UserProfile> user_profile) const {
  auto window = new BookViewWindow(Ref(user_profile),
    user_profile->GetTickerInfoQueryModel(),
    user_profile->GetKeyBindings(),
    user_profile->GetBookViewPropertiesWindowFactory(),
    user_profile->GetBookViewModelBuilder(), user_profile->MakePropertyHub());
  Apply(Ref(user_profile), out(*window));
  return window;
}

void BookViewWindowSettings::Apply(
    Ref<UserProfile> user_profile, Out<QWidget> widget) const {
  auto& window = dynamic_cast<BookViewWindow&>(*widget);
  restore_geometry(window, m_geometry);
  window.m_ticker_view->restore(m_ticker_view);
}
