#ifndef SPIRE_LOGIN_EXCEPTION_HPP
#define SPIRE_LOGIN_EXCEPTION_HPP
#include <stdexcept>
#include "Spire/Login/Login.hpp"

namespace Spire {

  /** Stores an exception indicating a failure to login. */
  class LoginException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
  };
}

#endif
