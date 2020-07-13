#ifndef NEXUS_REJECT_CANCELS_COMPLIANCE_RULE_HPP
#define NEXUS_REJECT_CANCELS_COMPLIANCE_RULE_HPP
#include <string>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"

namespace Nexus::Compliance {

  /** Rejects all cancels. */
  class RejectCancelsComplianceRule : public ComplianceRule {
    public:

      /** Constructs a RejectCancelsComplianceRule with a default message. */
      RejectCancelsComplianceRule();

      /**
       * Constructs a RejectCancelsComplianceRule with a specified rejection
       * message.
       * @param reason The reason to give for rejecting cancels.
       */
      explicit RejectCancelsComplianceRule(std::string reason);

      void Cancel(const OrderExecutionService::Order& order) override;

    private:
      std::string m_reason;
  };

  inline RejectCancelsComplianceRule::RejectCancelsComplianceRule()
    : RejectCancelsComplianceRule("Cancels not allowed.") {}

  inline RejectCancelsComplianceRule::RejectCancelsComplianceRule(
    std::string reason)
    : m_reason(std::move(reason)) {}

  inline void RejectCancelsComplianceRule::Cancel(
      const OrderExecutionService::Order& order) {
    throw ComplianceCheckException(m_reason);
  }
}

#endif
