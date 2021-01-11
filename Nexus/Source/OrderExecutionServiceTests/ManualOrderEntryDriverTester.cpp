#include <doctest/doctest.h>
#include "Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionService/ManualOrderEntryDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  using TestManualOrderEntryDriver = ManualOrderEntryDriver<
    MockOrderExecutionDriver*, AdministrationClientBox>;

  const auto SECURITY = Security("ABX", DefaultMarkets::TSX(),
    DefaultCountries::CA());

  struct Fixture {
    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    optional<AdministrationServiceTestEnvironment>
      m_administrationTestEnvironment;
    MockOrderExecutionDriver m_testDriver;
    optional<TestManualOrderEntryDriver> m_driver;

    Fixture() {
      m_serviceLocatorEnvironment.GetRoot().MakeAccount("administrator", "",
        DirectoryEntry::GetStarDirectory());
      auto adminServiceLocatorClient = m_serviceLocatorEnvironment.MakeClient(
        "administrator", "");
      m_serviceLocatorEnvironment.GetRoot().StorePermissions(
        adminServiceLocatorClient.GetAccount(),
        DirectoryEntry::GetStarDirectory(),
        Permissions().
          Set(Permission::ADMINISTRATE).
          Set(Permission::MOVE).
          Set(Permission::READ));
      auto adminAccount = adminServiceLocatorClient.GetAccount();
      m_administrationTestEnvironment.emplace(adminServiceLocatorClient);
      m_administrationTestEnvironment->MakeAdministrator(adminAccount);
      m_driver.emplace("MOE", &m_testDriver,
        m_administrationTestEnvironment->MakeClient(
          m_serviceLocatorEnvironment.MakeClient("administrator", "")));
    }
  };
}

TEST_SUITE("ManualOrderEntryDriver") {
  TEST_CASE_FIXTURE(Fixture, "submit_moe_without_permission") {
    auto account = m_serviceLocatorEnvironment.GetRoot().MakeAccount("trader",
      "1234", DirectoryEntry::GetStarDirectory());
    auto orderInfo = OrderInfo(OrderFields::MakeLimitOrder(SECURITY, Side::BID,
      "MOE", 100, Money::ONE), account, 100, false,
      time_from_string("2020-12-18 14:49:12"));
    auto& rejectedOrder = m_driver->Submit(orderInfo);
    auto snapshot = rejectedOrder.GetPublisher().GetSnapshot();
    REQUIRE(snapshot.is_initialized());
    REQUIRE(!snapshot->empty());
    REQUIRE(snapshot->back().m_status == OrderStatus::REJECTED);
  }

  TEST_CASE_FIXTURE(Fixture, "submit_moe_with_permission") {
    auto adminAccount = m_serviceLocatorEnvironment.GetRoot().FindAccount(
      "administrator");
    REQUIRE(adminAccount.is_initialized());
    auto orderInfo = OrderInfo(OrderFields::MakeLimitOrder(SECURITY, Side::BID,
      "MOE", 100, Money::ONE), *adminAccount, 100, false,
      time_from_string("2020-12-18 14:49:12"));
    auto& filledOrder = m_driver->Submit(orderInfo);
    auto executionReports = std::make_shared<Queue<ExecutionReport>>();
    filledOrder.GetPublisher().Monitor(executionReports);
    REQUIRE(executionReports->Pop().m_status == OrderStatus::PENDING_NEW);
    REQUIRE(executionReports->Pop().m_status == OrderStatus::NEW);
    REQUIRE(executionReports->Pop().m_status == OrderStatus::FILLED);
  }

  TEST_CASE_FIXTURE(Fixture, "submit") {
    auto account = m_serviceLocatorEnvironment.GetRoot().MakeAccount("trader",
      "1234", DirectoryEntry::GetStarDirectory());
    auto orderInfo = OrderInfo(OrderFields::MakeLimitOrder(SECURITY, Side::BID,
      "TSX", 100, Money::ONE), account, 100, false,
      time_from_string("2020-12-18 14:49:12"));
    auto submittedOrders = std::make_shared<Queue<PrimitiveOrder*>>();
    m_testDriver.SetOrderStatusNewOnSubmission(true);
    m_testDriver.GetPublisher().Monitor(submittedOrders);
    auto& order = m_driver->Submit(orderInfo);
    auto snapshot = order.GetPublisher().GetSnapshot();
    REQUIRE(snapshot.is_initialized());
    REQUIRE(!snapshot->empty());
    REQUIRE(snapshot->back().m_status == OrderStatus::NEW);
    auto receivedOrder = submittedOrders->Pop();
    REQUIRE(receivedOrder == &order);
    auto session = OrderExecutionSession();
    session.SetAccount(account);
    m_driver->Cancel(session, order.GetInfo().m_orderId);
    snapshot = order.GetPublisher().GetSnapshot();
    REQUIRE(snapshot.is_initialized());
    REQUIRE(!snapshot->empty());
    REQUIRE(snapshot->back().m_status == OrderStatus::PENDING_CANCEL);
  }

  TEST_CASE_FIXTURE(Fixture, "recover_moe") {
    auto account = m_serviceLocatorEnvironment.GetRoot().MakeAccount("trader",
      "1234", DirectoryEntry::GetStarDirectory());
    auto orderInfo = OrderInfo(OrderFields::MakeLimitOrder(SECURITY, Side::BID,
      "MOE", 100, Money::ONE), account, 100, false,
      time_from_string("2020-12-18 14:49:12"));
    auto submittedOrders = std::make_shared<Queue<PrimitiveOrder*>>();
    m_testDriver.SetOrderStatusNewOnSubmission(true);
    m_testDriver.GetPublisher().Monitor(submittedOrders);
    auto& order = m_driver->Recover(SequencedValue(
      IndexedValue(OrderRecord(orderInfo, {}), account),
      Queries::Sequence(100)));
    REQUIRE_THROWS(m_testDriver.FindOrder(order.GetInfo().m_orderId));
  }
}
