#ifndef NEXUS_SYMBOLRESTRICTIONCOMPLIANCERULE_HPP
#define NEXUS_SYMBOLRESTRICTIONCOMPLIANCERULE_HPP
#include <vector>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class SymbolRestrictionComplianceRule
      \brief Restricts orders that are submitted on any symbol found within a
             list of symbols.
   */
  class SymbolRestrictionComplianceRule : public ComplianceRule {
    public:

      //! Constructs a SymbolRestrictionComplianceRule.
      /*!
        \param parameters The list of parameters used by this rule.
      */
      explicit SymbolRestrictionComplianceRule(
        const std::vector<ComplianceParameter>& parameters);

      //! Constructs a SymbolRestrictionComplianceRule.
      /*!
        \param restrictions The set of Securities to restrict.
      */
      explicit SymbolRestrictionComplianceRule(SecuritySet restrictions);

      virtual void Submit(const OrderExecutionService::Order& order);

    private:
      SecuritySet m_restrictions;
  };

  //! Builds a ComplianceRuleSchema representing a
  //! SymbolRestrictionComplianceRule.
  inline ComplianceRuleSchema BuildSymbolRestrictionComplianceRuleSchema() {
    std::vector<ComplianceValue> symbols;
    symbols.push_back(Security{});
    std::vector<ComplianceParameter> parameters;
    parameters.emplace_back("symbols", symbols);
    ComplianceRuleSchema schema{"symbol_restriction", parameters};
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
      : m_restrictions{std::move(restrictions)} {}

  inline void SymbolRestrictionComplianceRule::Submit(
      const OrderExecutionService::Order& order) {
    if(m_restrictions.Contains(order.GetInfo().m_fields.m_security)) {
      throw ComplianceCheckException{"Submission restricted on symbol."};
    }
  }
}
}

#endif
