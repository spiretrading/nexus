#include <Beam/Queues/Queue.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/BuyingPowerCheck.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::RiskService;

namespace {
  auto TST = Security("TST", DefaultMarkets::NYSE(), DefaultCountries::US());

  struct Fixture {
    using TestBuyingPowerCheck = BuyingPowerCheck<VirtualAdministrationClient*,
      VirtualMarketDataClient*>;

    TestEnvironment m_environment;
    std::shared_ptr<Queue<const Order*>> m_orderSubmissions;
    TestServiceClients m_serviceClients;
    TestBuyingPowerCheck m_buyingPowerCheck;
    RiskParameters m_traderRiskParameters;

    Fixture()
        : m_orderSubmissions(std::make_shared<Queue<const Order*>>()),
          m_serviceClients(Ref(m_environment)),
          m_buyingPowerCheck(std::vector<ExchangeRate>{},
            &m_serviceClients.GetAdministrationClient(),
            &m_serviceClients.GetMarketDataClient()) {
      m_environment.Open();
      m_environment.MonitorOrderSubmissions(m_orderSubmissions);
      m_environment.UpdateBboPrice(TST, Money::ONE, Money::ONE + Money::CENT);
      m_serviceClients.Open();
      m_traderRiskParameters.m_currency = DefaultCurrencies::USD();
      m_traderRiskParameters.m_allowedState.m_type = RiskState::Type::ACTIVE;
      m_traderRiskParameters.m_buyingPower = 1000 * Money::ONE;
      m_environment.GetAdministrationEnvironment().MakeAdministrator(
        m_serviceClients.GetServiceLocatorClient().GetAccount());
      m_serviceClients.GetAdministrationClient().StoreRiskParameters(
        DirectoryEntry::GetRootAccount(), m_traderRiskParameters);
    }
  };
}

TEST_SUITE("BuyingPowerCheck") {
  TEST_CASE_FIXTURE(Fixture, "submission") {
    auto orderInfoA = OrderInfo(OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
      Side::BID, "NYSE", 100, Money::ONE), 1,
      m_environment.GetTimeEnvironment().GetTime());
    auto orderA = PrimitiveOrder(orderInfoA);
    REQUIRE_NOTHROW(m_buyingPowerCheck.Submit(orderInfoA));
    m_buyingPowerCheck.Add(orderA);
    auto orderInfoB = OrderInfo(OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
      Side::BID, "NYSE", 1000, Money::ONE), 2,
      m_environment.GetTimeEnvironment().GetTime());
    REQUIRE_THROWS_AS(m_buyingPowerCheck.Submit(orderInfoB),
      OrderSubmissionCheckException);
  }

  TEST_CASE_FIXTURE(Fixture, "add_without_submission") {
    auto orderInfoA = OrderInfo(OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
      Side::BID, "NYSE", 100, Money::ONE), 1,
      m_environment.GetTimeEnvironment().GetTime());
    auto orderA = PrimitiveOrder(orderInfoA);
    m_buyingPowerCheck.Add(orderA);
    auto orderInfoB = OrderInfo(OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
      Side::BID, "NYSE", 1000, Money::ONE), 2,
      m_environment.GetTimeEnvironment().GetTime());
    REQUIRE_THROWS_AS(m_buyingPowerCheck.Submit(orderInfoB),
      OrderSubmissionCheckException);
  }

  TEST_CASE_FIXTURE(Fixture, "submission_then_rejection") {
    auto orderInfoA = OrderInfo(OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
      Side::BID, "NYSE", 100, Money::ONE), 1, not_a_date_time);
    auto orderA = PrimitiveOrder(orderInfoA);
    REQUIRE_NOTHROW(m_buyingPowerCheck.Submit(orderInfoA));
    m_buyingPowerCheck.Reject(orderInfoA);
    auto orderInfoB = OrderInfo(OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
      Side::BID, "NYSE", 1000, Money::ONE), 2,
      m_environment.GetTimeEnvironment().GetTime());
    REQUIRE_NOTHROW(m_buyingPowerCheck.Submit(orderInfoB));
  }

  TEST_CASE_FIXTURE(Fixture, "order_recovery") {
    auto recoveryFields = OrderFields::BuildLimitOrder(TST, Side::BID, 100,
      Money::ONE);
    auto& recoverOrder = m_serviceClients.GetOrderExecutionClient().Submit(
      recoveryFields);
    auto submittedRecoveryOrder = m_orderSubmissions->Top();
    m_orderSubmissions->Pop();
    m_environment.AcceptOrder(*submittedRecoveryOrder);
    m_environment.FillOrder(*submittedRecoveryOrder, 100);
    REQUIRE_NOTHROW(m_buyingPowerCheck.Add(*submittedRecoveryOrder));
    auto submissionFields = OrderFields::BuildLimitOrder(TST, Side::BID, 100,
      2 * Money::ONE);
    auto& submissionOrder = m_serviceClients.GetOrderExecutionClient().Submit(
      submissionFields);
    auto submittedOrder = m_orderSubmissions->Top();
    m_orderSubmissions->Pop();
    REQUIRE_NOTHROW(m_buyingPowerCheck.Submit(submittedOrder->GetInfo()));
  }
}
