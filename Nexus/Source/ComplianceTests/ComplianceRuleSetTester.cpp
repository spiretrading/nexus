#include <future>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceRuleSet.hpp"
#include "Nexus/ComplianceTests/TestComplianceClient.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
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
    auto account = fixture.m_service_locator_environment.GetRoot().MakeAccount(
      "user", "pw", DirectoryEntry::GetStarDirectory());
    auto rule_set = TestComplianceRuleSet(&fixture.m_client,
      fixture.m_service_locator_environment.MakeClient("user", "pw"),
      [] (const ComplianceRuleEntry&) {
        struct DummyRule final : ComplianceRule {
          void submit(const std::shared_ptr<const Order>&) override {}
          void cancel(const std::shared_ptr<const Order>&) override {}
          void add(const std::shared_ptr<const Order>&) override {}
        };
        return std::make_unique<DummyRule>();
      });
    auto security = Security("TST", TSX);
    auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
      make_limit_order_fields(account, security, CAD, Side::BID, "TSX", 100,
        Money::ONE), 123, time_from_string("2024-03-12 13:12:00:00")));
    auto submission = std::async(std::launch::async, [&] {
      rule_set.submit(order);
    });
    auto operation = fixture.m_operations->Pop();
    auto monitor_operation =
      std::get_if<TestComplianceClient::MonitorComplianceRuleEntriesOperation>(
        &*operation);
    REQUIRE(monitor_operation != nullptr);
    REQUIRE(monitor_operation->m_directory_entry == account);
    monitor_operation->m_result.set(std::vector<ComplianceRuleEntry>());
    submission.get();
  }
}
