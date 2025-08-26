#ifndef NEXUS_COMPLIANCE_RULE_BUILDER_HPP
#define NEXUS_COMPLIANCE_RULE_BUILDER_HPP
#include <Beam/TimeService/TimeClientBox.hpp>
#include <boost/variant/get.hpp>
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
    return nullptr;
  }
}

#endif
