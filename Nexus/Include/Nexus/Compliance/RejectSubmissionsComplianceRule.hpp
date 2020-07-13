#ifndef NEXUS_REJECT_SUBMISSIONS_COMPLIANCE_RULE_HPP
#define NEXUS_REJECT_SUBMISSIONS_COMPLIANCE_RULE_HPP
#include <string>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"

namespace Nexus::Compliance {

  /** Rejects all submissions. */
  class RejectSubmissionsComplianceRule : public ComplianceRule {
    public:

      /**
       * Constructs a RejectSubmissionsComplianceRule with a default message.
       */
      RejectSubmissionsComplianceRule();

      /**
       * Constructs a RejectSubmissionsComplianceRule with a specified rejection
       * message.
       * @param reason The reason to give for rejecting order submissions.
       */
      explicit RejectSubmissionsComplianceRule(std::string reason);

      void Submit(const OrderExecutionService::Order& order) override;

    private:
      std::string m_reason;
  };

  inline RejectSubmissionsComplianceRule::RejectSubmissionsComplianceRule()
    : RejectSubmissionsComplianceRule("Submissions not allowed.") {}

  inline RejectSubmissionsComplianceRule::RejectSubmissionsComplianceRule(
    std::string reason)
    : m_reason(std::move(reason)) {}

  inline void RejectSubmissionsComplianceRule::Submit(
      const OrderExecutionService::Order& order) {
    throw ComplianceCheckException(m_reason);
  }
}

#endif
