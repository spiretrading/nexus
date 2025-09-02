#ifndef NEXUS_MAP_COMPLIANCE_RULE_HPP
#define NEXUS_MAP_COMPLIANCE_RULE_HPP
#include <functional>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"

namespace Nexus {

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
      using KeyBuilder = std::function<Key (const Order& order)>;

      /**
       * Constructs a MapComplianceRule.
       * @param schema The ComplianceRuleSchema to apply.
       * @param rule_builder Returns the compliance rule.
       * @param key_builder Returns the key.
       */
      MapComplianceRule(ComplianceRuleSchema schema,
        ComplianceRuleBuilder rule_builder, KeyBuilder key_builder);

      void submit(const std::shared_ptr<Order>& order) override;
      void cancel(const std::shared_ptr<Order>& order) override;
      void add(const std::shared_ptr<Order>& order) override;

    private:
      ComplianceRuleSchema m_schema;
      ComplianceRuleBuilder m_rule_builder;
      KeyBuilder m_key_builder;
      Beam::SynchronizedUnorderedMap<Key, std::unique_ptr<ComplianceRule>>
        m_rules;
  };

  template<typename K>
  MapComplianceRule(ComplianceRuleSchema,
    std::function<std::unique_ptr<ComplianceRule>(const ComplianceRuleSchema&)>,
    K) -> MapComplianceRule<std::invoke_result_t<K, const Order&>>;

  /**
   * Defines a MapComplianceRule that applies on an account by account basis.
   */
  using PerAccountComplianceRule =
    MapComplianceRule<Beam::ServiceLocator::DirectoryEntry>;

  /** The name of a PerAccountComplianceRuleSchema. */
  inline const auto PER_ACCOUNT_COMPLIANCE_RULE_NAME = "per_account";

  /**
   * Returns a MapComplianceRule that applies per Security.
   * @param schema The ComplianceRuleSchema to apply.
   * @param rule_builder Returns the compliance rule.
   */
  inline std::unique_ptr<MapComplianceRule<Security>>
      make_map_security_compliance_rule(ComplianceRuleSchema schema,
        MapComplianceRule<Security>::ComplianceRuleBuilder rule_builder) {
    return std::make_unique<MapComplianceRule<Security>>(
      std::move(schema), std::move(rule_builder), [] (const auto& order) {
        return order.get_info().m_fields.m_security;
      });
  }

  template<typename K>
  MapComplianceRule<K>::MapComplianceRule(ComplianceRuleSchema schema,
      ComplianceRuleBuilder rule_builder, KeyBuilder key_builder)
    : m_schema(std::move(schema)),
      m_rule_builder(std::move(rule_builder)),
      m_key_builder(std::move(key_builder)) {}

  template<typename K>
  void MapComplianceRule<K>::submit(const std::shared_ptr<Order>& order) {
    auto& rule = *m_rules.GetOrInsert(m_key_builder(*order), [&] {
      return m_rule_builder(m_schema);
    });
    rule.submit(order);
  }

  template<typename K>
  void MapComplianceRule<K>::cancel(const std::shared_ptr<Order>& order) {
    auto rule = m_rules.Find(m_key_builder(*order));
    if(!rule) {
      return;
    }
    (*rule)->cancel(order);
  }

  template<typename K>
  void MapComplianceRule<K>::add(const std::shared_ptr<Order>& order) {
    auto& rule = *m_rules.GetOrInsert(m_key_builder(*order), [&] {
      return m_rule_builder(m_schema);
    });
    rule.add(order);
  }
}

#endif
