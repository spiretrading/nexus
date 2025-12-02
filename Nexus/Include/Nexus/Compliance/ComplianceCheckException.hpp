#ifndef NEXUS_COMPLIANCE_CHECK_EXCEPTION_HPP
#define NEXUS_COMPLIANCE_CHECK_EXCEPTION_HPP
#include <stdexcept>

namespace Nexus {

  /** Exception to indicate that an operation failed a compliance check. */
  class ComplianceCheckException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a ComplianceCheckException. */
      ComplianceCheckException();
  };

  inline ComplianceCheckException::ComplianceCheckException()
    : ComplianceCheckException("Compliance check failed.") {}
}

#endif
