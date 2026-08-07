#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include <quickfix/SessionID.h>
#include <quickfix/SessionSettings.h>
#include "Nexus/FixUtilities/FixOrderExecutionDriver.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct TestFixApplication : FixApplication {
    std::vector<OrderInfo> m_submissions;
    std::vector<OrderId> m_cancels;

    std::shared_ptr<Order> recover(
        const SequencedAccountOrderRecord&) override {
      return nullptr;
    }

    std::shared_ptr<Order> submit(const OrderInfo& info) override {
      m_submissions.push_back(info);
      return std::make_shared<PrimitiveOrder>(info);
    }

    void cancel(const OrderExecutionSession&, OrderId id) override {
      m_cancels.push_back(id);
    }

    void update(
      const OrderExecutionSession&, OrderId, const ExecutionReport&) override {}

    void onCreate(const FIX::SessionID&) override {}

    void onLogon(const FIX::SessionID&) override {}

    void onLogout(const FIX::SessionID&) override {}

    void toAdmin(FIX::Message&, const FIX::SessionID&) override {}

    void toApp(FIX::Message&, const FIX::SessionID&) override {}

    void fromAdmin(const FIX::Message&, const FIX::SessionID&) override {}

    void fromApp(const FIX::Message&, const FIX::SessionID&) override {}
  };

  auto make_record(OrderId id, OrderStatus status) {
    auto timestamp = time_from_string("2026-07-15 09:30:00.000");
    auto info = OrderInfo(make_limit_order_fields(parse_ticker("SHOP.TSX"),
      Side::BID, "TSX", 100, Money::ONE), id, timestamp);
    auto report = ExecutionReport(id, timestamp);
    auto update = make_update(report, status, timestamp);
    return SequencedValue(
      OrderRecord(info, {report, update}), Sequence(id));
  }
}

TEST_SUITE("FixOrderExecutionDriver") {
  TEST_CASE("destination_routing") {
    auto application_a = std::make_shared<TestFixApplication>();
    auto application_b = std::make_shared<TestFixApplication>();
    auto destinations_a = std::vector<std::string>{"TSX", "TSXV"};
    auto destinations_b = std::vector<std::string>{"ALPHA"};
    auto entries = std::vector<FixApplicationEntry>();
    entries.push_back(FixApplicationEntry(
      FIX::SessionSettings(), destinations_a, application_a));
    entries.push_back(FixApplicationEntry(
      FIX::SessionSettings(), destinations_b, application_b));
    auto driver = FixOrderExecutionDriver(entries);
    auto timestamp = time_from_string("2026-07-15 09:30:00.000");
    auto ticker = parse_ticker("SHOP.TSX");
    auto tsx_info = OrderInfo(make_limit_order_fields(
      ticker, Side::BID, "TSX", 100, Money::ONE), 1, timestamp);
    auto tsx_order = driver.submit(tsx_info);
    REQUIRE(tsx_order);
    REQUIRE(application_a->m_submissions.size() == 1);
    REQUIRE(application_a->m_submissions.back() == tsx_info);
    REQUIRE(application_b->m_submissions.empty());
    auto alpha_info = OrderInfo(make_limit_order_fields(
      ticker, Side::BID, "ALPHA", 100, Money::ONE), 2, timestamp);
    auto alpha_order = driver.submit(alpha_info);
    REQUIRE(alpha_order);
    REQUIRE(application_a->m_submissions.size() == 1);
    REQUIRE(application_b->m_submissions.size() == 1);
    REQUIRE(application_b->m_submissions.back() == alpha_info);
    auto session = OrderExecutionSession();
    driver.cancel(session, 1);
    REQUIRE(application_a->m_cancels.size() == 1);
    REQUIRE(application_a->m_cancels.back() == 1);
    REQUIRE(application_b->m_cancels.empty());
    driver.cancel(session, 2);
    REQUIRE(application_b->m_cancels.size() == 1);
    REQUIRE(application_b->m_cancels.back() == 2);
    auto unknown_info = OrderInfo(make_limit_order_fields(
      ticker, Side::BID, "CHIX", 100, Money::ONE), 3, timestamp);
    auto unknown_order = driver.submit(unknown_info);
    REQUIRE(unknown_order);
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    unknown_order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::REJECTED);
    REQUIRE(application_a->m_submissions.size() == 1);
    REQUIRE(application_b->m_submissions.size() == 1);
    driver.cancel(session, 3);
    REQUIRE(application_a->m_cancels.size() == 1);
    REQUIRE(application_b->m_cancels.size() == 1);
  }

  TEST_CASE("restore_unknown_destination_non_terminal") {
    auto driver = FixOrderExecutionDriver(std::vector<FixApplicationEntry>());
    auto records = std::vector<SequencedOrderRecord>();
    records.push_back(make_record(1, OrderStatus::NEW));
    REQUIRE_THROWS_AS(driver.restore(DirectoryEntry::make_account(123, "test"),
      InventorySnapshot(), records), OrderUnrecoverableException);
  }

  TEST_CASE("restore_unknown_destination_terminal") {
    auto driver = FixOrderExecutionDriver(std::vector<FixApplicationEntry>());
    auto records = std::vector<SequencedOrderRecord>();
    records.push_back(make_record(1, OrderStatus::CANCELED));
    auto orders = driver.restore(
      DirectoryEntry::make_account(123, "test"), InventorySnapshot(), records);
    REQUIRE(orders.size() == 1);
    REQUIRE(orders[0]->get_info() == records[0]->m_info);
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    orders[0]->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::CANCELED);
  }
}
