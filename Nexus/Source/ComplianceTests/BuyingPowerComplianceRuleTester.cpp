#include "Nexus/ComplianceTests/BuyingPowerComplianceRuleTester.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::Compliance::Tests;
using namespace Nexus::OrderExecutionService;
using namespace std;

namespace {
  Security TST{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
}

void BuyingPowerComplianceRuleTester::setUp() {
  m_testEnvironment.emplace();
  m_orderSubmissions = std::make_shared<Queue<const Order*>>();
  m_testEnvironment->Open();
  m_testEnvironment->MonitorOrderSubmissions(m_orderSubmissions);
  m_testEnvironment->UpdateBboPrice(TST, Money::ONE, Money::ONE + Money::CENT);
  m_serviceClients.emplace(Ref(*m_testEnvironment));
  m_serviceClients->Open();
}

void BuyingPowerComplianceRuleTester::tearDown() {
  m_serviceClients.reset();
  m_orderSubmissions.reset();
  m_testEnvironment.reset();
}

void BuyingPowerComplianceRuleTester::TestOrderRecovery() {
  vector<ComplianceParameter> parameters;
  parameters.emplace_back("currency", DefaultCurrencies::USD());
  parameters.emplace_back("buying_power", 1000 * Money::ONE);
  std::vector<ComplianceValue> symbols;
  symbols.push_back(SecuritySet::GetSecurityWildCard());
  parameters.emplace_back("symbols", symbols);
  BuyingPowerComplianceRule rule{parameters, vector<ExchangeRate>{},
    &m_serviceClients->GetMarketDataClient()};
  auto recoveryFields = OrderFields::BuildLimitOrder(TST, Side::BID, 100,
    Money::ONE);
  auto& recoverOrder = m_serviceClients->GetOrderExecutionClient().Submit(
    recoveryFields);
  auto submittedRecoveryOrder = m_orderSubmissions->Top();
  m_orderSubmissions->Pop();
  m_testEnvironment->AcceptOrder(*submittedRecoveryOrder);
  m_testEnvironment->FillOrder(*submittedRecoveryOrder, 100);
  CPPUNIT_ASSERT_NO_THROW(rule.Add(*submittedRecoveryOrder));
  auto submissionFields = OrderFields::BuildLimitOrder(TST, Side::BID, 100,
    2 * Money::ONE);
  auto& submissionOrder = m_serviceClients->GetOrderExecutionClient().Submit(
    submissionFields);
  auto submittedOrder = m_orderSubmissions->Top();
  m_orderSubmissions->Pop();
  CPPUNIT_ASSERT_NO_THROW(rule.Submit(*submittedOrder));
}
