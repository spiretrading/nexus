#ifndef NEXUS_COMPLIANCE_RULE_BUILDER_HPP
#define NEXUS_COMPLIANCE_RULE_BUILDER_HPP
#include <Beam/TimeService/TimeClientBox.hpp>
#include <boost/variant/get.hpp>
#include "Nexus/Compliance/BuyingPowerComplianceRule.hpp"
#include "Nexus/Compliance/CancelRestrictionPeriodComplianceRule.hpp"
#include "Nexus/Compliance/OpposingOrderCancellationComplianceRule.hpp"
#include "Nexus/Compliance/OpposingOrderSubmissionComplianceRule.hpp"
#include "Nexus/Compliance/OrderCountPerSideComplianceRule.hpp"
#include "Nexus/Compliance/PerAccountComplianceRule.hpp"
#include "Nexus/Compliance/SubmissionRestrictionPeriodComplianceRule.hpp"
#include "Nexus/Compliance/SymbolRestrictionComplianceRule.hpp"
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus {

  /**
   * Returns a ComplianceRule from a ComplianceRuleSchema.
   * @param schema The ComplianceRuleSchema to build the ComplianceRule from.
   * @param market_data_client The MarketDataClient needed by various rules.
   * @param definitions_client The DefinitionsClient needed by various rules.
   * @param time_client The TimeClient needed by various rules.
   * @return The ComplianceRule represented by the <i>schema</i>.
   */
  template<IsMarketDataClient MarketDataClient,
    IsDefinitionsClient DefinitionsClient, typename TimeClient>
  std::unique_ptr<ComplianceRule> make_compliance_rule(
      const ComplianceRuleSchema& schema, MarketDataClient& market_data_client,
      DefinitionsClient& definitions_client, TimeClient& time_client) {
    if(schema.get_name() == "buying_power") {
      return std::make_unique<BuyingPowerComplianceRule<MarketDataClient*>>(
        schema.get_parameters(), definitions_client.load_exchange_rates(),
        &market_data_client);
    } else if(schema.get_name() == "cancel_restriction_period") {
      return std::make_unique<
        CancelRestrictionPeriodComplianceRule<TimeClient*>>(
          schema.get_parameters(), &time_client);
    } else if(schema.get_name() == "opposing_order_cancellation") {
      return make_opposing_order_cancellation_compliance_rule(
        schema.get_parameters(), &time_client);
    } else if(schema.get_name() == "opposing_order_submission") {
      return make_opposing_order_submission_compliance_rule(
        schema.get_parameters(), &time_client);
    } else if(schema.get_name() == "orders_per_side_limit") {
      return std::make_unique<OrderCountPerSideComplianceRule>(
        schema.get_parameters());
    } else if(schema.get_name() == "submission_restriction_period") {
      return std::make_unique<
        SubmissionRestrictionPeriodComplianceRule<TimeClient*>>(
          schema.get_parameters(), &time_client);
    } else if(schema.get_name() == "symbol_restriction") {
      return std::make_unique<SymbolRestrictionComplianceRule>(
        schema.get_parameters());
    } else if(schema.get_name() == PerAccountComplianceRule::NAME) {
      auto name = std::string();
      auto parameters = std::vector<ComplianceParameter>();
      for(auto& parameter : schema.get_parameters()) {
        if(parameter.m_name == "name") {
          name = boost::get<std::string>(parameter.m_value);
        } else if(!parameter.m_name.empty() &&
            parameter.m_name.front() == '\\') {
          parameters.emplace_back(
            parameter.m_name.substr(1), parameter.m_value);
        }
      }
      auto per_account_schema =
        ComplianceRuleSchema(std::move(name), std::move(parameters));
      return std::make_unique<PerAccountComplianceRule>(per_account_schema,
        [&market_data_client, &definitions_client, &time_client] (
            const ComplianceRuleSchema& schema) {
          return make_compliance_rule(
            schema, market_data_client, definitions_client, time_client);
        });
    }
    return nullptr;
  }
}

#endif
