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
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace std;

void BoardLotCheckTester::setUp() {
  m_environment.emplace();
  m_environment->Open();
  m_serviceClients.emplace(Ref(*m_environment));
  m_serviceClients->Open();
  m_check.emplace(&m_serviceClients->GetMarketDataClient(),
    GetDefaultMarketDatabase(), GetDefaultTimeZoneDatabase());
}

void BoardLotCheckTester::tearDown() {
  m_check.reset();
  m_serviceClients.reset();
  m_environment.reset();
}

void BoardLotCheckTester::TestUnavailableBboQuote() {
  Security security{"TST", DefaultMarkets::TSX(), DefaultCountries::CA()};
  OrderInfo orderInfoA{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::CAD(),
    Side::BID, "TSX", 100, Money::ONE), 1,
    m_environment->GetTimeEnvironment().GetTime()};
  PrimitiveOrder orderA{orderInfoA};
  CPPUNIT_ASSERT_THROW(m_check->Submit(orderInfoA),
    OrderSubmissionCheckException);
  m_environment->Publish(security,
    BboQuote{Quote{Money::ONE, 100, Side::BID},
    Quote{Money::ONE + Money::CENT, 100, Side::ASK}, not_a_date_time});
  CPPUNIT_ASSERT_NO_THROW(m_check->Submit(orderInfoA));
}
