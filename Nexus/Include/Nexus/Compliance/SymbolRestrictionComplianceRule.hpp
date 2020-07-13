#ifndef NEXUS_SYMBOL_RESTRICTION_COMPLIANCE_RULE_HPP
#define NEXUS_SYMBOL_RESTRICTION_COMPLIANCE_RULE_HPP
#include <vector>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus::Compliance {

  /**
   * Restricts orders that are submitted on any symbol found within a list of
   * symbols.
   */
  class SymbolRestrictionComplianceRule : public ComplianceRule {
    public:

      /**
       * Constructs a SymbolRestrictionComplianceRule.
       * @param parameters The list of parameters used by this rule.
       */
      explicit SymbolRestrictionComplianceRule(
        const std::vector<ComplianceParameter>& parameters);

      /**
       * Constructs a SymbolRestrictionComplianceRule.
       * @param restrictions The set of Securities to restrict.
       */
      explicit SymbolRestrictionComplianceRule(SecuritySet restrictions);

      void Submit(const OrderExecutionService::Order& order) override;

    private:
      SecuritySet m_restrictions;
  };

  /**
   * Builds a ComplianceRuleSchema representing a
   * SymbolRestrictionComplianceRule.
   */
  inline ComplianceRuleSchema BuildSymbolRestrictionComplianceRuleSchema() {
    auto symbols = std::vector<ComplianceValue>();
    symbols.push_back(Security());
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("symbols", symbols);
    auto schema = ComplianceRuleSchema("symbol_restriction", parameters);
    return schema;
  }

  inline SymbolRestrictionComplianceRule::SymbolRestrictionComplianceRule(
      const std::vector<ComplianceParameter>& parameters) {
    for(auto& parameter : parameters) {
      if(parameter.m_name == "symbols") {
        for(auto& security : boost::get<std::vector<ComplianceValue>>(
            parameter.m_value)) {
          m_restrictions.Add(std::move(boost::get<Security>(security)));
        }
      }
    }
  }

  inline SymbolRestrictionComplianceRule::SymbolRestrictionComplianceRule(
    SecuritySet restrictions)
    : m_restrictions(std::move(restrictions)) {}

  inline void SymbolRestrictionComplianceRule::Submit(
      const OrderExecutionService::Order& order) {
    if(m_restrictions.Contains(order.GetInfo().m_fields.m_security)) {
      throw ComplianceCheckException("Submission restricted on symbol.");
    }
  }
}

#endif
