#include "spire/toolbar/toolbar_window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

connection toolbar_window::connect_closed_signal(
    const closed_signal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

connection toolbar_window::connect_reopen_signal(
    const reopen_signal::slot_type& slot) const {
  return m_reopen_signal.connect(slot);
}

void toolbar_window::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}
