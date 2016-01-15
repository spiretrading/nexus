#ifndef NEXUS_MAPCOMPLIANCERULE_HPP
#define NEXUS_MAPCOMPLIANCERULE_HPP
#include <functional>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class MapComplianceRule
      \brief Maps a key to an independent set of ComplianceRules.
      \tparam KeyType The type of key used as an index to the ComplianceRule to
              apply.
   */
  template<typename KeyType>
  class MapComplianceRule : public ComplianceRule {
    public:

      //! The type of key used as an index to the ComplianceRule to apply.
      using Key = KeyType;

      //! Type of function used to build ComplianceRules.
      /*!
        \param schema The ComplianceRuleSchema representing the rule to build.
        \return The ComplianceRule represented by the <i>schema</i>.
      */
      using ComplianceRuleBuilder = std::function<
        std::unique_ptr<ComplianceRule> (const ComplianceRuleSchema& schema)>;

      //! The type of function used to compute the key.
      using KeyBuilder = std::function<
        Key (const OrderExecutionService::Order& order)>;

      //! Constructs a MapComplianceRule.
      /*!
        \param schema The ComplianceRuleSchema to apply.
        \param complianceRuleBuilder Builds the compliance rule.
        \param keyBuilder Builds the key.
      */
      MapComplianceRule(ComplianceRuleSchema schema,
        ComplianceRuleBuilder complianceRuleBuilder, KeyBuilder keyBuilder);

      virtual void Submit(const OrderExecutionService::Order& order) override;

      virtual void Cancel(const OrderExecutionService::Order& order) override;

      virtual void Add(const OrderExecutionService::Order& order) override;

    private:
      ComplianceRuleSchema m_schema;
      ComplianceRuleBuilder m_complianceRuleBuilder;
      KeyBuilder m_keyBuilder;
      Beam::SynchronizedUnorderedMap<Key, std::unique_ptr<ComplianceRule>>
        m_rules;
  };

  template<typename KeyType>
  MapComplianceRule<KeyType>::MapComplianceRule(ComplianceRuleSchema schema,
      ComplianceRuleBuilder complianceRuleBuilder, KeyBuilder keyBuilder)
      : m_schema{std::move(schema)},
        m_complianceRuleBuilder{std::move(complianceRuleBuilder)},
        m_keyBuilder{std::move(keyBuilder)} {}

  template<typename KeyType>
  void MapComplianceRule<KeyType>::Submit(
      const OrderExecutionService::Order& order) {
    auto& rule = *m_rules.GetOrInsert(m_keyBuilder(order),
      [&] {
        return m_complianceRuleBuilder(m_schema);
      });
    rule.Submit(order);
  }

  template<typename KeyType>
  void MapComplianceRule<KeyType>::Cancel(
      const OrderExecutionService::Order& order) {
    auto rule = m_rules.Find(m_keyBuilder(order));
    if(!rule.is_initialized()) {
      return;
    }
    (*rule)->Cancel(order);
  }

  template<typename KeyType>
  void MapComplianceRule<KeyType>::Add(
      const OrderExecutionService::Order& order) {
    auto& rule = *m_rules.GetOrInsert(m_keyBuilder(order),
      [&] {
        return m_complianceRuleBuilder(m_schema);
      });
    rule.Add(order);
  }
}
}

#endif
