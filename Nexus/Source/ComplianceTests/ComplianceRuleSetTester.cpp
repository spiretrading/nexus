#include <future>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceRuleSet.hpp"
#include "Nexus/ComplianceTests/TestComplianceClient.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

namespace {
  using TestComplianceRuleSet =
    ComplianceRuleSet<TestComplianceClient*, ServiceLocatorClient>;

  struct Fixture {
    ServiceLocatorTestEnvironment m_service_locator_environment;
    std::shared_ptr<TestComplianceClient::Queue> m_operations;
    TestComplianceClient m_client;

    Fixture()
      : m_operations(std::make_shared<TestComplianceClient::Queue>()),
        m_client(m_operations) {}
  };
}

TEST_SUITE("ComplianceRuleSet") {
  TEST_CASE("load_rules_for_account_on_first_use") {
    auto fixture = Fixture();
    auto account =
      fixture.m_service_locator_environment.get_root().make_account(
        "user", "pw", DirectoryEntry::STAR_DIRECTORY);
    auto rule_set = TestComplianceRuleSet(&fixture.m_client,
      fixture.m_service_locator_environment.make_client("user", "pw"),
      [] (const ComplianceRuleEntry&) {
        struct DummyRule final : ComplianceRule {
          void submit(const std::shared_ptr<Order>&) override {}
          void cancel(const std::shared_ptr<Order>&) override {}
          void add(const std::shared_ptr<Order>&) override {}
        };
        return std::make_unique<DummyRule>();
      });
    auto security = parse_ticker("TST.TSX");
    auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
      make_limit_order_fields(account, security, CAD, Side::BID, "TSX", 100,
        Money::ONE), 123, time_from_string("2024-03-12 13:12:00:00")));
    auto submission = std::async(std::launch::async, [&] {
      rule_set.submit(order);
    });
    auto operation = fixture.m_operations->pop();
    auto monitor_operation =
      std::get_if<TestComplianceClient::MonitorComplianceRuleEntriesOperation>(
        &*operation);
    REQUIRE(monitor_operation);
    REQUIRE(monitor_operation->m_directory_entry == account);
    monitor_operation->m_result.set(std::vector<ComplianceRuleEntry>());
    submission.get();
  }
}
