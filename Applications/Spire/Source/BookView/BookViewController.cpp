#include "Spire/BookView/BookViewController.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/BookView/ServiceBookViewModel.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;

BookViewController::BookViewController(Ref<UserProfile> user_profile)
  : m_user_profile(user_profile.Get()),
    m_window(nullptr) {}

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
    [=] (const auto& security) {
      return std::make_shared<ServiceBookViewModel>(security,
        m_user_profile->GetMarketDatabase(),
        m_user_profile->GetServiceClients().GetMarketDataClient());
    });
  m_window->show();
}

void BookViewController::close() {
  if(!m_window) {
    return;
  }
  m_window->close();
  m_window->deleteLater();
  m_window = nullptr;
  m_closed_signal();
}

connection BookViewController::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}
