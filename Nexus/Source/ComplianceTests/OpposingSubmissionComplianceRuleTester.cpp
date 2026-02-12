#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/OpposingSubmissionComplianceRule.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("OpposingSubmissionComplianceRule") {
  TEST_CASE("submit_without_cancel") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-29 10:00:00"));
    auto rule =
      OpposingSubmissionComplianceRule(seconds(10), Money::ZERO, &time_client);
    auto fields = make_limit_order_fields(
      parse_ticker("ABX.TSX"), Side::BID, 100, Money::ONE);
    auto info = OrderInfo(fields, 3, time_from_string("2024-07-29 10:00:00"));
    auto order = std::make_shared<PrimitiveOrder>(info);
    REQUIRE_NOTHROW(rule.submit(order));
  }

  TEST_CASE("submit_after_cancel_within_and_outside_timeout") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-29 09:59:50"));
    auto rule =
      OpposingSubmissionComplianceRule(seconds(10), Money::ZERO, &time_client);
    auto ask_fields = make_limit_order_fields(
      parse_ticker("ABX.TSX"), Side::ASK, 100, Money::ONE);
    auto ask_info =
      OrderInfo(ask_fields, 1, time_from_string("2024-07-29 09:59:50"));
    auto ask_order = std::make_shared<PrimitiveOrder>(ask_info);
    rule.add(ask_order);
    ask_order->with([&] (auto status, const auto& reports) {
      auto cancel_report = make_update(reports.back(), OrderStatus::CANCELED,
        time_from_string("2024-07-29 09:59:59"));
      ask_order->update(cancel_report);
    });
    auto bid_fields = make_limit_order_fields(
      parse_ticker("ABX.TSX"), Side::BID, 100, Money::ONE);
    auto bid_info =
      OrderInfo(bid_fields, 2, time_from_string("2024-07-29 10:00:00"));
    auto bid_order = std::make_shared<PrimitiveOrder>(bid_info);
    rule.add(bid_order);
    time_client.set(time_from_string("2024-07-29 10:00:05"));
    REQUIRE_THROWS_AS(rule.submit(bid_order), ComplianceCheckException);
    time_client.set(time_from_string("2024-07-29 10:00:10"));
    REQUIRE_NOTHROW(rule.submit(bid_order));
  }

  TEST_CASE("submit_outside_price_range") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-29 09:59:50"));
    auto rule = OpposingSubmissionComplianceRule(
      seconds(10), 5 * Money::CENT, &time_client);
    auto ask_fields = make_limit_order_fields(
      parse_ticker("ABX.TSX"), Side::ASK, 100, Money::ONE);
    auto ask_info =
      OrderInfo(ask_fields, 1, time_from_string("2024-07-29 09:59:50"));
    auto ask_order = std::make_shared<PrimitiveOrder>(ask_info);
    rule.add(ask_order);
    ask_order->with([&] (auto status, const auto& reports) {
      auto cancel_report = make_update(reports.back(), OrderStatus::CANCELED,
        time_from_string("2024-07-29 09:59:59"));
      ask_order->update(cancel_report);
    });
    auto bid_fields = make_limit_order_fields(
      parse_ticker("ABX.TSX"), Side::BID, 100, 94 * Money::CENT);
    auto bid_info =
      OrderInfo(bid_fields, 2, time_from_string("2024-07-29 10:00:00"));
    auto bid_order = std::make_shared<PrimitiveOrder>(bid_info);
    rule.add(bid_order);
    time_client.set(time_from_string("2024-07-29 10:00:05"));
    REQUIRE_NOTHROW(rule.submit(bid_order));
  }

  TEST_CASE("multiple_cancels_updates_last_cancel_time") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-29 09:59:50"));
    auto rule =
      OpposingSubmissionComplianceRule(seconds(10), Money::ZERO, &time_client);
    auto ask_fields = make_limit_order_fields(
      parse_ticker("ABX.TSX"), Side::ASK, 100, Money::ONE);
    auto ask_info1 =
      OrderInfo(ask_fields, 1, time_from_string("2024-07-29 09:59:50"));
    auto ask_order1 = std::make_shared<PrimitiveOrder>(ask_info1);
    rule.add(ask_order1);
    ask_order1->with([&] (auto status, const auto& reports) {
      auto cancel_report1 = make_update(reports.back(), OrderStatus::CANCELED,
        time_from_string("2024-07-29 09:59:55"));
      ask_order1->update(cancel_report1);
    });
    auto ask_info2 =
      OrderInfo(ask_fields, 2, time_from_string("2024-07-29 09:59:51"));
    auto ask_order2 = std::make_shared<PrimitiveOrder>(ask_info2);
    rule.add(ask_order2);
    ask_order2->with([&] (auto status, const auto& reports) {
      auto cancel_report2 = make_update(reports.back(), OrderStatus::CANCELED,
        time_from_string("2024-07-29 09:59:58"));
      ask_order2->update(cancel_report2);
    });
    auto bid_fields = make_limit_order_fields(
      parse_ticker("ABX.TSX"), Side::BID, 100, Money::ONE);
    auto bid_info =
      OrderInfo(bid_fields, 3, time_from_string("2024-07-29 10:00:00"));
    auto bid_order = std::make_shared<PrimitiveOrder>(bid_info);
    rule.add(bid_order);
    time_client.set(time_from_string("2024-07-29 10:00:05"));
    REQUIRE_THROWS_AS(rule.submit(bid_order), ComplianceCheckException);
    time_client.set(time_from_string("2024-07-29 10:00:08"));
    REQUIRE_THROWS_AS(rule.submit(bid_order), ComplianceCheckException);
    time_client.set(time_from_string("2024-07-29 10:00:10"));
    REQUIRE_NOTHROW(rule.submit(bid_order));
  }

  TEST_CASE("submit_on_same_side_as_cancel") {
    auto time_client = FixedTimeClient(time_from_string("2024-07-29 09:59:50"));
    auto rule =
      OpposingSubmissionComplianceRule(seconds(10), Money::ZERO, &time_client);
    auto bid_fields = make_limit_order_fields(
      parse_ticker("ABX.TSX"), Side::BID, 100, Money::ONE);
    auto bid_info =
      OrderInfo(bid_fields, 1, time_from_string("2024-07-29 09:59:50"));
    auto bid_order = std::make_shared<PrimitiveOrder>(bid_info);
    rule.add(bid_order);
    bid_order->with([&] (auto status, const auto& reports) {
      auto cancel_report = make_update(reports.back(), OrderStatus::CANCELED,
        time_from_string("2024-07-29 09:59:59"));
      bid_order->update(cancel_report);
    });
    auto bid_info2 =
      OrderInfo(bid_fields, 2, time_from_string("2024-07-29 10:00:00"));
    auto bid_order2 = std::make_shared<PrimitiveOrder>(bid_info2);
    rule.add(bid_order2);
    time_client.set(time_from_string("2024-07-29 10:00:05"));
    REQUIRE_NOTHROW(rule.submit(bid_order2));
  }
}
