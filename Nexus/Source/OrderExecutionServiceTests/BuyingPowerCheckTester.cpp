#include "Nexus/OrderExecutionServiceTests/BuyingPowerCheckTester.hpp"
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::UidService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::RiskService;
using namespace std;

void BuyingPowerCheckTester::setUp() {
  m_serviceLocatorInstance.Initialize();
  m_serviceLocatorInstance->Open();
  m_uidServiceInstance.Initialize();
  m_uidServiceInstance->Open();
  auto servicesDirectory = m_serviceLocatorInstance->GetRoot().MakeDirectory(
    "services", DirectoryEntry::GetStarDirectory());
  auto administrationAccount = m_serviceLocatorInstance->GetRoot().MakeAccount(
    "administration_service", "", servicesDirectory);
  m_traderAccount = m_serviceLocatorInstance->GetRoot().MakeAccount("trader",
    "", servicesDirectory);
  m_serviceLocatorInstance->GetRoot().StorePermissions(administrationAccount,
    DirectoryEntry::GetStarDirectory(), Permissions(~0));
  auto administrationServiceLocatorClient =
    m_serviceLocatorInstance->BuildClient();
  administrationServiceLocatorClient->SetCredentials("administration_service",
    "");
  administrationServiceLocatorClient->Open();
  m_administrationServiceInstance.Initialize(
    std::move(administrationServiceLocatorClient));
  m_administrationServiceInstance->Open();
  auto administratorsDirectory =
    m_serviceLocatorInstance->GetRoot().LoadDirectoryEntry(
    DirectoryEntry::GetStarDirectory(), "administrators");
  auto marketDataServiceLocatorClient = m_serviceLocatorInstance->BuildClient();
  marketDataServiceLocatorClient->SetCredentials("root", "");
  marketDataServiceLocatorClient->Open();
  m_marketDataServiceInstance.Initialize(
    std::move(marketDataServiceLocatorClient));
  m_marketDataServiceInstance->Open();
  auto orderExecutionServiceAccount =
    m_serviceLocatorInstance->GetRoot().MakeAccount("order_execution_service",
    "", servicesDirectory);
  m_serviceLocatorInstance->GetRoot().Associate(orderExecutionServiceAccount,
    administratorsDirectory);
  m_serviceLocatorClient = m_serviceLocatorInstance->BuildClient();
  auto orderExecutionUidClient = m_uidServiceInstance->BuildClient();
  m_serviceLocatorClient->SetCredentials("order_execution_service", "");
  m_serviceLocatorClient->Open();
  auto administrationClient = m_administrationServiceInstance->BuildClient(
    Ref(*m_serviceLocatorClient));
  administrationClient->Open();
  m_traderRiskParameters.m_currency = DefaultCurrencies::USD();
  m_traderRiskParameters.m_allowedState.m_type = RiskState::Type::ACTIVE;
  m_traderRiskParameters.m_buyingPower = 1000 * Money::ONE;
  administrationClient->StoreRiskParameters(m_traderAccount,
    m_traderRiskParameters);
  auto marketDataClient = m_marketDataServiceInstance->BuildClient(
    Ref(*m_serviceLocatorClient));
  marketDataClient->Open();
  m_buyingPowerCheck.Initialize(vector<ExchangeRate>(),
    std::move(administrationClient), std::move(marketDataClient));
}

void BuyingPowerCheckTester::tearDown() {
  m_buyingPowerCheck.Reset();
  m_serviceLocatorClient.reset();
  m_marketDataServiceInstance.Reset();
  m_administrationServiceInstance.Reset();
  m_uidServiceInstance.Reset();
  m_serviceLocatorInstance.Reset();
}

void BuyingPowerCheckTester::TestSubmission() {
  OrderExecutionSession session;
  Security security("TST", DefaultMarkets::NYSE(), DefaultCountries::US());
  m_marketDataServiceInstance->SetBbo(security,
    BboQuote(Quote(Money::ONE, 100, Side::BID),
    Quote(Money::ONE + Money::CENT, 100, Side::ASK),
    second_clock::universal_time()));
  OrderInfo orderInfoA{OrderFields::BuildLimitOrder(m_traderAccount,
    security, DefaultCurrencies::USD(), Side::BID, "NYSE", 100, Money::ONE), 1,
    second_clock::universal_time()};
  PrimitiveOrder orderA(orderInfoA);
  CPPUNIT_ASSERT_NO_THROW(m_buyingPowerCheck->Submit(orderInfoA));
  m_buyingPowerCheck->Add(orderA);
  OrderInfo orderInfoB{OrderFields::BuildLimitOrder(m_traderAccount,
    security, DefaultCurrencies::USD(), Side::BID, "NYSE", 1000, Money::ONE), 2,
    second_clock::universal_time()};
  CPPUNIT_ASSERT_THROW(m_buyingPowerCheck->Submit(orderInfoB),
    OrderSubmissionCheckException);
}

void BuyingPowerCheckTester::TestAddWithoutSubmission() {
  OrderExecutionSession session;
  Security security("TST", DefaultMarkets::NYSE(), DefaultCountries::US());
  m_marketDataServiceInstance->SetBbo(security,
    BboQuote(Quote(Money::ONE, 100, Side::BID),
    Quote(Money::ONE + Money::CENT, 100, Side::ASK),
    second_clock::universal_time()));
  OrderInfo orderInfoA{OrderFields::BuildLimitOrder(m_traderAccount,
    security, DefaultCurrencies::USD(), Side::BID, "NYSE", 100, Money::ONE), 1,
    second_clock::universal_time()};
  PrimitiveOrder orderA(orderInfoA);
  m_buyingPowerCheck->Add(orderA);
  OrderInfo orderInfoB{OrderFields::BuildLimitOrder(m_traderAccount,
    security, DefaultCurrencies::USD(), Side::BID, "NYSE", 1000, Money::ONE), 2,
    second_clock::universal_time()};
  CPPUNIT_ASSERT_THROW(m_buyingPowerCheck->Submit(orderInfoB),
    OrderSubmissionCheckException);
}

void BuyingPowerCheckTester::TestSubmissionThenRejection() {
  OrderExecutionSession session;
  Security security("TST", DefaultMarkets::NYSE(), DefaultCountries::US());
  m_marketDataServiceInstance->SetBbo(security,
    BboQuote(Quote(Money::ONE, 100, Side::BID),
    Quote(Money::ONE + Money::CENT, 100, Side::ASK),
    second_clock::universal_time()));
  OrderInfo orderInfoA{OrderFields::BuildLimitOrder(m_traderAccount,
    security, DefaultCurrencies::USD(), Side::BID, "NYSE", 100, Money::ONE), 1,
    second_clock::universal_time()};
  PrimitiveOrder orderA(orderInfoA);
  CPPUNIT_ASSERT_NO_THROW(m_buyingPowerCheck->Submit(orderInfoA));
  m_buyingPowerCheck->Reject(orderInfoA);
  OrderInfo orderInfoB{OrderFields::BuildLimitOrder(m_traderAccount,
    security, DefaultCurrencies::USD(), Side::BID, "NYSE", 1000, Money::ONE), 2,
    second_clock::universal_time()};
  CPPUNIT_ASSERT_NO_THROW(m_buyingPowerCheck->Submit(orderInfoB));
}
