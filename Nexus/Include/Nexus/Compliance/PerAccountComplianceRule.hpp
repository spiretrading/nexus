#ifndef NEXUS_PERACCOUNTCOMPLIANCERULE_HPP
#define NEXUS_PERACCOUNTCOMPLIANCERULE_HPP
#include <functional>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class PerAccountComplianceRule
      \brief Applies a ComplianceRule on an account-by-account basis.
   */
  class PerAccountComplianceRule : public ComplianceRule {
    public:

      //! Returns the canonical name used for this rule.
      static std::string GetName();

      //! Type of function used to build ComplianceRules.
      /*!
        \param schema The ComplianceRuleSchema representing the rule to build.
        \return The ComplianceRule represented by the <i>schema</i>.
      */
      using ComplianceRuleBuilder = std::function<
        std::unique_ptr<ComplianceRule> (const ComplianceRuleSchema& schema)>;

      //! Constructs a PerAccountComplianceRule.
      /*!
        \param schema The ComplianceRuleSchema to apply.
        \param builder Builds the compliance rule.
      */
      PerAccountComplianceRule(ComplianceRuleSchema schema,
        ComplianceRuleBuilder complianceRuleBuilder);

      virtual void Submit(const OrderExecutionService::Order& order);

      virtual void Cancel(const OrderExecutionService::Order& order);

      virtual void Add(const OrderExecutionService::Order& order);

    private:
      ComplianceRuleSchema m_schema;
      ComplianceRuleBuilder m_complianceRuleBuilder;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::unique_ptr<ComplianceRule>> m_accountEntries;
  };

  inline std::string PerAccountComplianceRule::GetName() {
    return "per_account";
  }

  inline PerAccountComplianceRule::PerAccountComplianceRule(
      ComplianceRuleSchema schema, ComplianceRuleBuilder complianceRuleBuilder)
      : m_schema{std::move(schema)},
        m_complianceRuleBuilder{std::move(complianceRuleBuilder)} {}

  inline void PerAccountComplianceRule::Submit(
      const OrderExecutionService::Order& order) {
    auto& rule = *m_accountEntries.GetOrInsert(
      order.GetInfo().m_fields.m_account,
      [&] {
        auto rule = m_complianceRuleBuilder(m_schema);
        return rule;
      });
    rule.Submit(order);
  }

  inline void PerAccountComplianceRule::Cancel(
      const OrderExecutionService::Order& order) {
    auto rule = m_accountEntries.Find(order.GetInfo().m_fields.m_account);
    if(!rule.is_initialized()) {
      return;
    }
    (*rule)->Cancel(order);
  }

  inline void PerAccountComplianceRule::Add(
      const OrderExecutionService::Order& order) {
    auto& rule = *m_accountEntries.GetOrInsert(
      order.GetInfo().m_fields.m_account,
      [&] {
        auto rule = m_complianceRuleBuilder(m_schema);
        return rule;
      });
    rule.Add(order);
  }
}
}

#endif
