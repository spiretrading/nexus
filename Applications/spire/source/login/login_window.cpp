#include "spire/login/login_window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

login_window::login_window(QWidget* parent)
    : QWidget(parent) {}

void login_window::set_state(state state) {
}

connection login_window::connect_login_signal(
    const login_signal::slot_type& slot) const {
  return m_login_signal.connect(slot);
}

connection login_window::connect_cancel_signal(
    const cancel_signal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}
