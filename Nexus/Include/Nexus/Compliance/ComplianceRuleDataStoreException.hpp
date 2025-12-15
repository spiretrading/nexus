#ifndef NEXUS_COMPLIANCE_RULE_DATA_STORE_EXCEPTION_HPP
#define NEXUS_COMPLIANCE_RULE_DATA_STORE_EXCEPTION_HPP
#include <Beam/IO/IOException.hpp>

namespace Nexus {

  /** Exception to indicate an operation on a ComplianceRuleDataStore failed. */
  class ComplianceRuleDataStoreException : public Beam::IOException {
    public:
      using Beam::IOException::IOException;

      /** Constructs a ComplianceRuleDataStoreException. */
      ComplianceRuleDataStoreException();
  };

  inline ComplianceRuleDataStoreException::ComplianceRuleDataStoreException()
    : ComplianceRuleDataStoreException("Operation failed.") {}
}

#endif
