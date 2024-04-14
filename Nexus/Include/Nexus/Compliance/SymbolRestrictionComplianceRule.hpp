#ifndef NEXUS_SYMBOL_RESTRICTION_COMPLIANCE_RULE_HPP
#define NEXUS_SYMBOL_RESTRICTION_COMPLIANCE_RULE_HPP
#include <vector>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus::Compliance {

  /** Restricts orders that are submitted in a Region. */
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
       * @param region The Region to restrict.
       */
      explicit SymbolRestrictionComplianceRule(Region region);

      void Submit(const OrderExecutionService::Order& order) override;

    private:
      Region m_region;
  };

  /**
   * Returns a ComplianceRuleSchema representing a
   * SymbolRestrictionComplianceRule.
   */
  inline ComplianceRuleSchema MakeSymbolRestrictionComplianceRuleSchema() {
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
        for(auto& security :
            boost::get<std::vector<ComplianceValue>>(parameter.m_value)) {
          m_region += boost::get<Security>(security);
        }
      }
    }
  }

  inline SymbolRestrictionComplianceRule::SymbolRestrictionComplianceRule(
    Region region)
    : m_region(std::move(region)) {}

  inline void SymbolRestrictionComplianceRule::Submit(
      const OrderExecutionService::Order& order) {
    if(m_region.Contains(order.GetInfo().m_fields.m_security)) {
      throw ComplianceCheckException("Submission restricted on symbol.");
    }
  }
}

#endif
