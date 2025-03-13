#include "Spire/BookView/BookViewController.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;

BookViewController::BookViewController(Ref<UserProfile> user_profile)
  : m_window(nullptr) {}

BookViewController::~BookViewController() {
  close();
}

void BookViewController::open() {
  if(m_window) {
    return;
  }
  m_window = new BookViewWindow(Ref(*m_user_profile),
    m_user_profile->GetSecurityInfoQueryModel(),
    m_user_profile->GetKeyBindings(), m_user_profile->GetMarketDatabase(),
    m_user_profile->GetBookViewPropertiesWindowFactory(),
    [] (const auto&) { return std::shared_ptr<BookViewModel>(); });
}

void BookViewController::close() {
  if(!m_window) {
    return;
  }
}

connection BookViewController::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}
