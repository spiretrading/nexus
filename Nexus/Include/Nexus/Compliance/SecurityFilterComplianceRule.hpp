#ifndef NEXUS_SECURITY_FILTER_COMPLIANCE_RULE_HPP
#define NEXUS_SECURITY_FILTER_COMPLIANCE_RULE_HPP
#include <Beam/Pointers/Out.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceParameter.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus::Compliance {

  /** Applies a ComplianceRule only to a specified set of Securities. */
  class SecurityFilterComplianceRule : public ComplianceRule {
    public:

      /**
       * Constructs a SecurityFilterComplianceRule.
       * @param securities The set of Securities the rule applies to.
       * @param rule The ComplianceRule to apply.
       */
      SecurityFilterComplianceRule(SecuritySet securities,
        std::unique_ptr<ComplianceRule> rule);

      void Submit(const OrderExecutionService::Order& order) override;

      void Cancel(const OrderExecutionService::Order& order) override;

      void Add(const OrderExecutionService::Order& order) override;

    private:
      SecuritySet m_securities;
      std::unique_ptr<ComplianceRule> m_rule;
  };

  inline SecurityFilterComplianceRule::SecurityFilterComplianceRule(
    SecuritySet securities, std::unique_ptr<ComplianceRule> rule)
    : m_securities(std::move(securities)),
      m_rule(std::move(rule)) {}

  inline void SecurityFilterComplianceRule::Submit(
      const OrderExecutionService::Order& order) {
    if(m_securities.Contains(order.GetInfo().m_fields.m_security)) {
      m_rule->Submit(order);
    } else {
      Add(order);
    }
  }

  inline void SecurityFilterComplianceRule::Cancel(
      const OrderExecutionService::Order& order) {
    if(m_securities.Contains(order.GetInfo().m_fields.m_security)) {
      m_rule->Cancel(order);
    }
  }

  inline void SecurityFilterComplianceRule::Add(
      const OrderExecutionService::Order& order) {
    m_rule->Add(order);
  }
}

#endif
