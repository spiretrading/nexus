#include "Nexus/AccountingTests/PortfolioMonitorTester.hpp"
#include <Beam/Queues/SequencePublisher.hpp>
#include "Nexus/Accounting/PortfolioMonitor.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::Accounting::Tests;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace std;

void PortfolioMonitorTester::setUp() {
  m_environment.emplace();
  m_environment->SetTime(ptime{
    gregorian::date{2000, gregorian::Jan, 1}, seconds(0)});
  m_environment->Open();
  m_serviceClients.emplace(Ref(*m_environment));
  m_serviceClients->Open();
}

void PortfolioMonitorTester::tearDown() {
  m_serviceClients.reset();
  m_environment.reset();
}

void PortfolioMonitorTester::TestOutOfOrderExecutionReports() {
  Security security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
  m_environment->Publish(security, BboQuote{
    Quote{Money::ONE, 100, Side::BID}, Quote{Money::ONE, 100, Side::ASK},
    not_a_date_time});
  SequencePublisher<const Order*> orderPublisher;
  PortfolioMonitor portfolioMonitor{Initialize(GetDefaultMarketDatabase()),
    &m_serviceClients->GetMarketDataClient(), orderPublisher};
  auto queue = std::make_shared<Queue<PortfolioMonitor::UpdateEntry>>();
  portfolioMonitor.GetPublisher().Monitor(queue);
  auto fieldsA = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
    security, DefaultCurrencies::USD(), Side::BID, "NYSE", 100, Money::CENT);
  PrimitiveOrder orderA{{fieldsA, 1,
    m_serviceClients->GetTimeClient().GetTime()}};
  auto fieldsB = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
    security, DefaultCurrencies::USD(), Side::BID, "NYSE", 100,
    2 * Money::CENT);
  m_environment->AdvanceTime(seconds(1));
  PrimitiveOrder orderB{{fieldsB, 2,
    m_serviceClients->GetTimeClient().GetTime()}};
  auto fieldsC = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
    security, DefaultCurrencies::USD(), Side::ASK, "NYSE", 100,
    3 * Money::CENT);
  m_environment->AdvanceTime(seconds(1));
  PrimitiveOrder orderC{{fieldsC, 3,
    m_serviceClients->GetTimeClient().GetTime()}};
  orderPublisher.Push(&orderA);
  orderPublisher.Push(&orderB);
  orderPublisher.Push(&orderC);
  auto date = m_serviceClients->GetTimeClient().GetTime().date();
  FillOrder(orderA, 100, ptime{date, seconds(1)});
  {
    auto update = queue->Top();
    queue->Pop();
    CPPUNIT_ASSERT(update.m_securityInventory.m_position.m_quantity == 100);
    CPPUNIT_ASSERT(GetAveragePrice(update.m_securityInventory.m_position) ==
      Money::CENT);
  }
  FillOrder(orderC, 100, ptime{date, seconds(2)});
  {
    auto update = queue->Top();
    queue->Pop();
    CPPUNIT_ASSERT(update.m_securityInventory.m_position.m_quantity == 0);
    CPPUNIT_ASSERT(GetAveragePrice(update.m_securityInventory.m_position) ==
      Money::ZERO);
  }
  FillOrder(orderB, 100, ptime{date, seconds(3)});
  {
    auto update = queue->Top();
    queue->Pop();
    CPPUNIT_ASSERT(update.m_securityInventory.m_position.m_quantity == 100);
    CPPUNIT_ASSERT(GetAveragePrice(update.m_securityInventory.m_position) ==
      2 * Money::CENT);
  }
  CPPUNIT_ASSERT(queue->IsEmpty());
}
