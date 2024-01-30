#ifndef SPIRE_LOGIN_EXCEPTION_HPP
#define SPIRE_LOGIN_EXCEPTION_HPP
#include <stdexcept>
#include "Spire/Login/Login.hpp"
#include "Spire/Login/LoginWindow.hpp"

namespace Spire {

  /** Stores an exception indicating a failure to login. */
  class LoginException : public std::runtime_error {
    public:

      /**
       * Constructs a LoginException.
       * @param state The state of the login.
       */
      explicit LoginException(LoginWindow::State state);

      LoginWindow::State get_state() const;

    private:
      LoginWindow::State m_state;
  };
}

#endif
