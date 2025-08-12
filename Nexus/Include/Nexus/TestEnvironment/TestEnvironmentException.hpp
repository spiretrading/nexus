#ifndef NEXUS_TEST_ENVIRONMENT_EXCEPTION_HPP
#define NEXUS_TEST_ENVIRONMENT_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>

namespace Nexus {

  /** Signals an invalid operation performed on a TestEnvironment. */
  class TestEnvironmentException :
      public std::runtime_error, public boost::exception {
    public:

      /** Constructs a TestEnvironmentException. */
      TestEnvironmentException();

      /**
       * Constructs a TestEnvironmentException.
       * @param message A message describing the error.
       */
      TestEnvironmentException(const std::string& message);
  };

  inline TestEnvironmentException::TestEnvironmentException()
    : TestEnvironmentException("Invalid operation performed.") {}

  inline TestEnvironmentException::TestEnvironmentException(
    const std::string& message)
    : std::runtime_error(message) {}
}

#endif
