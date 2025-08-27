#ifndef NEXUS_REGION_FILTER_COMPLIANCE_RULE_HPP
#define NEXUS_REGION_FILTER_COMPLIANCE_RULE_HPP
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"

namespace Nexus {

  /** Applies a ComplianceRule only within a specified region. */
  class RegionFilterComplianceRule : public ComplianceRule {
    public:

      /**
       * Constructs a RegionFilterComplianceRule.
       * @param region The Region this rule applies to.
       * @param rule The ComplianceRule to apply.
       */
      RegionFilterComplianceRule(
        Region region, std::unique_ptr<ComplianceRule> rule);

      void submit(const std::shared_ptr<Order>& order) override;
      void cancel(const std::shared_ptr<Order>& order) override;
      void add(const std::shared_ptr<Order>& order) override;

    private:
      Region m_region;
      std::unique_ptr<ComplianceRule> m_rule;
  };

  inline RegionFilterComplianceRule::RegionFilterComplianceRule(
    Region region, std::unique_ptr<ComplianceRule> rule)
    : m_region(std::move(region)),
      m_rule(std::move(rule)) {}

  inline void RegionFilterComplianceRule::submit(
      const std::shared_ptr<Order>& order) {
    if(m_region.contains(order->get_info().m_fields.m_security)) {
      m_rule->submit(order);
    } else {
      add(order);
    }
  }

  inline void RegionFilterComplianceRule::cancel(
      const std::shared_ptr<Order>& order) {
    if(m_region.contains(order->get_info().m_fields.m_security)) {
      m_rule->cancel(order);
    }
  }

  inline void RegionFilterComplianceRule::add(
      const std::shared_ptr<Order>& order) {
    m_rule->add(order);
  }
}

#endif
