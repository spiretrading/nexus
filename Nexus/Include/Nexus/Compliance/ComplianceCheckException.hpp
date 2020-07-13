#ifndef NEXUS_COMPLIANCE_CHECK_EXCEPTION_HPP
#define NEXUS_COMPLIANCE_CHECK_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Nexus/Compliance/Compliance.hpp"

namespace Nexus::Compliance {

  /** Exception to indicate that an operation failed a compliance check. */
  class ComplianceCheckException : public std::runtime_error,
      public boost::exception {
    public:

      /**
       * Constructs a ComplianceCheckException.
       * @param message A message describing the failure.
       */
      explicit ComplianceCheckException(const std::string& message);

      virtual ~ComplianceCheckException() throw();
  };

  inline ComplianceCheckException::ComplianceCheckException(
    const std::string& message)
    : std::runtime_error{message} {}

  inline ComplianceCheckException::~ComplianceCheckException() throw() {}
}

#endif
