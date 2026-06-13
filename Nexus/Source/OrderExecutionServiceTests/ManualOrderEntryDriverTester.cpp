#include <future>
#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/TestAdministrationClient.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/OrderExecutionService/ManualOrderEntryDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionDriver.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Tests;
using namespace Nexus::Venues;

namespace {
  struct Fixture {
    std::shared_ptr<Queue<std::shared_ptr<TestOrderExecutionDriver::Operation>>>
      m_driver_operations;
    TestOrderExecutionDriver m_test_driver;
    std::shared_ptr<Queue<std::shared_ptr<TestAdministrationClient::Operation>>>
      m_admin_operations;
    TestAdministrationClient m_admin_client;
    ManualOrderEntryDriver<
      TestOrderExecutionDriver*, TestAdministrationClient*> m_driver;

    Fixture()
      : m_driver_operations(std::make_shared<
          Queue<std::shared_ptr<TestOrderExecutionDriver::Operation>>>()),
        m_test_driver(m_driver_operations),
        m_admin_operations(std::make_shared<
          Queue<std::shared_ptr<TestAdministrationClient::Operation>>>()),
        m_admin_client(m_admin_operations),
        m_driver("MANUAL", &m_test_driver, &m_admin_client) {}
  };

  auto make_order_info(const DirectoryEntry& account,
      const std::string& destination, OrderId id = 1) {
    auto fields = OrderFields();
    fields.m_account = account;
    fields.m_ticker = parse_ticker("TST.TSX");
    fields.m_currency = CAD;
    fields.m_type = OrderType::LIMIT;
    fields.m_side = Side::BID;
    fields.m_destination = destination;
    fields.m_quantity = 100;
    fields.m_price = Money::ONE;
    return OrderInfo(fields, id, false, ptime(not_a_date_time));
  }
}

