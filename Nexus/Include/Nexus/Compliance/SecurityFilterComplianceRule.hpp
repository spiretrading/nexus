#ifndef NEXUS_SECURITY_FILTER_COMPLIANCE_RULE_HPP
#define NEXUS_SECURITY_FILTER_COMPLIANCE_RULE_HPP
#include <Beam/Pointers/Out.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Region.hpp"
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
       * @param region The Region this rule applies to.
       * @param rule The ComplianceRule to apply.
       */
      SecurityFilterComplianceRule(
        Region region, std::unique_ptr<ComplianceRule> rule);

      void Submit(const OrderExecutionService::Order& order) override;

      void Cancel(const OrderExecutionService::Order& order) override;

      void Add(const OrderExecutionService::Order& order) override;

    private:
      Region m_region;
      std::unique_ptr<ComplianceRule> m_rule;
  };

  inline SecurityFilterComplianceRule::SecurityFilterComplianceRule(
    Region region, std::unique_ptr<ComplianceRule> rule)
    : m_region(std::move(region)),
      m_rule(std::move(rule)) {}

  inline void SecurityFilterComplianceRule::Submit(
      const OrderExecutionService::Order& order) {
    if(m_region.Contains(order.GetInfo().m_fields.m_security)) {
      m_rule->Submit(order);
    } else {
      Add(order);
    }
  }

  inline void SecurityFilterComplianceRule::Cancel(
      const OrderExecutionService::Order& order) {
    if(m_region.Contains(order.GetInfo().m_fields.m_security)) {
      m_rule->Cancel(order);
    }
  }

  inline void SecurityFilterComplianceRule::Add(
      const OrderExecutionService::Order& order) {
    m_rule->Add(order);
  }
}

#endif
