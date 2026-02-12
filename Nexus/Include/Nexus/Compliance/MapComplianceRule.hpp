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
  MapComplianceRule(ComplianceRuleSchema, std::function<
    std::unique_ptr<ComplianceRule> (const ComplianceRuleSchema&)>, K) ->
      MapComplianceRule<std::invoke_result_t<K, const Order&>>;

  /**
   * Defines a MapComplianceRule that applies on an account by account basis.
   */
  using PerAccountComplianceRule = MapComplianceRule<Beam::DirectoryEntry>;

  /** The standard name used to identify the PerAccountComplianceRule. */
  inline const auto PER_ACCOUNT_RULE_NAME = std::string("per_account");

  /**
   * Returns a ComplianceRuleSchema representing a PerAccountComplianceRule.
   * @param schema The ComplianceRuleSchema to apply per account.
   */
  inline ComplianceRuleSchema make_per_account_compliance_rule_schema(
      const ComplianceRuleSchema& schema) {
    return wrap(PER_ACCOUNT_RULE_NAME, schema);
  }

  /**
   * Makes a new PerTickerComplianceRule.
   * @param schema The ComplianceRuleSchema to apply.
   * @param rule_builder Builds an instance of the compliance rule per account.
   */
  inline std::unique_ptr<PerAccountComplianceRule>
      make_per_account_compliance_rule(ComplianceRuleSchema schema,
        PerAccountComplianceRule::ComplianceRuleBuilder rule_builder) {
    return std::make_unique<PerAccountComplianceRule>(
      std::move(schema), std::move(rule_builder), [] (const auto& order) {
        return order.get_info().m_fields.m_account;
      });
  }

  /** Defines a MapComplianceRule that applies to each ticker individually. */
  using PerTickerComplianceRule = MapComplianceRule<Ticker>;

  /** The standard name used to identify the PerTickerComplianceRule. */
  inline const auto PER_TICKER_RULE_NAME = std::string("per_ticker");

  /**
   * Returns a ComplianceRuleSchema representing a PerTickerComplianceRule.
   * @param schema The ComplianceRuleSchema to apply per ticker.
   */
  inline ComplianceRuleSchema make_per_ticker_compliance_rule_schema(
      const ComplianceRuleSchema& schema) {
    return wrap(PER_TICKER_RULE_NAME, schema);
  }

  /**
   * Makes a new PerTickerComplianceRule.
   * @param schema The ComplianceRuleSchema to apply.
   * @param rule_builder Builds an instance of the compliance rule per ticker.
   */
  inline std::unique_ptr<PerTickerComplianceRule>
      make_per_ticker_compliance_rule(ComplianceRuleSchema schema,
        PerTickerComplianceRule::ComplianceRuleBuilder rule_builder) {
    return std::make_unique<PerTickerComplianceRule>(
      std::move(schema), std::move(rule_builder), [] (const auto& order) {
        return order.get_info().m_fields.m_ticker;
      });
  }

  /** Defines a MapComplianceRule that applies to each Side individually. */
  using PerSideComplianceRule = MapComplianceRule<Side>;

  /** The standard name used to identify the PerSideComplianceRule. */
  inline const auto PER_SIDE_RULE_NAME = std::string("per_side");

  /**
   * Returns a ComplianceRuleSchema representing a PerSideComplianceRule.
   * @param schema The ComplianceRuleSchema to apply per side.
   */
  inline ComplianceRuleSchema make_per_side_compliance_rule_schema(
      const ComplianceRuleSchema& schema) {
    return wrap(PER_SIDE_RULE_NAME, schema);
  }

  /**
   * Makes a new PerSideComplianceRule.
   * @param schema The ComplianceRuleSchema to apply.
   * @param rule_builder Builds an instance of the compliance rule per Side.
   */
  inline std::unique_ptr<PerSideComplianceRule>
      make_per_side_compliance_rule(ComplianceRuleSchema schema,
        PerTickerComplianceRule::ComplianceRuleBuilder rule_builder) {
    return std::make_unique<PerSideComplianceRule>(
      std::move(schema), std::move(rule_builder), [] (const auto& order) {
        return order.get_info().m_fields.m_side;
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
    auto& rule = *m_rules.get_or_insert(m_key_builder(*order), [&] {
      return m_rule_builder(m_schema);
    });
    rule.submit(order);
  }

  template<typename K>
  void MapComplianceRule<K>::cancel(const std::shared_ptr<Order>& order) {
    if(auto rule = m_rules.find(m_key_builder(*order))) {
      (*rule)->cancel(order);
    }
  }

  template<typename K>
  void MapComplianceRule<K>::add(const std::shared_ptr<Order>& order) {
    auto& rule = *m_rules.get_or_insert(m_key_builder(*order), [&] {
      return m_rule_builder(m_schema);
    });
    rule.add(order);
  }
}

#endif
