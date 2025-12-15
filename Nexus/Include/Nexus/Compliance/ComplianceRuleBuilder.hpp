#ifndef NEXUS_COMPLIANCE_RULE_BUILDER_HPP
#define NEXUS_COMPLIANCE_RULE_BUILDER_HPP
#include <memory>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/variant/get.hpp>
#include "Nexus/Compliance/BuyingPowerComplianceRule.hpp"
#include "Nexus/Compliance/MapComplianceRule.hpp"
#include "Nexus/Compliance/OpposingCancelComplianceRule.hpp"
#include "Nexus/Compliance/OpposingSubmissionComplianceRule.hpp"
#include "Nexus/Compliance/OrderCountLimitComplianceRule.hpp"
#include "Nexus/Compliance/RegionFilterComplianceRule.hpp"
#include "Nexus/Compliance/RejectCancelsComplianceRule.hpp"
#include "Nexus/Compliance/RejectSubmissionsComplianceRule.hpp"
#include "Nexus/Compliance/TimeFilterComplianceRule.hpp"
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
  std::unique_ptr<ComplianceRule> make_compliance_rule(
      const ComplianceRuleSchema& schema,
      IsMarketDataClient auto& market_data_client,
      IsDefinitionsClient auto& definitions_client,
      Beam::IsTimeClient auto& time_client) {
    if(schema.get_name() == BUYING_POWER_COMPLIANCE_RULE_NAME) {
      return make_buying_power_compliance_rule(schema.get_parameters(),
        ExchangeRateTable(definitions_client.load_exchange_rates()),
        market_data_client);
    } else if(schema.get_name() == OPPOSING_CANCEL_RULE_NAME) {
      return make_opposing_cancel_compliance_rule(
        schema.get_parameters(), time_client);
    } else if(schema.get_name() == OPPOSING_SUBMISSION_RULE_NAME) {
      return make_opposing_submission_compliance_rule(
        schema.get_parameters(), time_client);
    } else if(schema.get_name() == ORDER_COUNT_LIMIT_RULE_NAME) {
      return make_order_count_limit_compliance_rule(schema.get_parameters());
    } else if(schema.get_name() == ORDER_COUNT_LIMIT_PER_SIDE_RULE_NAME) {
      return make_order_count_limit_per_side_compliance_rule(
        schema.get_parameters());
    } else if(schema.get_name() == PER_ACCOUNT_RULE_NAME) {
      return make_per_account_compliance_rule(
        unwrap(schema), [&] (const auto& schema) {
          return make_compliance_rule(schema, market_data_client,
            definitions_client, time_client);
        });
    } else if(schema.get_name() == PER_SECURITY_RULE_NAME) {
      return make_per_security_compliance_rule(
        unwrap(schema), [&] (const auto& schema) {
          return make_compliance_rule(schema, market_data_client,
            definitions_client, time_client);
        });
    } else if(schema.get_name() == PER_SIDE_RULE_NAME) {
      return make_per_side_compliance_rule(
        unwrap(schema), [&] (const auto& schema) {
          return make_compliance_rule(schema, market_data_client,
            definitions_client, time_client);
        });
    } else if(schema.get_name() == REGION_FILTER_RULE_NAME) {
      auto sub_schema = unwrap(schema);
      auto sub_rule = make_compliance_rule(
        sub_schema, market_data_client, definitions_client, time_client);
      return make_region_filter_compliance_rule(
        schema.get_parameters(), std::move(sub_rule));
    } else if(schema.get_name() == REJECT_CANCELS_RULE_NAME) {
      return std::make_unique<RejectCancelsComplianceRule>();
    } else if(schema.get_name() == REJECT_SUBMISSIONS_RULE_NAME) {
      return std::make_unique<RejectSubmissionsComplianceRule>();
    } else if(schema.get_name() == TIME_FILTER_RULE_NAME) {
      auto sub_schema = unwrap(schema);
      auto sub_rule = make_compliance_rule(
        sub_schema, market_data_client, definitions_client, time_client);
      return make_time_filter_compliance_rule(schema.get_parameters(),
        definitions_client.load_time_zone_database(),
        definitions_client.load_venue_database(), time_client,
        std::move(sub_rule));
    }
    return nullptr;
  }
}

#endif
