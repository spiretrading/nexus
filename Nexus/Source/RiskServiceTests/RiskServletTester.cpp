#include "Nexus/RiskServiceTests/RiskServletTester.hpp"
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

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
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::Queries;
using namespace Nexus::RiskService;
using namespace Nexus::RiskService::Tests;
using namespace std;

void RiskServletTester::setUp() {
  m_serviceLocatorInstance.Initialize();
  m_serviceLocatorInstance->Open();
  m_uidServiceInstance.Initialize();
  m_uidServiceInstance->Open();
  DirectoryEntry servicesDirectory =
    m_serviceLocatorInstance->GetRoot().MakeDirectory("services",
    DirectoryEntry::GetStarDirectory());
  DirectoryEntry administrationAccount =
    m_serviceLocatorInstance->GetRoot().MakeAccount("administration_service",
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
  auto marketDataServiceLocatorClient = m_serviceLocatorInstance->BuildClient();
  marketDataServiceLocatorClient->SetCredentials("root", "");
  marketDataServiceLocatorClient->Open();
  m_marketDataServiceInstance.Initialize(
    std::move(marketDataServiceLocatorClient));
  m_marketDataServiceInstance->Open();
  m_serviceLocatorInstance->GetRoot().MakeAccount("order_execution_service", "",
    servicesDirectory);
  auto orderExecutionServiceLocatorClient =
    m_serviceLocatorInstance->BuildClient();
  auto orderExecutionUidClient = m_uidServiceInstance->BuildClient();
  orderExecutionServiceLocatorClient->SetCredentials("order_execution_service",
    "");
  orderExecutionServiceLocatorClient->Open();
  auto orderExecutionAdministrationClient =
    m_administrationServiceInstance->BuildClient(
    Ref(*orderExecutionServiceLocatorClient));
  m_orderExecutionServiceInstance.Initialize(
    std::move(orderExecutionServiceLocatorClient),
    std::move(orderExecutionUidClient),
    std::move(orderExecutionAdministrationClient));
  m_orderExecutionServiceInstance->Open();
  m_serviceLocatorInstance->GetRoot().MakeAccount("trader", "",
    servicesDirectory);
  m_serviceLocatorInstance->GetRoot().MakeAccount("risk_service", "",
    servicesDirectory);
  auto riskServiceLocatorClient = m_serviceLocatorInstance->BuildClient();
  riskServiceLocatorClient->SetCredentials("risk_service", "");
  riskServiceLocatorClient->Open();
  std::unique_ptr<AdministrationClient> riskAdministrationClient =
    m_administrationServiceInstance->BuildClient(
    Ref(*riskServiceLocatorClient));
  m_marketDataClient = m_marketDataServiceInstance->BuildClient(
    Ref(*riskServiceLocatorClient));
  m_marketDataClient->Open();
  std::shared_ptr<OrderExecutionClient> riskOrderExecutionClient =
    m_orderExecutionServiceInstance->BuildClient(
    Ref(*riskServiceLocatorClient));
  riskOrderExecutionClient->Open();
  m_serverConnection.Initialize();
  m_transitionTimer.Initialize();
  AccountQuery orderSubmissionQuery;
  orderSubmissionQuery.SetIndex(DirectoryEntry::GetStarDirectory());
  orderSubmissionQuery.SetRange(Beam::Queries::Range::RealTime());
  m_orderSubmissionServletQueue = std::make_shared<Queue<const Order*>>();
  riskOrderExecutionClient->QueryOrderSubmissions(orderSubmissionQuery,
    m_orderSubmissionServletQueue);
  m_orderSubmissionTransitionQueue = std::make_shared<Queue<const Order*>>();
  riskOrderExecutionClient->QueryOrderSubmissions(orderSubmissionQuery,
    m_orderSubmissionTransitionQueue);
  m_container.Initialize(Initialize(std::move(riskServiceLocatorClient),
    Initialize(m_orderSubmissionServletQueue,
    std::move(riskAdministrationClient), riskOrderExecutionClient,
    Initialize(&*riskAdministrationClient, &*m_marketDataClient,
    m_orderSubmissionTransitionQueue, &*m_transitionTimer, Initialize(),
    GetDefaultMarketDatabase(), vector<ExchangeRate>()),
    GetDefaultDestinationDatabase(), GetDefaultMarketDatabase())),
    &*m_serverConnection, factory<std::shared_ptr<TriggerTimer>>());
  m_container->Open();
}

void RiskServletTester::tearDown() {
  m_orderSubmissionTransitionQueue.reset();
  m_orderSubmissionServletQueue.reset();
  m_container.Reset();
  m_serverConnection.Reset();
  m_transitionTimer.Reset();
  m_orderExecutionServiceInstance.Reset();
  m_marketDataClient.reset();
  m_marketDataServiceInstance.Reset();
  m_administrationServiceInstance.Reset();
  m_uidServiceInstance.Reset();
  m_serviceLocatorInstance.Reset();
}

void RiskServletTester::TestOpenPosition() {
/* TODO
  std::unique_ptr<OrderExecutionServiceTestInstance::ServiceLocatorClient>
    orderExecutionServiceLocatorClient =
    m_serviceLocatorInstance->BuildClient();
  orderExecutionServiceLocatorClient->Login("trader", "");
  std::unique_ptr<OrderExecutionServiceTestInstance::OrderExecutionClient>
    orderExecutionClient = m_orderExecutionServiceInstance->BuildClient(
    Ref(*orderExecutionServiceLocatorClient));
  orderExecutionClient->Open();
  Security security("A", DefaultMarkets::NYSE(), DefaultCountries::US());
  const Order& order = orderExecutionClient->Submit(
    OrderFields::BuildMarketOrder(
    orderExecutionServiceLocatorClient->GetAccount(), security,
    DefaultCurrencies::USD(), Side::BID, "NYSE", 100));
  PrimitiveOrder& serverOrder =
    m_orderExecutionServiceInstance->GetDriver().FindOrder(order.GetId());
  SetOrderStatus(serverOrder, OrderStatus::NEW,
    microsec_clock::universal_time());
  FillOrder(serverOrder, 100, microsec_clock::universal_time());
*/
}
