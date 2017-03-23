#include "Nexus/OrderExecutionServiceTests/BoardLotCheckTester.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace std;

void BoardLotCheckTester::setUp() {
  m_serviceLocatorInstance.emplace();
  m_serviceLocatorInstance->Open();
  auto marketDataServiceLocatorClient = m_serviceLocatorInstance->BuildClient();
  marketDataServiceLocatorClient->SetCredentials("root", "");
  marketDataServiceLocatorClient->Open();
  m_marketDataServiceInstance.emplace(
    std::move(marketDataServiceLocatorClient));
  m_marketDataServiceInstance->Open();
  m_serviceLocatorClient = m_serviceLocatorInstance->BuildClient();
  m_serviceLocatorClient->SetCredentials("root", "");
  m_serviceLocatorClient->Open();
  auto marketDataClient = m_marketDataServiceInstance->BuildClient(
    Ref(*m_serviceLocatorClient));
  marketDataClient->Open();
  m_check.emplace(std::move(marketDataClient), std::make_unique<TimeClient>(),
    GetDefaultMarketDatabase(), GetDefaultTimeZoneDatabase());
}

void BoardLotCheckTester::tearDown() {
  m_check.reset();
  m_serviceLocatorClient.reset();
  m_marketDataServiceInstance.reset();
  m_serviceLocatorInstance.reset();
}

void BoardLotCheckTester::TestUnavailableBboQuote() {
  Security security{"TST", DefaultMarkets::TSX(), DefaultCountries::CA()};
  OrderInfo orderInfoA{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::CAD(),
    Side::BID, "TSX", 100, Money::ONE), 1, second_clock::universal_time()};
  PrimitiveOrder orderA{orderInfoA};
  CPPUNIT_ASSERT_THROW(m_check->Submit(orderInfoA),
    OrderSubmissionCheckException);
  m_marketDataServiceInstance->SetBbo(security,
    BboQuote{Quote{Money::ONE, 100, Side::BID},
    Quote{Money::ONE + Money::CENT, 100, Side::ASK},
    second_clock::universal_time()});
  CPPUNIT_ASSERT_NO_THROW(m_check->Submit(orderInfoA));
}
