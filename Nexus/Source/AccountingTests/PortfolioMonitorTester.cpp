#include <Beam/Queues/SequencePublisher.hpp>
#include <doctest/doctest.h>
#include "Nexus/Accounting/PortfolioMonitor.hpp"
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
  struct Fixture {
    using TestPortfolioMonitor = PortfolioMonitor<Portfolio<
      TrueAverageBookkeeper<Inventory<Position<Security>>>>,
      VirtualMarketDataClient*>;

    TestEnvironment m_environment;
    TestServiceClients m_serviceClients;

    Fixture()
        : m_serviceClients(Ref(m_environment)) {
      m_environment.SetTime(
        ptime(gregorian::date(2000, gregorian::Jan, 1), seconds(0)));
      m_environment.Open();
      m_serviceClients.Open();
    }
  };
}

TEST_SUITE("PortfolioMonitor") {
  TEST_CASE_FIXTURE(Fixture, "out_of_order_execution_reports") {
    auto security = Security("TST", DefaultMarkets::NYSE(),
      DefaultCountries::US());
    m_environment.Publish(security, BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(Money::ONE, 100, Side::ASK), not_a_date_time));
    auto orderPublisher = SequencePublisher<const Order*>();
    auto portfolioMonitor = TestPortfolioMonitor(
      Initialize(GetDefaultMarketDatabase()),
      &m_serviceClients.GetMarketDataClient(), orderPublisher);
    auto queue = std::make_shared<Queue<TestPortfolioMonitor::UpdateEntry>>();
    portfolioMonitor.GetPublisher().Monitor(queue);
    auto fieldsA = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "NYSE", 100, Money::CENT);
    auto orderA = PrimitiveOrder({fieldsA, 1,
      m_serviceClients.GetTimeClient().GetTime()});
    auto fieldsB = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::BID, "NYSE", 100, 2 * Money::CENT);
    m_environment.AdvanceTime(seconds(1));
    auto orderB = PrimitiveOrder({fieldsB, 2,
      m_serviceClients.GetTimeClient().GetTime()});
    auto fieldsC = OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
      Side::ASK, "NYSE", 100, 3 * Money::CENT);
    m_environment.AdvanceTime(seconds(1));
    auto orderC = PrimitiveOrder({fieldsC, 3,
      m_serviceClients.GetTimeClient().GetTime()});
    orderPublisher.Push(&orderA);
    orderPublisher.Push(&orderB);
    orderPublisher.Push(&orderC);
    auto date = m_serviceClients.GetTimeClient().GetTime().date();
    FillOrder(orderA, 100, ptime{date, seconds(1)});
    {
      auto update = queue->Top();
      queue->Pop();
      REQUIRE(update.m_securityInventory.m_position.m_quantity == 100);
      REQUIRE(GetAveragePrice(update.m_securityInventory.m_position) ==
        Money::CENT);
    }
    FillOrder(orderC, 100, ptime{date, seconds(2)});
    {
      auto update = queue->Top();
      queue->Pop();
      REQUIRE(update.m_securityInventory.m_position.m_quantity == 0);
      REQUIRE(GetAveragePrice(update.m_securityInventory.m_position) ==
        Money::ZERO);
    }
    FillOrder(orderB, 100, ptime{date, seconds(3)});
    {
      auto update = queue->Top();
      queue->Pop();
      REQUIRE(update.m_securityInventory.m_position.m_quantity == 100);
      REQUIRE(GetAveragePrice(update.m_securityInventory.m_position) ==
        2 * Money::CENT);
    }
    REQUIRE(queue->IsEmpty());
  }
}
