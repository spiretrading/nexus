#include <doctest/doctest.h>
#include "Nexus/Accounting/PortfolioController.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  static auto TST = Security("TST", DefaultMarkets::NYSE(),
    DefaultCountries::US());

  struct Fixture {
    using TestPortfolio = Portfolio<TrueAverageBookkeeper<Inventory<
      Position<Security>>>>;
    using TestPortfolioController = PortfolioController<TestPortfolio,
      VirtualMarketDataClient*>;

    TestEnvironment m_environment;
    TestServiceClients m_serviceClients;

    Fixture()
        : m_serviceClients(Ref(m_environment)) {
      m_environment.SetTime(
        ptime(gregorian::date(2000, gregorian::Jan, 1), seconds(0)));
    }
  };
}

TEST_SUITE("PortfolioController") {
  TEST_CASE_FIXTURE(Fixture, "out_of_order_execution_reports") {
    m_environment.Publish(TST, BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(Money::ONE, 100, Side::ASK), not_a_date_time));
    auto orders = std::make_shared<Queue<const Order*>>();
    auto controller = TestPortfolioController(
      Initialize(GetDefaultMarketDatabase()),
      &m_serviceClients.GetMarketDataClient(), orders);
    auto queue = std::make_shared<
      Queue<TestPortfolioController::UpdateEntry>>();
    controller.GetPublisher().Monitor(queue);
    auto fieldsA = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
      Side::BID, "NYSE", 100, Money::CENT);
    auto orderA = PrimitiveOrder({fieldsA, 1,
      m_serviceClients.GetTimeClient().GetTime()});
    auto fieldsB = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
      Side::BID, "NYSE", 100, 2 * Money::CENT);
    m_environment.AdvanceTime(seconds(1));
    auto orderB = PrimitiveOrder({fieldsB, 2,
      m_serviceClients.GetTimeClient().GetTime()});
    auto fieldsC = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
      Side::ASK, "NYSE", 100, 3 * Money::CENT);
    m_environment.AdvanceTime(seconds(1));
    auto orderC = PrimitiveOrder({fieldsC, 3,
      m_serviceClients.GetTimeClient().GetTime()});
    orders->Push(&orderA);
    orders->Push(&orderB);
    orders->Push(&orderC);
    auto date = m_serviceClients.GetTimeClient().GetTime().date();
    Fill(orderA, 100, ptime{date, seconds(1)});
    {
      auto update = queue->Pop();
      REQUIRE(update.m_securityInventory.m_position.m_quantity == 100);
      REQUIRE(GetAveragePrice(update.m_securityInventory.m_position) ==
        Money::CENT);
    }
    Fill(orderC, 100, ptime{date, seconds(2)});
    {
      auto update = queue->Pop();
      REQUIRE(update.m_securityInventory.m_position.m_quantity == 0);
      REQUIRE(GetAveragePrice(update.m_securityInventory.m_position) ==
        Money::ZERO);
    }
    Fill(orderB, 100, ptime{date, seconds(3)});
    {
      auto update = queue->Pop();
      REQUIRE(update.m_securityInventory.m_position.m_quantity == 100);
      REQUIRE(GetAveragePrice(update.m_securityInventory.m_position) ==
        2 * Money::CENT);
    }
    REQUIRE(!queue->TryPop());
  }

  TEST_CASE_FIXTURE(Fixture, "zero_position_bbo_update") {
    m_environment.Publish(TST, BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(Money::ONE, 100, Side::ASK), not_a_date_time));
    auto orders = std::make_shared<Queue<const Order*>>();
    auto controller = TestPortfolioController(
      Initialize(GetDefaultMarketDatabase()),
      &m_serviceClients.GetMarketDataClient(), orders);
    auto queue = std::make_shared<
      Queue<TestPortfolioController::UpdateEntry>>();
    controller.GetPublisher().Monitor(queue);
    auto orderA = PrimitiveOrder({OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
      Side::BID, "NYSE", 100, Money::ONE), 1,
      m_serviceClients.GetTimeClient().GetTime()});
    Accept(orderA);
    orders->Push(&orderA);
    m_environment.Publish(TST, BboQuote(Quote(2 * Money::ONE, 100, Side::BID),
      Quote(2 * Money::ONE, 100, Side::ASK), not_a_date_time));
    Fill(orderA, 100);
    {
      auto update = queue->Pop();
      REQUIRE(update.m_securityInventory.m_position.m_quantity == 100);
      REQUIRE(update.m_unrealizedSecurity == 100 * Money::ONE);
    }
    auto orderB = PrimitiveOrder({OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
      Side::ASK, "NYSE", 100, Money::ONE), 2,
      m_serviceClients.GetTimeClient().GetTime()});
    Accept(orderB);
    orders->Push(&orderB);
    Fill(orderB, 100);
    {
      auto update = queue->Pop();
      REQUIRE(update.m_securityInventory.m_position.m_quantity == 0);
      REQUIRE(update.m_unrealizedSecurity == Money::ZERO);
    }
    m_environment.Publish(TST, BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(Money::ONE, 100, Side::ASK), not_a_date_time));
    auto orderC = PrimitiveOrder({OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
      Side::ASK, "NYSE", 100, Money::ONE), 2,
      m_serviceClients.GetTimeClient().GetTime()});
    Accept(orderC);
    orders->Push(&orderC);
    Fill(orderC, 100);
    {
      auto update = queue->Pop();
      REQUIRE(update.m_securityInventory.m_position.m_quantity == -100);
    }
  }

  TEST_CASE_FIXTURE(Fixture, "initial_bookkeeper") {
    m_environment.Publish(TST, BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(Money::ONE, 100, Side::ASK), not_a_date_time));
    auto inventories = std::vector<TestPortfolio::Inventory>();
    auto tstInventory = TestPortfolio::Inventory(
      {TST, DefaultCurrencies::USD()});
    tstInventory.m_position.m_quantity = 600;
    tstInventory.m_position.m_costBasis = 1800 * Money::ONE;
    tstInventory.m_grossProfitAndLoss = -Money::CENT;
    tstInventory.m_fees = 12 * Money::ONE;
    tstInventory.m_transactionCount = 12;
    tstInventory.m_volume = 300;
    inventories.push_back(tstInventory);
    auto orders = std::make_shared<Queue<const Order*>>();
    auto controller = TestPortfolioController(Initialize(
      GetDefaultMarketDatabase(), TestPortfolio::Bookkeeper(inventories)),
      &m_serviceClients.GetMarketDataClient(), orders);
    auto queue = std::make_shared<
      Queue<TestPortfolioController::UpdateEntry>>();
    controller.GetPublisher().Monitor(queue);
    auto currencyInventory = tstInventory;
    currencyInventory.m_position.m_key.m_index = Security();
    auto receivedInventory = queue->Pop();
    REQUIRE(receivedInventory.m_securityInventory == tstInventory);
    REQUIRE(receivedInventory.m_currencyInventory == currencyInventory);
    REQUIRE(receivedInventory.m_unrealizedSecurity == -1200 * Money::ONE);
    REQUIRE(receivedInventory.m_unrealizedCurrency == -1200 * Money::ONE);
  }

  TEST_CASE_FIXTURE(Fixture, "empty_snapshot") {
    auto inventories = std::vector<TestPortfolio::Inventory>();
    inventories.push_back(
      TestPortfolio::Inventory({TST, DefaultCurrencies::USD()}));
    auto orders = std::make_shared<Queue<const Order*>>();
    auto controller = TestPortfolioController(Initialize(
      GetDefaultMarketDatabase(), TestPortfolio::Bookkeeper(inventories)),
      &m_serviceClients.GetMarketDataClient(), orders);
    auto queue = std::make_shared<
      Queue<TestPortfolioController::UpdateEntry>>();
    controller.GetPublisher().Monitor(queue);
    REQUIRE(!queue->TryPop());
  }
}
