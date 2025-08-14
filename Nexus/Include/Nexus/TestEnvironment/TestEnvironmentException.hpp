#ifndef NEXUS_TEST_ENVIRONMENT_EXCEPTION_HPP
#define NEXUS_TEST_ENVIRONMENT_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>

namespace Nexus {

  /** Signals an invalid operation performed on a TestEnvironment. */
  class TestEnvironmentException :
      public std::runtime_error, public boost::exception {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a TestEnvironmentException. */
      TestEnvironmentException();
  };

  inline TestEnvironmentException::TestEnvironmentException()
    : TestEnvironmentException("Invalid operation performed.") {}
}

#endif
