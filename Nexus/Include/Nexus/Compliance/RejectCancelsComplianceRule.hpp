#ifndef NEXUS_REJECTCANCELSCOMPLIANCERULE_HPP
#define NEXUS_REJECTCANCELSCOMPLIANCERULE_HPP
#include <string>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class RejectCancelsComplianceRule
      \brief Rejects all cancels.
   */
  class RejectCancelsComplianceRule : public ComplianceRule {
    public:

      //! Constructs a RejectCancelsComplianceRule with a default message.
      RejectCancelsComplianceRule();

      //! Constructs a RejectCancelsComplianceRule with a specified rejection
      //! message.
      RejectCancelsComplianceRule(std::string reason);

      virtual void Cancel(const OrderExecutionService::Order& order) override;

    private:
      std::string m_reason;
  };

  inline RejectCancelsComplianceRule::RejectCancelsComplianceRule()
      : m_reason{"Cancels not allowed."} {}

  inline RejectCancelsComplianceRule::RejectCancelsComplianceRule(
      std::string reason)
      : m_reason{std::move(reason)} {}

  inline void RejectCancelsComplianceRule::Cancel(
      const OrderExecutionService::Order& order) {
    throw ComplianceCheckException{m_reason};
  }
}
}

#endif
