#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

namespace {
  auto make_order_fields() {
    return make_limit_order_fields(
      Security("TST", TSX), Side::BID, "TSX", Quantity(100), Money::ONE);
  }

  auto get_status(const PrimitiveOrder& order) {
    return order.with([] (auto status, const auto& reports) {
      return status;
    });
  }

  void require_broken(auto& publisher) {
    auto queue = std::make_shared<Queue<ExecutionReport>>();
    publisher.monitor(queue);
    auto reports = std::vector<ExecutionReport>();
    flush(queue, std::back_inserter(reports));
    REQUIRE(queue->is_broken());
  }
}

TEST_SUITE("PrimitiveOrder") {
  TEST_CASE("construct_from_order_info") {
    auto timestamp = time_from_string("2024-05-21 00:00:10.000");
    auto fields = make_order_fields();
    auto info = OrderInfo(fields, 123, timestamp);
    auto order = PrimitiveOrder(info);
    REQUIRE(order.get_info() == info);
    auto reports = order.get_publisher().get_snapshot();
    REQUIRE(reports->size() == 1);
    auto& initial_report = reports->front();
    REQUIRE(initial_report.m_id == 123);
    REQUIRE(initial_report.m_timestamp == timestamp);
    REQUIRE(initial_report.m_status == OrderStatus::PENDING_NEW);
    REQUIRE(get_status(order) == OrderStatus::PENDING_NEW);
  }

  TEST_CASE("construct_from_order_record") {
    auto timestamp1 = time_from_string("2024-05-21 00:00:10.000");
    auto fields = make_order_fields();
    auto info = OrderInfo(fields, 124, timestamp1);
    auto report1 = ExecutionReport(124, timestamp1);
    auto timestamp2 = timestamp1 + seconds(1);
    auto report2 = make_update(report1, OrderStatus::NEW, timestamp2);
    auto timestamp3 = timestamp2 + seconds(1);
    auto report3 =
      make_update(report2, OrderStatus::PARTIALLY_FILLED, timestamp3);
    report3.m_last_quantity = 50;
    auto record = OrderRecord(info, {report1, report2, report3});
    auto order = PrimitiveOrder(record);
    REQUIRE(order.get_info() == info);
    auto reports = order.get_publisher().get_snapshot();
    REQUIRE(reports->size() == 3);
    REQUIRE((*reports)[0] == report1);
    REQUIRE((*reports)[1] == report2);
    REQUIRE((*reports)[2] == report3);
    REQUIRE(get_status(order) == OrderStatus::NEW);
  }

  TEST_CASE("update") {
    auto timestamp1 = time_from_string("2024-05-21 00:00:10.000");
    auto fields = make_order_fields();
    auto info = OrderInfo(fields, 125, timestamp1);
    auto order = PrimitiveOrder(info);
    auto initial_report = order.get_publisher().get_snapshot()->front();
    auto timestamp2 = timestamp1 + seconds(1);
    auto new_report = make_update(initial_report, OrderStatus::NEW, timestamp2);
    order.update(new_report);
    auto reports = order.get_publisher().get_snapshot();
    REQUIRE(reports->size() == 2);
    REQUIRE(reports->back() == new_report);
    REQUIRE(get_status(order) == OrderStatus::NEW);
    auto timestamp3 = timestamp2 + seconds(1);
    auto filled_report =
      make_update(new_report, OrderStatus::FILLED, timestamp3);
    order.update(filled_report);
    REQUIRE(get_status(order) == OrderStatus::FILLED);
    require_broken(order.get_publisher());
  }

  TEST_CASE("make_rejected_order") {
    auto timestamp = time_from_string("2024-05-21 00:00:10.000");
    auto fields = make_order_fields();
    auto info = OrderInfo(fields, 126, timestamp);
    auto reason = "Insufficient funds";
    auto order = make_rejected_order(info, reason);
    REQUIRE(order->get_info() == info);
    auto reports = order->get_publisher().get_snapshot();
    REQUIRE(reports->size() == 2);
    REQUIRE(reports->back().m_status == OrderStatus::REJECTED);
    REQUIRE(reports->back().m_text == reason);
    REQUIRE(get_status(*order) == OrderStatus::REJECTED);
    require_broken(order->get_publisher());
  }

  TEST_CASE("reject_cancel_request") {
    auto timestamp1 = time_from_string("2024-05-21 00:00:10.000");
    auto fields = make_order_fields();
    auto info = OrderInfo(fields, 127, timestamp1);
    auto order = PrimitiveOrder(info);
    auto initial_report = order.get_publisher().get_snapshot()->front();
    auto timestamp2 = timestamp1 + seconds(1);
    auto new_report = make_update(initial_report, OrderStatus::NEW, timestamp2);
    order.update(new_report);
    auto timestamp3 = timestamp2 + seconds(1);
    auto reason = std::string("Market closed");
    reject_cancel_request(order, timestamp3, reason);
    auto reports = order.get_publisher().get_snapshot();
    REQUIRE(reports->size() == 3);
    REQUIRE(reports->back().m_status == OrderStatus::CANCEL_REJECT);
    REQUIRE(reports->back().m_text == reason);
    REQUIRE(get_status(order) == OrderStatus::CANCEL_REJECT);
  }

  TEST_CASE("reject_cancel_request_on_terminal_order") {
    auto timestamp = time_from_string("2024-05-21 00:00:10.000");
    auto fields = make_order_fields();
    auto info = OrderInfo(fields, 128, timestamp);
    auto order = make_rejected_order(info, "Test reject");
    auto initial_report_count = order->get_publisher().get_snapshot()->size();
    reject_cancel_request(*order, timestamp + seconds(1), "Too late");
    auto final_report_count = order->get_publisher().get_snapshot()->size();
    REQUIRE(initial_report_count == final_report_count);
  }
}
