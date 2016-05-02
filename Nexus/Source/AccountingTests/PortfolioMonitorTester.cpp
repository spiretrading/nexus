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
  m_serviceLocatorInstance.Initialize();
  m_serviceLocatorInstance->Open();
  m_serviceLocatorClient = m_serviceLocatorInstance->BuildClient();
  m_serviceLocatorClient->SetCredentials("root", "");
  m_serviceLocatorClient->Open();
  auto servicesDirectory = m_serviceLocatorInstance->GetRoot().MakeDirectory(
    "services", DirectoryEntry::GetStarDirectory());
  auto marketDataServiceLocatorClient = m_serviceLocatorInstance->BuildClient();
  marketDataServiceLocatorClient->SetCredentials("root", "");
  marketDataServiceLocatorClient->Open();
  m_marketDataServiceInstance.Initialize(
    std::move(marketDataServiceLocatorClient));
  m_marketDataServiceInstance->Open();
}

void PortfolioMonitorTester::tearDown() {
  m_marketDataServiceInstance.Reset();
  m_serviceLocatorClient.reset();
  m_serviceLocatorInstance.Reset();
}

void PortfolioMonitorTester::TestOutOfOrderExecutionReports() {
  Security security("TST", DefaultMarkets::NYSE(), DefaultCountries::US());
  m_marketDataServiceInstance->SetBbo(security, BboQuote(
    Quote(Money::ONE, 100, Side::BID), Quote(Money::ONE, 100, Side::ASK),
    second_clock::local_time()));
  SequencePublisher<const Order*> orderPublisher;
  auto marketDataClient = m_marketDataServiceInstance->BuildClient(
    Ref(*m_serviceLocatorClient));
  marketDataClient->Open();
  PortfolioMonitor portfolioMonitor(Initialize(GetDefaultMarketDatabase()),
    std::move(marketDataClient), orderPublisher);
  auto queue = std::make_shared<Queue<PortfolioMonitor::UpdateEntry>>();
  portfolioMonitor.GetPublisher().Monitor(queue);
  gregorian::date date(2000, gregorian::Jan, 1);
  auto fieldsA = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
    security, DefaultCurrencies::USD(), Side::BID, "NYSE", 100, Money::CENT);
  PrimitiveOrder orderA{{fieldsA, 1, ptime{date, seconds(1)}}};
  auto fieldsB = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
    security, DefaultCurrencies::USD(), Side::BID, "NYSE", 100,
    2 * Money::CENT);
  PrimitiveOrder orderB{{fieldsB, 2, ptime{date, seconds(2)}}};
  auto fieldsC = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
    security, DefaultCurrencies::USD(), Side::ASK, "NYSE", 100,
    3 * Money::CENT);
  PrimitiveOrder orderC{{fieldsC, 3, ptime{date, seconds(3)}}};
  orderPublisher.Push(&orderA);
  orderPublisher.Push(&orderB);
  orderPublisher.Push(&orderC);
  FillOrder(orderA, 100, ptime(date, seconds(1)));
  {
    auto update = queue->Top();
    queue->Pop();
    CPPUNIT_ASSERT(update.m_securityInventory.m_position.m_quantity == 100);
    CPPUNIT_ASSERT(GetAveragePrice(update.m_securityInventory.m_position) ==
      Money::CENT);
  }
  FillOrder(orderC, 100, ptime(date, seconds(2)));
  {
    auto update = queue->Top();
    queue->Pop();
    CPPUNIT_ASSERT(update.m_securityInventory.m_position.m_quantity == 0);
    CPPUNIT_ASSERT(GetAveragePrice(update.m_securityInventory.m_position) ==
      Money::ZERO);
  }
  FillOrder(orderB, 100, ptime(date, seconds(3)));
  {
    auto update = queue->Top();
    queue->Pop();
    CPPUNIT_ASSERT(update.m_securityInventory.m_position.m_quantity == 100);
    CPPUNIT_ASSERT(GetAveragePrice(update.m_securityInventory.m_position) ==
      2 * Money::CENT);
  }
  CPPUNIT_ASSERT(queue->IsEmpty());
}
