#include <doctest/doctest.h>
#include "Nexus/Compliance/OrderCountLimitComplianceRule.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("OrderCountLimitComplianceRule") {
  TEST_CASE("submit") {
    auto rule = OrderCountLimitComplianceRule(2);
    auto info1 = OrderInfo();
    info1.m_id = 1;
    info1.m_fields.m_ticker = parse_ticker("FOO.TSX");
    info1.m_fields.m_side = Side::BID;
    auto order1 = std::make_shared<PrimitiveOrder>(info1);
    REQUIRE_NOTHROW(rule.submit(order1));
    auto info2 = OrderInfo();
    info2.m_id = 2;
    info2.m_fields.m_ticker = parse_ticker("FOO.TSX");
    info2.m_fields.m_side = Side::ASK;
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    REQUIRE_NOTHROW(rule.submit(order2));
    auto info3 = OrderInfo();
    info3.m_id = 3;
    info3.m_fields.m_ticker = parse_ticker("FOO.TSX");
    info3.m_fields.m_side = Side::BID;
    auto order3 = std::make_shared<PrimitiveOrder>(info3);
    REQUIRE_THROWS_AS(rule.submit(order3), ComplianceCheckException);
    auto report = ExecutionReport();
    report.m_id = 1;
    report.m_status = OrderStatus::FILLED;
    order1->update(report);
    flush_pending_routines();
    auto info4 = OrderInfo();
    info4.m_id = 4;
    info4.m_fields.m_ticker = parse_ticker("FOO.TSX");
    info4.m_fields.m_side = Side::ASK;
    auto order4 = std::make_shared<PrimitiveOrder>(info4);
    REQUIRE_NOTHROW(rule.submit(order4));
  }

  TEST_CASE("add_submit") {
    auto rule = OrderCountLimitComplianceRule(2);
    auto info1 = OrderInfo();
    info1.m_id = 1;
    info1.m_fields.m_ticker = parse_ticker("BAZ.TSX");
    info1.m_fields.m_side = Side::BID;
    auto order1 = std::make_shared<PrimitiveOrder>(info1);
    REQUIRE_NOTHROW(rule.add(order1));
    auto info2 = OrderInfo();
    info2.m_id = 2;
    info2.m_fields.m_ticker = parse_ticker("BAZ.TSX");
    info2.m_fields.m_side = Side::ASK;
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    REQUIRE_NOTHROW(rule.add(order2));
    auto info3 = OrderInfo();
    info3.m_id = 3;
    info3.m_fields.m_ticker = parse_ticker("BAZ.TSX");
    info3.m_fields.m_side = Side::BID;
    auto order3 = std::make_shared<PrimitiveOrder>(info3);
    REQUIRE_THROWS_AS(rule.submit(order3), ComplianceCheckException);
  }
}
