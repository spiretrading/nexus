#ifndef NEXUS_REJECT_CANCELS_COMPLIANCE_RULE_HPP
#define NEXUS_REJECT_CANCELS_COMPLIANCE_RULE_HPP
#include <boost/throw_exception.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"

namespace Nexus {

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

      void cancel(const std::shared_ptr<Order>& order) override;

    private:
      std::string m_reason;
  };

  /** The standard name used to identify the RejectCancelsComplianceRule. */
  inline const auto REJECT_CANCELS_RULE_NAME = std::string("reject_cancels");

  /**
   * Returns a ComplianceRuleSchema representing a RejectCancelsComplianceRule.
   */
  inline ComplianceRuleSchema make_reject_cancels_compliance_rule_schema() {
    return ComplianceRuleSchema(REJECT_CANCELS_RULE_NAME, {});
  }

  inline RejectCancelsComplianceRule::RejectCancelsComplianceRule()
    : RejectCancelsComplianceRule("Cancels not allowed.") {}

  inline RejectCancelsComplianceRule::RejectCancelsComplianceRule(
    std::string reason)
    : m_reason(std::move(reason)) {}

  inline void RejectCancelsComplianceRule::cancel(
      const std::shared_ptr<Order>& order) {
    boost::throw_with_location(ComplianceCheckException(m_reason));
  }
}

#endif
