#ifndef NEXUS_PER_ACCOUNT_COMPLIANCE_RULE_HPP
#define NEXUS_PER_ACCOUNT_COMPLIANCE_RULE_HPP
#include <functional>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus::Compliance {

  /** Applies a ComplianceRule on an account-by-account basis. */
  class PerAccountComplianceRule : public ComplianceRule {
    public:

      /** Returns the canonical name used for this rule. */
      static const std::string& GetName();

      /**
       * Type of function used to build ComplianceRules.
       * @param schema The ComplianceRuleSchema representing the rule to build.
       * @return The ComplianceRule represented by the <i>schema</i>.
       */
      using ComplianceRuleBuilder = std::function<
        std::unique_ptr<ComplianceRule> (const ComplianceRuleSchema& schema)>;

      /**
       * Constructs a PerAccountComplianceRule.
       * @param schema The ComplianceRuleSchema to apply.
       * @param builder Builds the compliance rule.
       */
      PerAccountComplianceRule(ComplianceRuleSchema schema,
        ComplianceRuleBuilder complianceRuleBuilder);

      void Submit(const OrderExecutionService::Order& order) override;

      void Cancel(const OrderExecutionService::Order& order) override;

      void Add(const OrderExecutionService::Order& order) override;

    private:
      ComplianceRuleSchema m_schema;
      ComplianceRuleBuilder m_complianceRuleBuilder;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::unique_ptr<ComplianceRule>> m_accountEntries;
  };

  inline const std::string& PerAccountComplianceRule::GetName() {
    static const auto name = std::string("per_account");
    return name;
  }

  inline PerAccountComplianceRule::PerAccountComplianceRule(
    ComplianceRuleSchema schema, ComplianceRuleBuilder complianceRuleBuilder)
    : m_schema(std::move(schema)),
      m_complianceRuleBuilder(std::move(complianceRuleBuilder)) {}

  inline void PerAccountComplianceRule::Submit(
      const OrderExecutionService::Order& order) {
    auto& rule = *m_accountEntries.GetOrInsert(
      order.GetInfo().m_fields.m_account,
      [&] {
        return m_complianceRuleBuilder(m_schema);
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
        return m_complianceRuleBuilder(m_schema);
      });
    rule.Add(order);
  }
}

#endif
