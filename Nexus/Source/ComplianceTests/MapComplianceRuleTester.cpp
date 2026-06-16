#include <future>
#include <doctest/doctest.h>
#include "Nexus/Compliance/MapComplianceRule.hpp"
#include "Nexus/ComplianceTests/TestComplianceRule.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace Nexus::Venues;

TEST_SUITE("MapComplianceRule") {
  TEST_CASE("submit") {
    auto rule_operations =
      std::make_shared<Queue<std::shared_ptr<TestComplianceRule::Queue>>>();
    auto rule_builder = [&] (const ComplianceRuleSchema&) {
      auto queue = std::make_shared<TestComplianceRule::Queue>();
      rule_operations->push(queue);
      return std::make_unique<TestComplianceRule>(queue);
    };
    auto key_builder = [] (const Order& order) {
      return order.get_info().m_fields.m_ticker;
    };
    auto schema = ComplianceRuleSchema("test_rule", {});
    auto rule = MapComplianceRule(schema, rule_builder, key_builder);
    auto info = OrderInfo();
    info.m_id = 42;
    info.m_fields.m_ticker = parse_ticker("FOO.TSX");
    auto order = std::make_shared<PrimitiveOrder>(info);
    auto async_submit = std::async(std::launch::async, [&] {
      rule.submit(order);
    });
    auto operations = rule_operations->pop();
    auto operation = operations->pop();
    auto submit_operation =
      std::get_if<TestComplianceRule::SubmitOperation>(&*operation);
    REQUIRE(submit_operation);
    REQUIRE(submit_operation->m_order == order);
    submit_operation->m_result.set();
    auto info2 = OrderInfo();
    info2.m_id = 43;
    info2.m_fields.m_ticker = parse_ticker("FOO.TSX");
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    auto async_submit2 = std::async(std::launch::async, [&] {
      rule.submit(order2);
    });
    operation = operations->pop();
    submit_operation =
      std::get_if<TestComplianceRule::SubmitOperation>(&*operation);
    REQUIRE(submit_operation);
    REQUIRE(submit_operation->m_order == order2);
    submit_operation->m_result.set();
  }

  TEST_CASE("add") {
    auto rule_operations =
      std::make_shared<Queue<std::shared_ptr<TestComplianceRule::Queue>>>();
    auto rule_builder = [&] (const ComplianceRuleSchema&) {
      auto queue = std::make_shared<TestComplianceRule::Queue>();
      rule_operations->push(queue);
      return std::make_unique<TestComplianceRule>(queue);
    };
    auto key_builder = [] (const Order& order) {
      return order.get_info().m_fields.m_ticker;
    };
    auto schema = ComplianceRuleSchema("test_rule", {});
    auto rule = MapComplianceRule(schema, rule_builder, key_builder);
    auto info1 = OrderInfo();
    info1.m_id = 100;
    info1.m_fields.m_ticker = parse_ticker("BAR.TSX");
    auto order1 = std::make_shared<PrimitiveOrder>(info1);
    auto async_add1 = std::async(std::launch::async, [&] {
      rule.add(order1);
    });
    auto operations1 = rule_operations->pop();
    auto operation1 = operations1->pop();
    auto add_operation1 =
      std::get_if<TestComplianceRule::AddOperation>(&*operation1);
    REQUIRE(add_operation1);
    REQUIRE(add_operation1->m_order == order1);
    add_operation1->m_result.set();
    auto info2 = OrderInfo();
    info2.m_id = 101;
    info2.m_fields.m_ticker = parse_ticker("BAR.TSX");
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    auto async_add2 = std::async(std::launch::async, [&] {
      rule.add(order2);
    });
    auto operation2 = operations1->pop();
    auto add_operation2 =
      std::get_if<TestComplianceRule::AddOperation>(&*operation2);
    REQUIRE(add_operation2);
    REQUIRE(add_operation2->m_order == order2);
    add_operation2->m_result.set();
  }

  TEST_CASE("restore") {
    auto rule_operations =
      std::make_shared<Queue<std::shared_ptr<TestComplianceRule::Queue>>>();
    auto rule_builder = [&] (const ComplianceRuleSchema&) {
      auto queue = std::make_shared<TestComplianceRule::Queue>();
      rule_operations->push(queue);
      return std::make_unique<TestComplianceRule>(queue);
    };
    auto key_builder = [] (const Order& order) {
      return order.get_info().m_fields.m_ticker;
    };
    auto schema = ComplianceRuleSchema("test_rule", {});
    auto rule = MapComplianceRule(schema, rule_builder, key_builder);
    auto account = DirectoryEntry::make_account(1, "test");
    auto info = OrderInfo();
    info.m_id = 200;
    info.m_fields.m_ticker = parse_ticker("FOO.TSX");
    auto order = std::make_shared<PrimitiveOrder>(info);
    auto snapshot = InventorySnapshot();
    snapshot.m_sequence = Beam::Sequence(5);
    auto async_restore = std::async(std::launch::async, [&] {
      rule.restore(account, snapshot,
        std::vector<std::shared_ptr<Order>>{order});
    });
    auto operations = rule_operations->pop();
    auto operation = operations->pop();
    auto restore_operation =
      std::get_if<TestComplianceRule::RestoreOperation>(&*operation);
    REQUIRE(restore_operation);
    REQUIRE(restore_operation->m_account == account);
    REQUIRE(restore_operation->m_snapshot == snapshot);
    REQUIRE(restore_operation->m_orders.size() == 1);
    REQUIRE(restore_operation->m_orders[0] == order);
    restore_operation->m_result.set();
    async_restore.get();
  }
}
