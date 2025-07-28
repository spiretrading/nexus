#include <future>
#include <doctest/doctest.h>
#include "Nexus/Compliance/RegionFilterComplianceRule.hpp"
#include "Nexus/ComplianceTests/TestComplianceRule.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::Compliance::Tests;
using namespace Nexus::DefaultVenues;
using namespace Nexus::OrderExecutionService;

TEST_SUITE("RegionFilterComplianceRule") {
  TEST_CASE("submit") {
    auto security = Security("FOO", TSX);
    auto region = Region(security);
    auto rule_queue = std::make_shared<TestComplianceRule::Queue>();
    auto region_rule = RegionFilterComplianceRule(
      region, std::make_unique<TestComplianceRule>(rule_queue));
    auto info = OrderInfo();
    info.m_id = 1;
    info.m_fields.m_security = security;
    auto order = std::make_shared<PrimitiveOrder>(info);
    auto async_submit = std::async(std::launch::async, [&] {
      region_rule.submit(order);
    });
    auto operation = rule_queue->Pop();
    auto submit_operation =
      std::get_if<TestComplianceRule::SubmitOperation>(&*operation);
    REQUIRE(submit_operation);
    REQUIRE(submit_operation->m_order == order);
    submit_operation->m_result.SetResult();
    async_submit.get();
    auto info2 = OrderInfo();
    info2.m_id = 2;
    info2.m_fields.m_security = Security("BAR", TSX);
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    auto async_submit2 = std::async(std::launch::async, [&] {
      region_rule.submit(order2);
    });
    auto operation2 = rule_queue->Pop();
    auto add_operation =
      std::get_if<TestComplianceRule::AddOperation>(&*operation2);
    REQUIRE(add_operation);
    REQUIRE(add_operation->m_order == order2);
    add_operation->m_result.SetResult();
    async_submit2.get();
  }

  TEST_CASE("cancel") {
    auto security = Security("FOO", TSX);
    auto region = Region(security);
    auto rule_queue = std::make_shared<TestComplianceRule::Queue>();
    auto region_rule = RegionFilterComplianceRule(
      region, std::make_unique<TestComplianceRule>(rule_queue));
    auto info = OrderInfo();
    info.m_id = 3;
    info.m_fields.m_security = security;
    auto order = std::make_shared<PrimitiveOrder>(info);
    auto async_cancel = std::async(std::launch::async, [&] {
      region_rule.cancel(order);
    });
    auto operation = rule_queue->Pop();
    auto cancel_operation =
      std::get_if<TestComplianceRule::CancelOperation>(&*operation);
    REQUIRE(cancel_operation);
    REQUIRE(cancel_operation->m_order == order);
    cancel_operation->m_result.SetResult();
    async_cancel.get();
    auto info2 = OrderInfo();
    info2.m_id = 4;
    info2.m_fields.m_security = Security("BAR", TSX);
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    region_rule.cancel(order2);
    REQUIRE(!rule_queue->TryPop());
  }

  TEST_CASE("add") {
    auto security1 = Security("FOO", TSX);
    auto security2 = Security("BAR", TSX);
    auto region = Region(security1);
    auto rule_queue = std::make_shared<TestComplianceRule::Queue>();
    auto region_rule = RegionFilterComplianceRule(
      region, std::make_unique<TestComplianceRule>(rule_queue));
    auto info1 = OrderInfo();
    info1.m_id = 5;
    info1.m_fields.m_security = security1;
    auto order1 = std::make_shared<PrimitiveOrder>(info1);
    auto async_add = std::async(std::launch::async, [&] {
      region_rule.add(order1);
    });
    auto operation1 = rule_queue->Pop();
    auto add_operation1 =
      std::get_if<TestComplianceRule::AddOperation>(&*operation1);
    REQUIRE(add_operation1);
    REQUIRE(add_operation1->m_order == order1);
    add_operation1->m_result.SetResult();
    async_add.get();
    auto info2 = OrderInfo();
    info2.m_id = 6;
    info2.m_fields.m_security = security2;
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    auto async_add2 = std::async(std::launch::async, [&] {
      region_rule.add(order2);
    });
    auto operation2 = rule_queue->Pop();
    auto add_operation2 =
      std::get_if<TestComplianceRule::AddOperation>(&*operation2);
    REQUIRE(add_operation2);
    REQUIRE(add_operation2->m_order == order2);
    add_operation2->m_result.SetResult();
    async_add2.get();
    REQUIRE(!rule_queue->TryPop());
  }
}