TEST_SUITE("ManualOrderEntryDriver") {
  TEST_CASE("successful_submission") {
    auto fixture = Fixture();
    auto info = make_order_info(DirectoryEntry::make_account(123), "MANUAL");
    auto future_order = std::async(std::launch::async, [&] {
      return fixture.m_driver.submit(info);
    });
    auto admin_operation = fixture.m_admin_operations->pop();
    auto& check_admin_operation =
      std::get<TestAdministrationClient::CheckAdministratorOperation>(
        *admin_operation);
    REQUIRE(check_admin_operation.m_account == info.m_submission_account);
    check_admin_operation.m_result.set(true);
    auto submitted_order = future_order.get();
    REQUIRE(submitted_order->get_info() == info);
    auto reports = *submitted_order->get_publisher().get_snapshot();
    REQUIRE(reports.size() == 3);
    REQUIRE(reports[0].m_status == OrderStatus::PENDING_NEW);
    REQUIRE(reports[1].m_status == OrderStatus::NEW);
    REQUIRE(reports[2].m_status == OrderStatus::FILLED);
    REQUIRE(reports[2].m_last_quantity == info.m_fields.m_quantity);
    REQUIRE(reports[2].m_last_price == info.m_fields.m_price);
    REQUIRE(reports[2].m_last_market == "MANUAL");
  }

  TEST_CASE("insufficient_permissions") {
    auto fixture = Fixture();
    auto info = make_order_info(DirectoryEntry::make_account(123), "MANUAL");
    auto future_order = std::async(std::launch::async, [&] {
      return fixture.m_driver.submit(info);
    });
    auto admin_operation = fixture.m_admin_operations->pop();
    auto& check_admin_operation =
      std::get<TestAdministrationClient::CheckAdministratorOperation>(
        *admin_operation);
    REQUIRE(check_admin_operation.m_account == info.m_submission_account);
    check_admin_operation.m_result.set(false);
    auto submitted_order = future_order.get();
    REQUIRE(submitted_order->get_info() == info);
    auto reports = *submitted_order->get_publisher().get_snapshot();
    REQUIRE(reports.size() == 2);
    REQUIRE(reports[0].m_status == OrderStatus::PENDING_NEW);
    REQUIRE(reports[1].m_status == OrderStatus::REJECTED);
    REQUIRE(reports[1].m_text ==
      "Insufficient permissions to execute a manual order.");
  }

  TEST_CASE("submitted_order_added_to_driver") {
    auto fixture = Fixture();
    auto info = make_order_info(DirectoryEntry::make_account(123), "MANUAL");
    auto future_order = std::async(std::launch::async, [&] {
      return fixture.m_driver.submit(info);
    });
    auto admin_operation = fixture.m_admin_operations->pop();
    auto& check_admin_operation =
      std::get<TestAdministrationClient::CheckAdministratorOperation>(
        *admin_operation);
    check_admin_operation.m_result.set(true);
    auto submitted_order = future_order.get();
    auto driver_operation = fixture.m_driver_operations->pop();
    auto& add_operation =
      std::get<TestOrderExecutionDriver::AddOperation>(*driver_operation);
    REQUIRE(add_operation.m_order == submitted_order);
  }

  TEST_CASE("forward_submission") {
    auto fixture = Fixture();
    auto info = make_order_info(DirectoryEntry::make_account(123), "FORWARD");
    auto future_order = std::async(std::launch::async, [&] {
      return fixture.m_driver.submit(info);
    });
    auto driver_operation = fixture.m_driver_operations->pop();
    auto& submit_operation =
      std::get<TestOrderExecutionDriver::SubmitOperation>(*driver_operation);
    REQUIRE(submit_operation.m_info == info);
    auto order = std::make_shared<PrimitiveOrder>(info);
    submit_operation.m_result.set(order);
    auto submitted_order = future_order.get();
    REQUIRE(submitted_order == order);
  }

  TEST_CASE("restore") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(123);
    auto snapshot = InventorySnapshot();
    snapshot.m_sequence = Beam::Sequence(3);
    snapshot.m_excluded_orders = {8};
    auto records = std::vector<SequencedOrderRecord>();
    records.push_back(SequencedValue(
      OrderRecord(make_order_info(account, "MANUAL", 6), {}),
      Beam::Sequence(4)));
    records.push_back(SequencedValue(
      OrderRecord(make_order_info(account, "FORWARD", 8), {}),
      Beam::Sequence(5)));
    auto future_orders = std::async(std::launch::async, [&] {
      return fixture.m_driver.restore(account, snapshot, records);
    });
    auto driver_operation = fixture.m_driver_operations->pop();
    auto& restore_operation =
      std::get<TestOrderExecutionDriver::RestoreOperation>(*driver_operation);
    REQUIRE(restore_operation.m_account == account);
    REQUIRE(restore_operation.m_snapshot == snapshot);
    REQUIRE(restore_operation.m_records ==
      std::vector<SequencedOrderRecord>{records[1]});
    auto inner_order = std::make_shared<PrimitiveOrder>(
      make_order_info(account, "FORWARD", 8));
    restore_operation.m_result.set(
      std::vector<std::shared_ptr<Order>>{inner_order});
    auto orders = future_orders.get();
    REQUIRE(orders.size() == 2);
    REQUIRE(orders[0]->get_info().m_id == 6);
    REQUIRE(orders[1] == inner_order);
  }

  TEST_CASE("restore_fills_unfilled_manual_order") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(123);
    auto info = make_order_info(account, "MANUAL", 6);
    auto records = std::vector<SequencedOrderRecord>();
    records.push_back(SequencedValue(OrderRecord(info, {}), Beam::Sequence(4)));
    auto future_orders = std::async(std::launch::async, [&] {
      return fixture.m_driver.restore(account, InventorySnapshot(), records);
    });
    auto driver_operation = fixture.m_driver_operations->pop();
    auto& restore_operation =
      std::get<TestOrderExecutionDriver::RestoreOperation>(*driver_operation);
    REQUIRE(restore_operation.m_records.empty());
    restore_operation.m_result.set(std::vector<std::shared_ptr<Order>>());
    auto orders = future_orders.get();
    REQUIRE(orders.size() == 1);
    REQUIRE(orders[0]->get_info().m_id == 6);
    auto reports = *orders[0]->get_publisher().get_snapshot();
    REQUIRE(reports.size() == 3);
    REQUIRE(reports[0].m_status == OrderStatus::PENDING_NEW);
    REQUIRE(reports[1].m_status == OrderStatus::NEW);
    REQUIRE(reports[2].m_status == OrderStatus::FILLED);
    REQUIRE(reports[2].m_last_quantity == info.m_fields.m_quantity);
    REQUIRE(reports[2].m_last_price == info.m_fields.m_price);
    REQUIRE(reports[2].m_last_market == "MANUAL");
  }

  TEST_CASE("restore_preserves_filled_manual_order") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(123);
    auto info = make_order_info(account, "MANUAL", 6);
    auto template_order = std::make_shared<PrimitiveOrder>(info);
    accept(*template_order);
    fill(*template_order, info.m_fields.m_quantity);
    auto filled_reports = *template_order->get_publisher().get_snapshot();
    auto records = std::vector<SequencedOrderRecord>();
    records.push_back(
      SequencedValue(OrderRecord(info, filled_reports), Beam::Sequence(4)));
    auto future_orders = std::async(std::launch::async, [&] {
      return fixture.m_driver.restore(account, InventorySnapshot(), records);
    });
    auto driver_operation = fixture.m_driver_operations->pop();
    auto& restore_operation =
      std::get<TestOrderExecutionDriver::RestoreOperation>(*driver_operation);
    REQUIRE(restore_operation.m_records.empty());
    restore_operation.m_result.set(std::vector<std::shared_ptr<Order>>());
    auto orders = future_orders.get();
    REQUIRE(orders.size() == 1);
    REQUIRE(*orders[0]->get_publisher().get_snapshot() == filled_reports);
  }

  TEST_CASE("filters_own_orders") {
    auto fixture = Fixture();
    auto info = make_order_info(DirectoryEntry::make_account(123), "MANUAL");
    auto future_order = std::async(std::launch::async, [&] {
      return fixture.m_driver.submit(info);
    });
    auto admin_operation = fixture.m_admin_operations->pop();
    std::get<TestAdministrationClient::CheckAdministratorOperation>(
      *admin_operation).m_result.set(true);
    future_order.get();
    auto add_operation = fixture.m_driver_operations->pop();
    REQUIRE(std::holds_alternative<TestOrderExecutionDriver::AddOperation>(
      *add_operation));
    auto session = OrderExecutionSession();
    fixture.m_driver.cancel(session, 1);
    fixture.m_driver.update(session, 1, ExecutionReport());
    auto future_cancel = std::async(std::launch::async, [&] {
      fixture.m_driver.cancel(session, 2);
    });
    auto driver_operation = fixture.m_driver_operations->pop();
    auto& cancel_operation =
      std::get<TestOrderExecutionDriver::CancelOperation>(*driver_operation);
    REQUIRE(cancel_operation.m_id == 2);
    cancel_operation.m_result.set();
    future_cancel.get();
  }
}
