#include <future>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceCheckOrderExecutionDriver.hpp"
#include "Nexus/ComplianceTests/TestComplianceClient.hpp"
#include "Nexus/ComplianceTests/TestComplianceRule.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::Compliance::Tests;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  using TestComplianceRuleSet =
    ComplianceRuleSet<TestComplianceClient*, ServiceLocatorClientBox>;
  using TestComplianceDriver = ComplianceCheckOrderExecutionDriver<
    TestOrderExecutionDriver*, FixedTimeClient*, TestComplianceRuleSet*>;

  struct Fixture {
    ServiceLocatorTestEnvironment m_service_locator_environment;
    DirectoryEntry m_account;
    FixedTimeClient m_time_client;
    OrderExecutionSession m_session;
    std::shared_ptr<TestOrderExecutionDriver::Queue> m_driver_operations;
    TestOrderExecutionDriver m_test_driver;
    std::shared_ptr<TestComplianceClient::Queue> m_client_operations;
    TestComplianceClient m_client;
    std::shared_ptr<Queue<std::shared_ptr<TestComplianceRule::Queue>>>
      m_rule_operations;
    optional<TestComplianceRuleSet> m_rules;
    optional<TestComplianceDriver> m_compliance_driver;

    Fixture()
        : m_time_client(time_from_string("2025-03-22 15:12:19:22")),
          m_driver_operations(
            std::make_shared<TestOrderExecutionDriver::Queue>()),
          m_test_driver(m_driver_operations),
          m_client_operations(std::make_shared<TestComplianceClient::Queue>()),
          m_client(m_client_operations),
          m_rule_operations(std::make_shared<
            Queue<std::shared_ptr<TestComplianceRule::Queue>>>()) {
      m_account = m_service_locator_environment.GetRoot().MakeAccount(
        "user", "", DirectoryEntry::GetStarDirectory());
      m_session.SetAccount(m_account);
      m_rules.emplace(&m_client,
        m_service_locator_environment.MakeClient("user", ""),
        [=, this] (const ComplianceRuleEntry& entry) {
          auto operations = std::make_shared<TestComplianceRule::Queue>();
          m_rule_operations->Push(operations);
          return std::make_unique<TestComplianceRule>(operations);
        });
      m_compliance_driver.emplace(&m_test_driver, &m_time_client, &*m_rules);
    }
  };
}

TEST_SUITE("ComplianceCheckOrderExecutionDriver") {
  TEST_CASE("submit") {
    auto fixture = Fixture();
    auto schema = ComplianceRuleSchema("test_rule", {});
    auto security = Security("TST", TSX);
    auto info = OrderInfo(make_limit_order_fields(
      fixture.m_account, security, CAD, Side::BID, "TSX", 100, Money::ONE), 123,
      time_from_string("2025-03-22 15:12:22:00"));
    auto async_order = std::async(std::launch::async, [&] {
      return fixture.m_compliance_driver->submit(info);
    });
    auto client_operation = fixture.m_client_operations->Pop();
    auto monitor_operation =
      std::get_if<TestComplianceClient::MonitorComplianceRuleEntriesOperation>(
        &*client_operation);
    REQUIRE(monitor_operation);
    REQUIRE(monitor_operation->m_directory_entry == fixture.m_account);
    auto entry = ComplianceRuleEntry(
      1, fixture.m_account, ComplianceRuleEntry::State::ACTIVE, schema);
    monitor_operation->m_result.set(std::vector{entry});
    auto rule_queue = fixture.m_rule_operations->Pop();
    auto rule_operation = rule_queue->Pop();
    auto submit_operation =
      std::get_if<TestComplianceRule::SubmitOperation>(&*rule_operation);
    REQUIRE(submit_operation);
    REQUIRE(submit_operation->m_order->get_info().m_id == info.m_id);
    SUBCASE("reject_submission") {
      submit_operation->m_result.SetException(
        ComplianceCheckException("Test order rejected."));
      client_operation = fixture.m_client_operations->Pop();
      auto report_operation =
        std::get_if<TestComplianceClient::ReportOperation>(&*client_operation);
      REQUIRE(report_operation);
      report_operation->m_result.set();
      auto order = async_order.get();
      auto reports = *order->get_publisher().GetSnapshot();
      REQUIRE(reports.size() == 2);
      REQUIRE(reports[1].m_status == OrderStatus::REJECTED);
      REQUIRE(reports[1].m_text == "Test order rejected.");
    }
    SUBCASE("accept_submission") {
      submit_operation->m_result.SetResult();
      auto driver_operation = fixture.m_driver_operations->Pop();
      auto driver_submit_operation =
        std::get_if<TestOrderExecutionDriver::SubmitOperation>(
          &*driver_operation);
      REQUIRE(driver_submit_operation);
      REQUIRE(driver_submit_operation->m_info.m_id == info.m_id);
      auto driver_order = std::make_shared<PrimitiveOrder>(info);
      driver_submit_operation->m_result.SetResult(driver_order);
      auto order = async_order.get();
      auto async_cancel = std::async(std::launch::async, [&] {
        return fixture.m_compliance_driver->cancel(
          fixture.m_session, order->get_info().m_id);
      });
      rule_operation = rule_queue->Pop();
      auto cancel_operation =
        std::get_if<TestComplianceRule::CancelOperation>(&*rule_operation);
      REQUIRE(cancel_operation);
      REQUIRE(cancel_operation->m_order == order);
      SUBCASE("accept_cancel") {
        cancel_operation->m_result.SetResult();
        driver_operation = fixture.m_driver_operations->Pop();
        auto driver_cancel_operation =
          std::get_if<TestOrderExecutionDriver::CancelOperation>(
            &*driver_operation);
        REQUIRE(driver_cancel_operation);
        REQUIRE(driver_cancel_operation->m_id == order->get_info().m_id);
        REQUIRE(driver_cancel_operation->m_session->GetAccount() ==
          fixture.m_session.GetAccount());
        driver_cancel_operation->m_result.SetResult();
        async_cancel.get();
      }
      SUBCASE("reject_cancel") {
        cancel_operation->m_result.SetException(
          ComplianceCheckException("Invalid test cancel."));
        client_operation = fixture.m_client_operations->Pop();
        auto report_operation =
          std::get_if<TestComplianceClient::ReportOperation>(
            &*client_operation);
        REQUIRE(report_operation);
        report_operation->m_result.set();
        auto reports = std::make_shared<Queue<ExecutionReport>>();
        order->get_publisher().Monitor(reports);
        reports->Pop();
        auto cancel_reject = reports->Pop();
        REQUIRE(cancel_reject.m_status == OrderStatus::CANCEL_REJECT);
        REQUIRE(cancel_reject.m_text == "Invalid test cancel.");
      }
    }
  }
}
