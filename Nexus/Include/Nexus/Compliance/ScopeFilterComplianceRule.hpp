#ifndef NEXUS_SCOPE_FILTER_COMPLIANCE_RULE_HPP
#define NEXUS_SCOPE_FILTER_COMPLIANCE_RULE_HPP
#include "Nexus/Definitions/Scope.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"

namespace Nexus {

  /** Applies a ComplianceRule only within a specified Scope. */
  class ScopeFilterComplianceRule : public ComplianceRule {
    public:

      /**
       * Constructs a ScopeFilterComplianceRule.
       * @param scope The Scope this rule applies to.
       * @param rule The ComplianceRule to apply.
       */
      ScopeFilterComplianceRule(
        Scope scope, std::unique_ptr<ComplianceRule> rule);

      void submit(const std::shared_ptr<Order>& order) override;
      void cancel(const std::shared_ptr<Order>& order) override;
      void add(const std::shared_ptr<Order>& order) override;

    private:
      Scope m_scope;
      std::unique_ptr<ComplianceRule> m_rule;
  };

  /** The standard name used to identify the ScopeFilterComplianceRule. */
  inline const auto SCOPE_FILTER_RULE_NAME = std::string("scope_filter");

  /**
   * Returns a ComplianceRuleSchema representing a ScopeFilterComplianceRule.
   * @param schema The ComplianceRuleSchema to apply to the filtered scope.
   */
  inline ComplianceRuleSchema make_scope_filter_compliance_rule_schema(
      const ComplianceRuleSchema& schema) {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("scope", Scope::GLOBAL);
    return wrap(SCOPE_FILTER_RULE_NAME, std::move(parameters), schema);
  }

  /**
   * Makes a new ScopeFilterComplianceRule.
   * @param parameters The parameters used to construct the rule.
   * @param rule The rule to apply within the time period.
   */
  inline auto make_scope_filter_compliance_rule(
      const std::vector<ComplianceParameter>& parameters,
      std::unique_ptr<ComplianceRule> rule) {
    auto scope = Scope::GLOBAL;
    for(auto& parameter : parameters) {
      if(parameter.m_name == "scope") {
        scope = boost::get<Scope>(parameter.m_value);
      }
    }
    return std::make_unique<ScopeFilterComplianceRule>(
      std::move(scope), std::move(rule));
  }

  inline ScopeFilterComplianceRule::ScopeFilterComplianceRule(
    Scope scope, std::unique_ptr<ComplianceRule> rule)
    : m_scope(std::move(scope)),
      m_rule(std::move(rule)) {}

  inline void ScopeFilterComplianceRule::submit(
      const std::shared_ptr<Order>& order) {
    if(m_scope.contains(order->get_info().m_fields.m_ticker)) {
      m_rule->submit(order);
    } else {
      add(order);
    }
  }

  inline void ScopeFilterComplianceRule::cancel(
      const std::shared_ptr<Order>& order) {
    if(m_scope.contains(order->get_info().m_fields.m_ticker)) {
      m_rule->cancel(order);
    }
  }

  inline void ScopeFilterComplianceRule::add(
      const std::shared_ptr<Order>& order) {
    m_rule->add(order);
  }
}

#endif
