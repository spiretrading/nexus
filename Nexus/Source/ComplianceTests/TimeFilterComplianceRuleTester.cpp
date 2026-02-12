#include <future>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/TimeFilterComplianceRule.hpp"
#include "Nexus/ComplianceTests/TestComplianceRule.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

namespace {
  void require_check(auto& rule, auto& time_client, auto& operations) {
    auto ticker = parse_ticker("TST.TSX");
    auto order_info = OrderInfo();
    order_info.m_fields = make_limit_order_fields(
      DirectoryEntry::make_account(1, "alice"), ticker, CAD, Side::BID, "TSX",
      100, Money::ONE);
    order_info.m_timestamp = time_client.get_time();
    order_info.m_id = 1;
    auto order = std::make_shared<PrimitiveOrder>(order_info);
    auto async_submit = std::async(std::launch::async, [&] {
      rule.submit(order);
    });
    auto operation = operations->pop();
    auto submit_operation =
      std::get_if<TestComplianceRule::SubmitOperation>(&*operation);
    REQUIRE(submit_operation);
    REQUIRE(submit_operation->m_order == order);
    submit_operation->m_result.set();
    async_submit.get();
    auto async_cancel = std::async(std::launch::async, [&] {
      rule.cancel(order);
    });
    operation = operations->pop();
    auto cancel_operation =
      std::get_if<TestComplianceRule::CancelOperation>(&*operation);
    REQUIRE(cancel_operation);
    REQUIRE(cancel_operation->m_order == order);
    cancel_operation->m_result.set();
    async_cancel.get();
  }

  void require_passthrough(auto& rule, auto& time_client, auto& operations) {
    auto ticker = parse_ticker("TST.TSX");
    auto order_info = OrderInfo();
    order_info.m_fields = make_limit_order_fields(
      DirectoryEntry::make_account(1, "alice"), ticker, CAD, Side::BID, "TSX",
      100, Money::ONE);
    order_info.m_timestamp = time_client.get_time();
    order_info.m_id = 1;
    auto order = std::make_shared<PrimitiveOrder>(order_info);
    auto async_submit = std::async(std::launch::async, [&] {
      rule.submit(order);
    });
    auto operation = operations->pop();
    auto add_operation =
      std::get_if<TestComplianceRule::AddOperation>(&*operation);
    REQUIRE(add_operation);
    REQUIRE(add_operation->m_order == order);
    add_operation->m_result.set();
    async_submit.get();
    auto async_cancel = std::async(std::launch::async, [&] {
      rule.cancel(order);
    });
    async_cancel.get();
    REQUIRE(!operations->try_pop());
  }
}

TEST_SUITE("TimeFilterComplianceRule") {
  TEST_CASE("same_day") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-25 16:00:00"));
    auto operations = std::make_shared<TestComplianceRule::Queue>();
    auto rule = TimeFilterComplianceRule(hours(10), hours(14),
      get_default_time_zone_database(), DEFAULT_VENUES, &time_client,
      std::make_unique<TestComplianceRule>(operations));
    SUBCASE("inside") {
      require_check(rule, time_client, operations);
    }
    SUBCASE("outside") {
      time_client.set(time_from_string("2024-07-25 13:59:00"));
      require_passthrough(rule, time_client, operations);
    }
  }

  TEST_CASE("overnight_period") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-25 03:00:00"));
    auto operations = std::make_shared<TestComplianceRule::Queue>();
    auto rule = TimeFilterComplianceRule(hours(22), hours(2),
      get_default_time_zone_database(), DEFAULT_VENUES, &time_client,
      std::make_unique<TestComplianceRule>(operations));
    SUBCASE("inside") {
      require_check(rule, time_client, operations);
    }
    SUBCASE("outside") {
      time_client.set(time_from_string("2024-07-25 07:00:00"));
      require_passthrough(rule, time_client, operations);
    }
  }

  TEST_CASE("convert_time_zones") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-25 15:00:00"));
    auto operations = std::make_shared<TestComplianceRule::Queue>();
    auto rule = TimeFilterComplianceRule(hours(10), hours(14),
      get_default_time_zone_database(), DEFAULT_VENUES, &time_client,
      std::make_unique<TestComplianceRule>(operations));
    require_check(rule, time_client, operations);
    time_client.set(time_from_string("2024-07-25 13:00:00"));
    require_passthrough(rule, time_client, operations);
  }

  TEST_CASE("edge_case_start_equals_end") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-25 14:00:00"));
    auto operations = std::make_shared<TestComplianceRule::Queue>();
    auto rule = TimeFilterComplianceRule(hours(10), hours(10),
      get_default_time_zone_database(), DEFAULT_VENUES, &time_client,
      std::make_unique<TestComplianceRule>(operations));
    require_check(rule, time_client, operations);
    time_client.set(time_from_string("2024-07-25 14:01:00"));
    require_passthrough(rule, time_client, operations);
  }

  TEST_CASE("throws_if_venue_not_in_database") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-25 16:00:00"));
    auto operations = std::make_shared<TestComplianceRule::Queue>();
    auto rule = TimeFilterComplianceRule(hours(10), hours(10),
      get_default_time_zone_database(), DEFAULT_VENUES, &time_client,
      std::make_unique<TestComplianceRule>(operations));
    auto unknown_venue = Venue("XXXX");
    auto ticker = parse_ticker("TST.XXXX");
    auto order_info = OrderInfo();
    order_info.m_fields = make_limit_order_fields(
      DirectoryEntry::make_account(1, "alice"), ticker, CAD, Side::BID,
      "XXXX", 100, Money::ONE);
    order_info.m_timestamp = time_client.get_time();
    order_info.m_id = 6;
    auto order = std::make_shared<PrimitiveOrder>(order_info);
    REQUIRE_THROWS_AS(rule.submit(order), ComplianceCheckException);
    REQUIRE_THROWS_AS(rule.cancel(order), ComplianceCheckException);
  }
}
