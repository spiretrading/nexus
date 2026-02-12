#include <future>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ScopeFilterComplianceRule.hpp"
#include "Nexus/ComplianceTests/TestComplianceRule.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Nexus;
using namespace Nexus::Tests;

TEST_SUITE("ScopeFilterComplianceRule") {
  TEST_CASE("submit") {
    auto ticker = parse_ticker("FOO.TSX");
    auto scope = Scope(ticker);
    auto rule_queue = std::make_shared<TestComplianceRule::Queue>();
    auto scope_rule = ScopeFilterComplianceRule(
      scope, std::make_unique<TestComplianceRule>(rule_queue));
    auto info = OrderInfo();
    info.m_id = 1;
    info.m_fields.m_ticker = ticker;
    auto order = std::make_shared<PrimitiveOrder>(info);
    auto async_submit = std::async(std::launch::async, [&] {
      scope_rule.submit(order);
    });
    auto operation = rule_queue->pop();
    auto submit_operation =
      std::get_if<TestComplianceRule::SubmitOperation>(&*operation);
    REQUIRE(submit_operation);
    REQUIRE(submit_operation->m_order == order);
    submit_operation->m_result.set();
    async_submit.get();
    auto info2 = OrderInfo();
    info2.m_id = 2;
    info2.m_fields.m_ticker = parse_ticker("BAR.TSX");
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    auto async_submit2 = std::async(std::launch::async, [&] {
      scope_rule.submit(order2);
    });
    auto operation2 = rule_queue->pop();
    auto add_operation =
      std::get_if<TestComplianceRule::AddOperation>(&*operation2);
    REQUIRE(add_operation);
    REQUIRE(add_operation->m_order == order2);
    add_operation->m_result.set();
    async_submit2.get();
  }

  TEST_CASE("cancel") {
    auto ticker = parse_ticker("FOO.TSX");
    auto scope = Scope(ticker);
    auto rule_queue = std::make_shared<TestComplianceRule::Queue>();
    auto scope_rule = ScopeFilterComplianceRule(
      scope, std::make_unique<TestComplianceRule>(rule_queue));
    auto info = OrderInfo();
    info.m_id = 3;
    info.m_fields.m_ticker = ticker;
    auto order = std::make_shared<PrimitiveOrder>(info);
    auto async_cancel = std::async(std::launch::async, [&] {
      scope_rule.cancel(order);
    });
    auto operation = rule_queue->pop();
    auto cancel_operation =
      std::get_if<TestComplianceRule::CancelOperation>(&*operation);
    REQUIRE(cancel_operation);
    REQUIRE(cancel_operation->m_order == order);
    cancel_operation->m_result.set();
    async_cancel.get();
    auto info2 = OrderInfo();
    info2.m_id = 4;
    info2.m_fields.m_ticker = parse_ticker("BAR.TSX");
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    scope_rule.cancel(order2);
    REQUIRE(!rule_queue->try_pop());
  }

  TEST_CASE("add") {
    auto ticker1 = parse_ticker("FOO.TSX");
    auto ticker2 = parse_ticker("BAR.TSX");
    auto scope = Scope(ticker1);
    auto rule_queue = std::make_shared<TestComplianceRule::Queue>();
    auto scope_rule = ScopeFilterComplianceRule(
      scope, std::make_unique<TestComplianceRule>(rule_queue));
    auto info1 = OrderInfo();
    info1.m_id = 5;
    info1.m_fields.m_ticker = ticker1;
    auto order1 = std::make_shared<PrimitiveOrder>(info1);
    auto async_add = std::async(std::launch::async, [&] {
      scope_rule.add(order1);
    });
    auto operation1 = rule_queue->pop();
    auto add_operation1 =
      std::get_if<TestComplianceRule::AddOperation>(&*operation1);
    REQUIRE(add_operation1);
    REQUIRE(add_operation1->m_order == order1);
    add_operation1->m_result.set();
    async_add.get();
    auto info2 = OrderInfo();
    info2.m_id = 6;
    info2.m_fields.m_ticker = ticker2;
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    auto async_add2 = std::async(std::launch::async, [&] {
      scope_rule.add(order2);
    });
    auto operation2 = rule_queue->pop();
    auto add_operation2 =
      std::get_if<TestComplianceRule::AddOperation>(&*operation2);
    REQUIRE(add_operation2);
    REQUIRE(add_operation2->m_order == order2);
    add_operation2->m_result.set();
    async_add2.get();
    REQUIRE(!rule_queue->try_pop());
  }
}
