#ifndef NEXUS_REGION_FILTER_COMPLIANCE_RULE_HPP
#define NEXUS_REGION_FILTER_COMPLIANCE_RULE_HPP
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"

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

  /** The standard name used to identify the RegionFilterComplianceRule. */
  inline const auto REGION_FILTER_RULE_NAME = std::string("region_filter");

  /**
   * Returns a ComplianceRuleSchema representing a RegionFilterComplianceRule.
   * @param schema The ComplianceRuleSchema to apply to the filtered region.
   */
  inline ComplianceRuleSchema make_region_filter_compliance_rule_schema(
      const ComplianceRuleSchema& schema) {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("region", Region::GLOBAL);
    return wrap(REGION_FILTER_RULE_NAME, std::move(parameters), schema);
  }

  /**
   * Makes a new RegionFilterComplianceRule.
   * @param parameters The parameters used to construct the rule.
   * @param rule The rule to apply within the time period.
   */
  inline auto make_region_filter_compliance_rule(
      const std::vector<ComplianceParameter>& parameters,
      std::unique_ptr<ComplianceRule> rule) {
    auto region = Region::GLOBAL;
    for(auto& parameter : parameters) {
      if(parameter.m_name == "region") {
        region = boost::get<Region>(parameter.m_value);
      }
    }
    return std::make_unique<RegionFilterComplianceRule>(
      std::move(region), std::move(rule));
  }

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
