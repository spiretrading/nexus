#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/OpposingCancelComplianceRule.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("OpposingCancelComplianceRule") {
  TEST_CASE("cancel_without_fill") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-29 10:00:00"));
    auto rule = OpposingCancelComplianceRule(seconds(10), &time_client);
    auto fields =
      make_limit_order_fields(Security("ABX", TSX), Side::BID, 100, Money::ONE);
    auto info = OrderInfo(fields, 3, time_from_string("2024-07-29 10:00:00"));
    auto order = std::make_shared<PrimitiveOrder>(info);
    REQUIRE_NOTHROW(rule.cancel(order));
  }

  TEST_CASE("cancel_after_fill_within_timeout") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-29 09:59:50"));
    auto rule = OpposingCancelComplianceRule(seconds(10), &time_client);
    auto ask_fields =
      make_limit_order_fields(Security("ABX", TSX), Side::ASK, 100, Money::ONE);
    auto ask_info =
      OrderInfo(ask_fields, 2, time_from_string("2024-07-29 09:59:50"));
    auto ask_order = std::make_shared<PrimitiveOrder>(ask_info);
    rule.add(ask_order);
    ask_order->with([&] (auto status, const auto& reports) {
      auto report = make_update(reports.back(), OrderStatus::FILLED,
        time_from_string("2024-07-29 09:59:59"));
      report.m_last_quantity = 100;
      ask_order->update(report);
    });
    auto bid_fields =
      make_limit_order_fields(Security("ABX", TSX), Side::BID, 100, Money::ONE);
    auto bid_info =
      OrderInfo(bid_fields, 3, time_from_string("2024-07-29 09:59:55"));
    auto bid_order = std::make_shared<PrimitiveOrder>(bid_info);
    rule.add(bid_order);
    time_client.set(time_from_string("2024-07-29 10:00:00"));
    REQUIRE_THROWS_AS(rule.cancel(bid_order), ComplianceCheckException);
    REQUIRE_NOTHROW(rule.cancel(ask_order));
    time_client.set(time_from_string("2024-07-29 10:00:10"));
    REQUIRE_NOTHROW(rule.cancel(bid_order));
  }

  TEST_CASE("multiple_fills_updates_last_fill_time") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-29 09:59:50"));
    auto rule = OpposingCancelComplianceRule(seconds(10), &time_client);
    auto ask_fields =
      make_limit_order_fields(Security("ABX", TSX), Side::ASK, 100, Money::ONE);
    auto ask_info =
      OrderInfo(ask_fields, 2, time_from_string("2024-07-29 09:59:50"));
    auto ask_order = std::make_shared<PrimitiveOrder>(ask_info);
    rule.add(ask_order);
    ask_order->with([&] (auto status, const auto& reports) {
      auto report1 = make_update(reports.back(), OrderStatus::PARTIALLY_FILLED,
        time_from_string("2024-07-29 09:59:55"));
      report1.m_last_quantity = 50;
      ask_order->update(report1);
      auto report2 = make_update(report1, OrderStatus::PARTIALLY_FILLED,
        time_from_string("2024-07-29 09:59:58"));
      report2.m_last_quantity = 50;
      ask_order->update(report2);
    });
    auto bid_fields =
      make_limit_order_fields(Security("ABX", TSX), Side::BID, 100, Money::ONE);
    auto bid_info =
      OrderInfo(bid_fields, 3, time_from_string("2024-07-29 09:59:55"));
    auto bid_order = std::make_shared<PrimitiveOrder>(bid_info);
    rule.add(bid_order);
    time_client.set(time_from_string("2024-07-29 10:00:05"));
    REQUIRE_THROWS_AS(rule.cancel(bid_order), ComplianceCheckException);
    time_client.set(time_from_string("2024-07-29 10:00:08"));
    REQUIRE_THROWS_AS(rule.cancel(bid_order), ComplianceCheckException);
    time_client.set(time_from_string("2024-07-29 10:00:10"));
    REQUIRE_NOTHROW(rule.cancel(bid_order));
  }
}
