#include "Spire/Login/LoginException.hpp"

using namespace Spire;

LoginException::LoginException(LoginWindow::State state)
  : std::runtime_error("Failed to login."),
    m_state(state) {}

LoginWindow::State LoginException::get_state() const {
  return m_state;
}
