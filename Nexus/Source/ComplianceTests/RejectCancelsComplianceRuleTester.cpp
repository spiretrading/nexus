#include <doctest/doctest.h>
#include "Nexus/Compliance/RejectCancelsComplianceRule.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Nexus;

TEST_SUITE("RejectCancelsComplianceRule") {
  TEST_CASE("cancel_throws_with_default_message") {
    auto rule = RejectCancelsComplianceRule();
    auto info = OrderInfo();
    info.m_id = 1;
    auto order = std::make_shared<PrimitiveOrder>(info);
    REQUIRE_THROWS_AS(rule.cancel(order), ComplianceCheckException);
    REQUIRE_THROWS_WITH(rule.cancel(order), "Cancels not allowed.");
  }

  TEST_CASE("cancel_throws_with_custom_message") {
    auto rule = RejectCancelsComplianceRule("Custom cancel rejection.");
    auto info = OrderInfo();
    info.m_id = 2;
    auto order = std::make_shared<PrimitiveOrder>(info);
    REQUIRE_THROWS_AS(rule.cancel(order), ComplianceCheckException);
    REQUIRE_THROWS_WITH(rule.cancel(order), "Custom cancel rejection.");
  }
}
