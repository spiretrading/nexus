#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceRuleBuilder.hpp"
#include "Nexus/DefinitionsService/DefaultDefinitionsClient.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

TEST_SUITE("ComplianceRuleBuilder") {
  TEST_CASE("expected_types") {
    auto market_data_client =
      TestMarketDataClient(std::make_shared<TestMarketDataClient::Queue>());
    auto definitions_client = DefaultDefinitionsClient();
    auto time_client = FixedTimeClient(time_from_string("2024-07-29 10:00:00"));
    {
      auto rule = make_compliance_rule(
        make_buying_power_compliance_rule_schema(), market_data_client,
        definitions_client, time_client);
      REQUIRE(dynamic_cast<BuyingPowerComplianceRule<TestMarketDataClient*>*>(
        rule.get()));
    }
    {
      auto rule = make_compliance_rule(
        make_opposing_submission_compliance_rule_schema(), market_data_client,
        definitions_client, time_client);
      REQUIRE(dynamic_cast<OpposingSubmissionComplianceRule<FixedTimeClient*>*>(
        rule.get()));
    }
    {
      auto rule = make_compliance_rule(
        make_order_count_limit_compliance_rule_schema(), market_data_client,
        definitions_client, time_client);
      REQUIRE(dynamic_cast<OrderCountLimitComplianceRule*>(rule.get()));
    }
    {
      auto rule = make_compliance_rule(
        make_order_count_limit_per_side_compliance_rule_schema(),
        market_data_client, definitions_client, time_client);
      REQUIRE(dynamic_cast<PerSideComplianceRule*>(rule.get()));
    }
    {
      auto schema = make_per_account_compliance_rule_schema(
        make_buying_power_compliance_rule_schema());
      auto rule = make_compliance_rule(schema, market_data_client,
        definitions_client, time_client);
      REQUIRE(dynamic_cast<PerAccountComplianceRule*>(rule.get()));
    }
    {
      auto schema = make_per_security_compliance_rule_schema(
        make_buying_power_compliance_rule_schema());
      auto rule = make_compliance_rule(schema, market_data_client,
        definitions_client, time_client);
      REQUIRE(dynamic_cast<PerSecurityComplianceRule*>(rule.get()));
    }
    {
      auto schema = make_region_filter_compliance_rule_schema(
        make_buying_power_compliance_rule_schema());
      auto rule = make_compliance_rule(schema, market_data_client,
        definitions_client, time_client);
      REQUIRE(dynamic_cast<RegionFilterComplianceRule*>(rule.get()));
    }
    {
      auto rule = make_compliance_rule(
        make_reject_cancels_compliance_rule_schema(), market_data_client,
        definitions_client, time_client);
      REQUIRE(dynamic_cast<RejectCancelsComplianceRule*>(rule.get()));
    }
    {
      auto rule = make_compliance_rule(
        make_reject_submissions_compliance_rule_schema(), market_data_client,
        definitions_client, time_client);
      REQUIRE(dynamic_cast<RejectSubmissionsComplianceRule*>(rule.get()));
    }
    {
      auto schema = make_time_filter_compliance_rule_schema(
        make_buying_power_compliance_rule_schema());
      auto rule = make_compliance_rule(schema, market_data_client,
        definitions_client, time_client);
      REQUIRE(dynamic_cast<TimeFilterComplianceRule<FixedTimeClient*>*>(
        rule.get()));
    }
  }
}
