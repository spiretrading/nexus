#include "Nexus/OrderExecutionServiceTests/BuyingPowerCheckTester.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

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
using namespace std;

void BuyingPowerCheckTester::setUp() {
  m_environment.emplace();
  m_environment->Open();
  m_serviceClients.emplace(Ref(*m_environment));
  m_serviceClients->Open();
  m_traderRiskParameters.m_currency = DefaultCurrencies::USD();
  m_traderRiskParameters.m_allowedState.m_type = RiskState::Type::ACTIVE;
  m_traderRiskParameters.m_buyingPower = 1000 * Money::ONE;
  m_serviceClients->GetAdministrationClient().StoreRiskParameters(
    DirectoryEntry::GetRootAccount(), m_traderRiskParameters);
  m_buyingPowerCheck.emplace(vector<ExchangeRate>{},
    &m_serviceClients->GetAdministrationClient(),
    &m_serviceClients->GetMarketDataClient());
}

void BuyingPowerCheckTester::tearDown() {
  m_buyingPowerCheck.reset();
  m_serviceClients.reset();
  m_environment.reset();
}

void BuyingPowerCheckTester::TestSubmission() {
  OrderExecutionSession session;
  Security security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
  m_environment->Update(security,
    BboQuote{Quote{Money::ONE, 100, Side::BID},
    Quote{Money::ONE + Money::CENT, 100, Side::ASK}, not_a_date_time});
  OrderInfo orderInfoA{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "NYSE", 100, Money::ONE), 1,
    m_environment->GetTimeEnvironment().GetTime()};
  PrimitiveOrder orderA{orderInfoA};
  CPPUNIT_ASSERT_NO_THROW(m_buyingPowerCheck->Submit(orderInfoA));
  m_buyingPowerCheck->Add(orderA);
  OrderInfo orderInfoB{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "NYSE", 1000, Money::ONE), 2,
    m_environment->GetTimeEnvironment().GetTime()};
  CPPUNIT_ASSERT_THROW(m_buyingPowerCheck->Submit(orderInfoB),
    OrderSubmissionCheckException);
}

void BuyingPowerCheckTester::TestAddWithoutSubmission() {
  OrderExecutionSession session;
  Security security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
  m_environment->Update(security,
    BboQuote{Quote{Money::ONE, 100, Side::BID},
    Quote{Money::ONE + Money::CENT, 100, Side::ASK}, not_a_date_time});
  OrderInfo orderInfoA{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "NYSE", 100, Money::ONE), 1,
    m_environment->GetTimeEnvironment().GetTime()};
  PrimitiveOrder orderA(orderInfoA);
  m_buyingPowerCheck->Add(orderA);
  OrderInfo orderInfoB{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "NYSE", 1000, Money::ONE), 2,
    m_environment->GetTimeEnvironment().GetTime()};
  CPPUNIT_ASSERT_THROW(m_buyingPowerCheck->Submit(orderInfoB),
    OrderSubmissionCheckException);
}

void BuyingPowerCheckTester::TestSubmissionThenRejection() {
  OrderExecutionSession session;
  Security security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
  m_environment->Update(security,
    BboQuote{Quote{Money::ONE, 100, Side::BID},
    Quote{Money::ONE + Money::CENT, 100, Side::ASK}, not_a_date_time});
  OrderInfo orderInfoA{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "NYSE", 100, Money::ONE), 1, not_a_date_time};
  PrimitiveOrder orderA{orderInfoA};
  CPPUNIT_ASSERT_NO_THROW(m_buyingPowerCheck->Submit(orderInfoA));
  m_buyingPowerCheck->Reject(orderInfoA);
  OrderInfo orderInfoB{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::USD(),
    Side::BID, "NYSE", 1000, Money::ONE), 2,
    m_environment->GetTimeEnvironment().GetTime()};
  CPPUNIT_ASSERT_NO_THROW(m_buyingPowerCheck->Submit(orderInfoB));
}
