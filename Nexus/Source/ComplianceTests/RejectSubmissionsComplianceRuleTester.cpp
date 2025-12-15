#include <doctest/doctest.h>
#include "Nexus/Compliance/RejectSubmissionsComplianceRule.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Nexus;

TEST_SUITE("RejectSubmissionsComplianceRule") {
  TEST_CASE("submit_throws_with_default_message") {
    auto rule = RejectSubmissionsComplianceRule();
    auto info = OrderInfo();
    info.m_id = 1;
    auto order = std::make_shared<PrimitiveOrder>(info);
    REQUIRE_THROWS_AS(rule.submit(order), ComplianceCheckException);
    REQUIRE_THROWS_WITH(rule.submit(order), "Submissions not allowed.");
  }

  TEST_CASE("submit_throws_with_custom_message") {
    auto rule = RejectSubmissionsComplianceRule("Custom rejection.");
    auto info = OrderInfo();
    info.m_id = 2;
    auto order = std::make_shared<PrimitiveOrder>(info);
    REQUIRE_THROWS_AS(rule.submit(order), ComplianceCheckException);
    REQUIRE_THROWS_WITH(rule.submit(order), "Custom rejection.");
  }
}
