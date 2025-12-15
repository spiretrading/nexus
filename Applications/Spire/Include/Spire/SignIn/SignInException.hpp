#ifndef SPIRE_SIGN_IN_EXCEPTION_HPP
#define SPIRE_SIGN_IN_EXCEPTION_HPP
#include <stdexcept>

namespace Spire {

  /** Stores an exception indicating a failure to sign in. */
  class SignInException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
  };
}

#endif
