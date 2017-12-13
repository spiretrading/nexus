#include "Nexus/OrderExecutionServiceTests/BuyingPowerCheckTester.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::RiskService;
using namespace std;

namespace {
  Security TST{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
}

void BuyingPowerCheckTester::setUp() {
  m_environment.emplace();
  m_environment->Open();
  m_orderSubmissions = std::make_shared<Queue<const Order*>>();
  m_environment->MonitorOrderSubmissions(m_orderSubmissions);
  m_environment->UpdateBboPrice(TST, Money::ONE, Money::ONE + Money::CENT);
  m_serviceClients.emplace(Ref(*m_environment));
  m_serviceClients->Open();
  m_traderRiskParameters.m_currency = DefaultCurrencies::USD();
  m_traderRiskParameters.m_allowedState.m_type = RiskState::Type::ACTIVE;
  m_traderRiskParameters.m_buyingPower = 1000 * Money::ONE;
  m_environment->GetAdministrationEnvironment().MakeAdministrator(
    m_serviceClients->GetServiceLocatorClient().GetAccount());
  m_serviceClients->GetAdministrationClient().StoreRiskParameters(
    DirectoryEntry::GetRootAccount(), m_traderRiskParameters);
  m_buyingPowerCheck.emplace(vector<ExchangeRate>{},
    &m_serviceClients->GetAdministrationClient(),
    &m_serviceClients->GetMarketDataClient());
}

void BuyingPowerCheckTester::tearDown() {
  m_buyingPowerCheck.reset();
  m_serviceClients.reset();
  m_orderSubmissions.reset();
  m_environment.reset();
}

void BuyingPowerCheckTester::TestSubmission() {
  OrderInfo orderInfoA{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
    Side::BID, "NYSE", 100, Money::ONE), 1,
    m_environment->GetTimeEnvironment().GetTime()};
  PrimitiveOrder orderA{orderInfoA};
  CPPUNIT_ASSERT_NO_THROW(m_buyingPowerCheck->Submit(orderInfoA));
  m_buyingPowerCheck->Add(orderA);
  OrderInfo orderInfoB{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
    Side::BID, "NYSE", 1000, Money::ONE), 2,
    m_environment->GetTimeEnvironment().GetTime()};
  CPPUNIT_ASSERT_THROW(m_buyingPowerCheck->Submit(orderInfoB),
    OrderSubmissionCheckException);
}

void BuyingPowerCheckTester::TestAddWithoutSubmission() {
  OrderInfo orderInfoA{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
    Side::BID, "NYSE", 100, Money::ONE), 1,
    m_environment->GetTimeEnvironment().GetTime()};
  PrimitiveOrder orderA(orderInfoA);
  m_buyingPowerCheck->Add(orderA);
  OrderInfo orderInfoB{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
    Side::BID, "NYSE", 1000, Money::ONE), 2,
    m_environment->GetTimeEnvironment().GetTime()};
  CPPUNIT_ASSERT_THROW(m_buyingPowerCheck->Submit(orderInfoB),
    OrderSubmissionCheckException);
}

void BuyingPowerCheckTester::TestSubmissionThenRejection() {
  OrderInfo orderInfoA{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
    Side::BID, "NYSE", 100, Money::ONE), 1, not_a_date_time};
  PrimitiveOrder orderA{orderInfoA};
  CPPUNIT_ASSERT_NO_THROW(m_buyingPowerCheck->Submit(orderInfoA));
  m_buyingPowerCheck->Reject(orderInfoA);
  OrderInfo orderInfoB{OrderFields::BuildLimitOrder(
    DirectoryEntry::GetRootAccount(), TST, DefaultCurrencies::USD(),
    Side::BID, "NYSE", 1000, Money::ONE), 2,
    m_environment->GetTimeEnvironment().GetTime()};
  CPPUNIT_ASSERT_NO_THROW(m_buyingPowerCheck->Submit(orderInfoB));
}

void BuyingPowerCheckTester::TestOrderRecovery() {
  auto recoveryFields = OrderFields::BuildLimitOrder(TST, Side::BID, 100,
    Money::ONE);
  auto& recoverOrder = m_serviceClients->GetOrderExecutionClient().Submit(
    recoveryFields);
  auto submittedRecoveryOrder = m_orderSubmissions->Top();
  m_orderSubmissions->Pop();
  m_environment->AcceptOrder(*submittedRecoveryOrder);
  m_environment->FillOrder(*submittedRecoveryOrder, 100);
  CPPUNIT_ASSERT_NO_THROW(m_buyingPowerCheck->Add(*submittedRecoveryOrder));
  auto submissionFields = OrderFields::BuildLimitOrder(TST, Side::BID, 100,
    2 * Money::ONE);
  auto& submissionOrder = m_serviceClients->GetOrderExecutionClient().Submit(
    submissionFields);
  auto submittedOrder = m_orderSubmissions->Top();
  m_orderSubmissions->Pop();
  CPPUNIT_ASSERT_NO_THROW(m_buyingPowerCheck->Submit(
    submittedOrder->GetInfo()));
}
