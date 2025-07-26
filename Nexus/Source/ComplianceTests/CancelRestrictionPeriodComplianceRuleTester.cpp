#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/CancelRestrictionPeriodComplianceRule.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::OrderExecutionService;

namespace {
  using TestCancelRestrictionPeriodComplianceRule =
    CancelRestrictionPeriodComplianceRule<TimeClientBox>;
}

TEST_SUITE("CancelRestrictionPeriodComplianceRule") {
  TEST_CASE("allows_cancel_outside_restricted_period") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-25 13:59:00"));
    auto security = Security("TST", NYSE);
    auto order_info = OrderInfo();
    order_info.m_fields = make_limit_order_fields(
      DirectoryEntry::MakeAccount(1, "alice"), security, USD, Side::BID, "NYSE",
      100, Money::ONE);
    order_info.m_timestamp = time_client.GetTime();
    order_info.m_id = 1;
    auto order = std::make_shared<PrimitiveOrder>(order_info);
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("start", hours(10));
    parameters.emplace_back("end", hours(14));
    auto rule = TestCancelRestrictionPeriodComplianceRule(parameters,
      get_default_time_zone_database(), DEFAULT_VENUES, &time_client);
    REQUIRE_NOTHROW(rule.cancel(order));
  }

  TEST_CASE("rejects_cancel_inside_restricted_period") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-25 16:00:00"));
    auto security = Security("TST", NYSE);
    auto order_info = OrderInfo();
    order_info.m_fields = make_limit_order_fields(
      DirectoryEntry::MakeAccount(1, "alice"), security, USD, Side::BID, "NYSE",
      100, Money::ONE);
    order_info.m_timestamp = time_client.GetTime();
    order_info.m_id = 2;
    auto order = std::make_shared<PrimitiveOrder>(order_info);
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("start", hours(10));
    parameters.emplace_back("end", hours(14));
    auto rule = TestCancelRestrictionPeriodComplianceRule(parameters,
      get_default_time_zone_database(), DEFAULT_VENUES, &time_client);
    REQUIRE_THROWS_AS(rule.cancel(order), ComplianceCheckException);
  }

  TEST_CASE("rejects_cancel_inside_overnight_period") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-25 03:00:00"));
    auto security = Security("TST", NYSE);
    auto order_info = OrderInfo();
    order_info.m_fields = make_limit_order_fields(
      DirectoryEntry::MakeAccount(1, "alice"), security, USD, Side::BID, "NYSE",
      100, Money::ONE);
    order_info.m_timestamp = time_client.GetTime();
    order_info.m_id = 3;
    auto order = std::make_shared<PrimitiveOrder>(order_info);
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("start", hours(22));
    parameters.emplace_back("end", hours(2));
    auto rule = TestCancelRestrictionPeriodComplianceRule(parameters,
      get_default_time_zone_database(), DEFAULT_VENUES, &time_client);
    REQUIRE_THROWS_AS(rule.cancel(order), ComplianceCheckException);
    time_client.SetTime(time_from_string("2024-07-25 07:00:00"));
    REQUIRE_NOTHROW(rule.cancel(order));
  }

  TEST_CASE("correctly_converts_time_zones") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-25 15:00:00"));
    auto security = Security("TST", NYSE);
    auto order_info = OrderInfo();
    order_info.m_fields = make_limit_order_fields(
      DirectoryEntry::MakeAccount(1, "alice"), security, USD, Side::BID, "NYSE",
      100, Money::ONE);
    order_info.m_timestamp = time_client.GetTime();
    order_info.m_id = 4;
    auto order = std::make_shared<PrimitiveOrder>(order_info);
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("start", hours(10));
    parameters.emplace_back("end", hours(14));
    auto rule = TestCancelRestrictionPeriodComplianceRule(parameters,
      get_default_time_zone_database(), DEFAULT_VENUES, &time_client);
    REQUIRE_THROWS_AS(rule.cancel(order), ComplianceCheckException);
    time_client.SetTime(time_from_string("2024-07-25 13:00:00"));
    REQUIRE_NOTHROW(rule.cancel(order));
  }

  TEST_CASE("edge_case_start_equals_end") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-25 14:00:00")); // 10:00 NY time (EDT)
    auto security = Security("TST", NYSE);
    auto order_info = OrderInfo();
    order_info.m_fields = make_limit_order_fields(
      DirectoryEntry::MakeAccount(1, "alice"), security, USD, Side::BID, "NYSE",
      100, Money::ONE);
    order_info.m_timestamp = time_client.GetTime();
    order_info.m_id = 5;
    auto order = std::make_shared<PrimitiveOrder>(order_info);
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("start", hours(10));
    parameters.emplace_back("end", hours(10));
    auto rule = TestCancelRestrictionPeriodComplianceRule(parameters,
      get_default_time_zone_database(), DEFAULT_VENUES, &time_client);
    REQUIRE_THROWS_AS(rule.cancel(order), ComplianceCheckException);
    time_client.SetTime(time_from_string("2024-07-25 14:01:00"));
    REQUIRE_NOTHROW(rule.cancel(order));
  }

  TEST_CASE("throws_if_venue_not_in_database") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-25 16:00:00"));
    auto unknown_venue = Venue("XXXX");
    auto security = Security("TST", unknown_venue);
    auto order_info = OrderInfo();
    order_info.m_fields = make_limit_order_fields(
      DirectoryEntry::MakeAccount(1, "alice"), security, USD, Side::BID, "XXXX",
      100, Money::ONE);
    order_info.m_timestamp = time_client.GetTime();
    order_info.m_id = 6;
    auto order = std::make_shared<PrimitiveOrder>(order_info);
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("start", hours(10));
    parameters.emplace_back("end", hours(14));
    auto rule = TestCancelRestrictionPeriodComplianceRule(parameters,
      get_default_time_zone_database(), DEFAULT_VENUES, &time_client);
    REQUIRE_THROWS_AS(rule.cancel(order), ComplianceCheckException);
  }
}
