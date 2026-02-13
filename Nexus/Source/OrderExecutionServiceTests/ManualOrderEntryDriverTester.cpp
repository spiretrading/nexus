#include <future>
#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/TestAdministrationClient.hpp"
#include "Nexus/OrderExecutionService/ManualOrderEntryDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionDriver.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

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

  auto make_order_info(
      const DirectoryEntry& account, const std::string& destination) {
    auto fields = OrderFields();
    fields.m_account = account;
    fields.m_ticker = parse_ticker("TST.TSX");
    fields.m_currency = CAD;
    fields.m_type = OrderType::LIMIT;
    fields.m_side = Side::BID;
    fields.m_destination = destination;
    fields.m_quantity = 100;
    fields.m_price = Money::ONE;
    return OrderInfo(fields, 1, false, ptime(not_a_date_time));
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
}
