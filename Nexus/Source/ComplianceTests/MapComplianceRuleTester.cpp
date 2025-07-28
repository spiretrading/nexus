#include <future>
#include <doctest/doctest.h>
#include "Nexus/Compliance/MapComplianceRule.hpp"
#include "Nexus/ComplianceTests/TestComplianceRule.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::Compliance::Tests;
using namespace Nexus::DefaultVenues;
using namespace Nexus::OrderExecutionService;

TEST_SUITE("map_compliance_rule") {
  TEST_CASE("submit") {
    auto rule_operations =
      std::make_shared<Queue<std::shared_ptr<TestComplianceRule::Queue>>>();
    auto rule_builder = [&] (const ComplianceRuleSchema&) {
      auto queue = std::make_shared<TestComplianceRule::Queue>();
      rule_operations->Push(queue);
      return std::make_unique<TestComplianceRule>(queue);
    };
    auto key_builder = [] (const Order& order) {
      return order.get_info().m_fields.m_security;
    };
    auto schema = ComplianceRuleSchema("test_rule", {});
    auto rule = MapComplianceRule<Security>(schema, rule_builder, key_builder);
    auto info = OrderInfo();
    info.m_id = 42;
    info.m_fields.m_security = Security("FOO", TSX);
    auto order = std::make_shared<PrimitiveOrder>(info);
    auto async_submit = std::async(std::launch::async, [&] {
      rule.submit(order);
    });
    auto operations = rule_operations->Pop();
    auto operation = operations->Pop();
    auto submit_operation =
      std::get_if<TestComplianceRule::SubmitOperation>(&*operation);
    REQUIRE(submit_operation);
    REQUIRE(submit_operation->m_order == order);
    submit_operation->m_result.SetResult();
    auto info2 = OrderInfo();
    info2.m_id = 43;
    info2.m_fields.m_security = Security("FOO", TSX);
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    auto async_submit2 = std::async(std::launch::async, [&] {
      rule.submit(order2);
    });
    operation = operations->Pop();
    submit_operation =
      std::get_if<TestComplianceRule::SubmitOperation>(&*operation);
    REQUIRE(submit_operation);
    REQUIRE(submit_operation->m_order == order2);
    submit_operation->m_result.SetResult();
  }

  TEST_CASE("add") {
    auto rule_operations =
      std::make_shared<Queue<std::shared_ptr<TestComplianceRule::Queue>>>();
    auto rule_builder = [&] (const ComplianceRuleSchema&) {
      auto queue = std::make_shared<TestComplianceRule::Queue>();
      rule_operations->Push(queue);
      return std::make_unique<TestComplianceRule>(queue);
    };
    auto key_builder = [] (const Order& order) {
      return order.get_info().m_fields.m_security;
    };
    auto schema = ComplianceRuleSchema("test_rule", {});
    auto rule = MapComplianceRule<Security>(schema, rule_builder, key_builder);
    auto info1 = OrderInfo();
    info1.m_id = 100;
    info1.m_fields.m_security = Security("BAR", TSX);
    auto order1 = std::make_shared<PrimitiveOrder>(info1);
    auto async_add1 = std::async(std::launch::async, [&] {
      rule.add(order1);
    });
    auto operations1 = rule_operations->Pop();
    auto operation1 = operations1->Pop();
    auto add_operation1 =
      std::get_if<TestComplianceRule::AddOperation>(&*operation1);
    REQUIRE(add_operation1);
    REQUIRE(add_operation1->m_order == order1);
    add_operation1->m_result.SetResult();
    auto info2 = OrderInfo();
    info2.m_id = 101;
    info2.m_fields.m_security = Security("BAR", TSX);
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    auto async_add2 = std::async(std::launch::async, [&] {
      rule.add(order2);
    });
    auto operation2 = operations1->Pop();
    auto add_operation2 =
      std::get_if<TestComplianceRule::AddOperation>(&*operation2);
    REQUIRE(add_operation2);
    REQUIRE(add_operation2->m_order == order2);
    add_operation2->m_result.SetResult();
  }
}
