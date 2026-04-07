#ifndef NEXUS_REJECT_SUBMISSIONS_COMPLIANCE_RULE_HPP
#define NEXUS_REJECT_SUBMISSIONS_COMPLIANCE_RULE_HPP
#include <boost/throw_exception.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"

namespace Nexus {

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

      void submit(const std::shared_ptr<Order>& order) override;

    private:
      std::string m_reason;
  };

  /** The standard name used to identify the RejectSubmissionsComplianceRule. */
  inline const auto REJECT_SUBMISSIONS_RULE_NAME =
    std::string("reject_submissions");

  /**
   * Returns a ComplianceRuleSchema representing a
   * RejectSubmissionsComplianceRule.
   */
  inline ComplianceRuleSchema make_reject_submissions_compliance_rule_schema() {
    return ComplianceRuleSchema(REJECT_SUBMISSIONS_RULE_NAME, {});
  }

  inline RejectSubmissionsComplianceRule::RejectSubmissionsComplianceRule()
    : RejectSubmissionsComplianceRule("Submissions not allowed.") {}

  inline RejectSubmissionsComplianceRule::RejectSubmissionsComplianceRule(
    std::string reason)
    : m_reason(std::move(reason)) {}

  inline void RejectSubmissionsComplianceRule::submit(
      const std::shared_ptr<Order>& order) {
    boost::throw_with_location(ComplianceCheckException(m_reason));
  }
}

#endif
