#include <Beam/Queues/Queue.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/ExecutionReportPublisher.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;

TEST_SUITE("ExecutionReportPublisher") {
  TEST_CASE("single_order_single_report") {
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields = make_limit_order_fields(
      DirectoryEntry::make_account(1, "test"), parse_ticker("TST.TSX"), Side::BID,
      "TSX", Quantity(100), Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    order_queue->push(order);
    order_queue->close();
    auto publisher = ExecutionReportPublisher(order_queue);
    auto entries = *publisher.get_snapshot();
    REQUIRE(entries.size() == 1);
    REQUIRE(entries.front().m_order == order);
    REQUIRE(entries.front().m_report.m_id == 1);
    REQUIRE(entries.front().m_report.m_timestamp == timestamp);
    REQUIRE(entries.front().m_report.m_status == OrderStatus::PENDING_NEW);
  }

  TEST_CASE("single_order_multiple_reports") {
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields = make_limit_order_fields(
      DirectoryEntry::make_account(2, "test"), parse_ticker("TST.TSX"), Side::ASK,
      "TSX", Quantity(200), Money::ONE);
    auto info = OrderInfo(fields, 2, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    auto report1 = ExecutionReport(2, timestamp);
    auto report2 =
      make_update(report1, OrderStatus::NEW, timestamp + seconds(1));
    auto report3 = make_update(
      report2, OrderStatus::PARTIALLY_FILLED, timestamp + seconds(2));
    order->update(report2);
    order->update(report3);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    order_queue->push(order);
    order_queue->close();
    auto publisher = ExecutionReportPublisher(order_queue);
    auto entries = *publisher.get_snapshot();
    REQUIRE(entries.size() == 3);
    REQUIRE(entries[0].m_order == order);
    REQUIRE(entries[0].m_report == report1);
    REQUIRE(entries[1].m_report == report2);
    REQUIRE(entries[2].m_report == report3);
  }

  TEST_CASE("multiple_orders_single_report_each") {
    auto timestamp1 = time_from_string("2024-07-21 10:00:00.000");
    auto timestamp2 = time_from_string("2024-07-21 10:01:00.000");
    auto fields1 = make_limit_order_fields(
      DirectoryEntry::make_account(3, "test"), parse_ticker("AAA.TSX"), Side::BID,
      "TSX", Quantity(50), Money::ONE);
    auto fields2 = make_limit_order_fields(
      DirectoryEntry::make_account(4, "test"), parse_ticker("BBB.TSX"), Side::ASK,
      "TSX", Quantity(75), Money::ONE);
    auto info1 = OrderInfo(fields1, 3, false, timestamp1);
    auto info2 = OrderInfo(fields2, 4, false, timestamp2);
    auto order1 = std::make_shared<PrimitiveOrder>(info1);
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    order_queue->push(order1);
    order_queue->push(order2);
    order_queue->close();
    auto publisher = ExecutionReportPublisher(order_queue);
    auto entries = *publisher.get_snapshot();
    REQUIRE(entries.size() == 2);
    REQUIRE((entries[0].m_order == order1 || entries[0].m_order == order2));
    REQUIRE((entries[1].m_order == order1 || entries[1].m_order == order2));
    REQUIRE(entries[0].m_report.m_status == OrderStatus::PENDING_NEW);
    REQUIRE(entries[1].m_report.m_status == OrderStatus::PENDING_NEW);
    REQUIRE(entries[0].m_report.m_id != entries[1].m_report.m_id);
  }

  TEST_CASE("dangling_publisher") {
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields = make_limit_order_fields(
      DirectoryEntry::make_account(1, "test"), parse_ticker("TST.TSX"), Side::BID,
      "TSX", Quantity(100), Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    {
      auto publisher = ExecutionReportPublisher(order_queue);
      order_queue->push(order);
      auto published_orders = std::make_shared<Queue<ExecutionReportEntry>>();
      publisher.monitor(published_orders);
      auto report = published_orders->pop();
      REQUIRE(report.m_order == order);
    }
    accept(*order);
  }
}
