#ifndef NEXUS_MAP_COMPLIANCE_RULE_HPP
#define NEXUS_MAP_COMPLIANCE_RULE_HPP
#include <functional>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus::Compliance {

  /**
   * Maps a key to an independent set of ComplianceRules.
   * @param <K> The type of key used as an index to the ComplianceRule to apply.
   */
  template<typename K>
  class MapComplianceRule : public ComplianceRule {
    public:

      /** The type of key used as an index to the ComplianceRule to apply. */
      using Key = K;

      /**
       * Type of function used to build ComplianceRules.
       * @param schema The ComplianceRuleSchema representing the rule to build.
       * @return The ComplianceRule represented by the <i>schema</i>.
       */
      using ComplianceRuleBuilder = std::function<
        std::unique_ptr<ComplianceRule> (const ComplianceRuleSchema& schema)>;

      /** The type of function used to compute the key. */
      using KeyBuilder = std::function<
        Key (const OrderExecutionService::Order& order)>;

      /**
       * Constructs a MapComplianceRule.
       * @param schema The ComplianceRuleSchema to apply.
       * @param complianceRuleBuilder Builds the compliance rule.
       * @param keyBuilder Builds the key.
       */
      MapComplianceRule(ComplianceRuleSchema schema,
        ComplianceRuleBuilder complianceRuleBuilder, KeyBuilder keyBuilder);

      void Submit(const OrderExecutionService::Order& order) override;

      void Cancel(const OrderExecutionService::Order& order) override;

      void Add(const OrderExecutionService::Order& order) override;

    private:
      ComplianceRuleSchema m_schema;
      ComplianceRuleBuilder m_complianceRuleBuilder;
      KeyBuilder m_keyBuilder;
      Beam::SynchronizedUnorderedMap<Key, std::unique_ptr<ComplianceRule>>
        m_rules;
  };

  /**
   * Builds a MapComplianceRule that applies per Security.
   * @param schema The ComplianceRuleSchema to apply.
   * @param complianceRuleBuilder Builds the compliance rule.
   */
  inline std::unique_ptr<MapComplianceRule<Security>>
      MakeMapSecurityComplianceRule(ComplianceRuleSchema schema,
      MapComplianceRule<Security>::ComplianceRuleBuilder
      complianceRuleBuilder) {
    return std::make_unique<MapComplianceRule<Security>>(
      std::move(schema), std::move(complianceRuleBuilder),
      [] (const auto& order) {
        return order.GetInfo().m_fields.m_security;
      });
  }

  template<typename K>
  MapComplianceRule<K>::MapComplianceRule(ComplianceRuleSchema schema,
    ComplianceRuleBuilder complianceRuleBuilder, KeyBuilder keyBuilder)
    : m_schema(std::move(schema)),
      m_complianceRuleBuilder(std::move(complianceRuleBuilder)),
      m_keyBuilder(std::move(keyBuilder)) {}

  template<typename K>
  void MapComplianceRule<K>::Submit(const OrderExecutionService::Order& order) {
    auto& rule = *m_rules.GetOrInsert(m_keyBuilder(order),
      [&] {
        return m_complianceRuleBuilder(m_schema);
      });
    rule.Submit(order);
  }

  template<typename K>
  void MapComplianceRule<K>::Cancel(const OrderExecutionService::Order& order) {
    auto rule = m_rules.Find(m_keyBuilder(order));
    if(!rule.is_initialized()) {
      return;
    }
    (*rule)->Cancel(order);
  }

  template<typename K>
  void MapComplianceRule<K>::Add(const OrderExecutionService::Order& order) {
    auto& rule = *m_rules.GetOrInsert(m_keyBuilder(order),
      [&] {
        return m_complianceRuleBuilder(m_schema);
      });
    rule.Add(order);
  }
}

#endif
