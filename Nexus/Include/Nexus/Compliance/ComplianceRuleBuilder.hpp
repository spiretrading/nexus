#ifndef NEXUS_COMPLIANCE_RULE_BUILDER_HPP
#define NEXUS_COMPLIANCE_RULE_BUILDER_HPP
#include <boost/variant/get.hpp>
#include "Nexus/Compliance/BuyingPowerComplianceRule.hpp"
#include "Nexus/Compliance/CancelRestrictionPeriodComplianceRule.hpp"
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/OpposingOrderCancellationComplianceRule.hpp"
#include "Nexus/Compliance/OpposingOrderSubmissionComplianceRule.hpp"
#include "Nexus/Compliance/OrderCountPerSideComplianceRule.hpp"
#include "Nexus/Compliance/PerAccountComplianceRule.hpp"
#include "Nexus/Compliance/SubmissionRestrictionPeriodComplianceRule.hpp"
#include "Nexus/Compliance/SymbolRestrictionComplianceRule.hpp"

namespace Nexus::Compliance {

  /**
   * Builds a ComplianceRule from a ComplianceRuleSchema.
   * @param schema The ComplianceRuleSchema to build the ComplianceRule from.
   * @return The ComplianceRule represented by the <i>schema</i>.
   */
  template<typename MarketDataClient, typename DefinitionsClient,
    typename TimeClient>
  std::unique_ptr<ComplianceRule> BuildComplianceRule(
      const ComplianceRuleSchema& schema, MarketDataClient& marketDataClient,
      DefinitionsClient& definitionsClient, TimeClient& timeClient) {
    if(schema.GetName() == "buying_power") {
      return std::make_unique<BuyingPowerComplianceRule<MarketDataClient*>>(
        schema.GetParameters(), definitionsClient.LoadExchangeRates(),
        &marketDataClient);
    } else if(schema.GetName() == "cancel_restriction_period") {
      return std::make_unique<
        CancelRestrictionPeriodComplianceRule<TimeClient*>>(
        schema.GetParameters(), &timeClient);
    } else if(schema.GetName() == "opposing_order_cancellation") {
      return MakeOpposingOrderCancellationComplianceRule(schema.GetParameters(),
        &timeClient);
    } else if(schema.GetName() == "opposing_order_submission") {
      return MakeOpposingOrderSubmissionComplianceRule(schema.GetParameters(),
        &timeClient);
    } else if(schema.GetName() == "orders_per_side_limit") {
      return std::make_unique<OrderCountPerSideComplianceRule>(
        schema.GetParameters());
    } else if(schema.GetName() == "submission_restriction_period") {
      return std::make_unique<
        SubmissionRestrictionPeriodComplianceRule<TimeClient*>>(
        schema.GetParameters(), &timeClient);
    } else if(schema.GetName() == "symbol_restriction") {
      return std::make_unique<SymbolRestrictionComplianceRule>(
        schema.GetParameters());
    } else if(schema.GetName() == PerAccountComplianceRule::GetName()) {
      auto name = std::string();
      auto parameters = std::vector<ComplianceParameter>();
      for(auto& parameter : schema.GetParameters()) {
        if(parameter.m_name == "name") {
          name = boost::get<std::string>(parameter.m_value);
        } else if(parameter.m_name.size() > 0 &&
            parameter.m_name.front() == '\\') {
          parameters.emplace_back(parameter.m_name.substr(1),
            parameter.m_value);
        }
      }
      auto perAccountSchema = ComplianceRuleSchema(std::move(name),
        std::move(parameters));
      return std::make_unique<PerAccountComplianceRule>(perAccountSchema,
        std::bind(&BuildComplianceRule<MarketDataClient, DefinitionsClient,
        TimeClient>, std::placeholders::_1, std::ref(marketDataClient),
        std::ref(definitionsClient), std::ref(timeClient)));
    }
    return nullptr;
  }
}

#endif
