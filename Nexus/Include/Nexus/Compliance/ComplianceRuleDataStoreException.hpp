#ifndef NEXUS_COMPLIANCE_RULE_DATA_STORE_EXCEPTION_HPP
#define NEXUS_COMPLIANCE_RULE_DATA_STORE_EXCEPTION_HPP
#include <Beam/IO/IOException.hpp>
#include "Nexus/Compliance/Compliance.hpp"

namespace Nexus::Compliance {

  /** Exception to indicate an operation on a ComplianceRuleDataStore failed. */
  class ComplianceRuleDataStoreException : public Beam::IO::IOException {
    public:

      /** Constructs a ComplianceRuleDataStoreException. */
      ComplianceRuleDataStoreException();

      /**
       * Constructs a ComplianceRuleDataStoreException.
       * @param message A message describing the error.
       */
      explicit ComplianceRuleDataStoreException(const std::string& message);

      virtual ~ComplianceRuleDataStoreException() throw();
  };

  inline ComplianceRuleDataStoreException::ComplianceRuleDataStoreException()
    : Beam::IO::IOException("Operation failed.") {}

  inline ComplianceRuleDataStoreException::ComplianceRuleDataStoreException(
    const std::string& message)
    : Beam::IO::IOException(message) {}

  inline ComplianceRuleDataStoreException::~ComplianceRuleDataStoreException()
    throw() {}
}

#endif
