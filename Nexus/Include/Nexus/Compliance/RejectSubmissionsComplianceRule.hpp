#ifndef NEXUS_REJECTSUBMISSIONSCOMPLIANCERULE_HPP
#define NEXUS_REJECTSUBMISSIONSCOMPLIANCERULE_HPP
#include <string>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class RejectSubmissionsComplianceRule
      \brief Rejects all submissions.
   */
  class RejectSubmissionsComplianceRule : public ComplianceRule {
    public:

      //! Constructs a RejectSubmissionsComplianceRule with a default message.
      RejectSubmissionsComplianceRule();

      //! Constructs a RejectSubmissionsComplianceRule with a specified
      //! rejection message.
      RejectSubmissionsComplianceRule(std::string reason);

      virtual void Submit(const OrderExecutionService::Order& order) override;

    private:
      std::string m_reason;
  };

  inline RejectSubmissionsComplianceRule::RejectSubmissionsComplianceRule()
      : m_reason{"Submissions not allowed."} {}

  inline RejectSubmissionsComplianceRule::RejectSubmissionsComplianceRule(
      std::string reason)
      : m_reason{std::move(reason)} {}

  inline void RejectSubmissionsComplianceRule::Submit(
      const OrderExecutionService::Order& order) {
    throw ComplianceCheckException{m_reason};
  }
}
}

#endif
