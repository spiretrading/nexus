#include "Spire/BookView/BookViewController.hpp"

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
