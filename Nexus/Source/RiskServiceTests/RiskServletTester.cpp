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
  m_serviceLocatorEnvironment.Initialize();
  m_serviceLocatorEnvironment->Open();
  m_uidServiceEnvironment.Initialize();
  m_uidServiceEnvironment->Open();
  DirectoryEntry servicesDirectory =
    m_serviceLocatorEnvironment->GetRoot().MakeDirectory("services",
    DirectoryEntry::GetStarDirectory());
  DirectoryEntry administrationAccount =
    m_serviceLocatorEnvironment->GetRoot().MakeAccount("administration_service",
    "", servicesDirectory);
  m_serviceLocatorEnvironment->GetRoot().StorePermissions(administrationAccount,
    DirectoryEntry::GetStarDirectory(), Permissions(~0));
  auto administrationServiceLocatorClient =
    m_serviceLocatorEnvironment->BuildClient();
  administrationServiceLocatorClient->SetCredentials("administration_service",
    "");
  administrationServiceLocatorClient->Open();
  m_administrationServiceEnvironment.Initialize(
    std::move(administrationServiceLocatorClient));
  m_administrationServiceEnvironment->Open();
  auto marketDataServiceLocatorClient = m_serviceLocatorEnvironment->BuildClient();
  marketDataServiceLocatorClient->SetCredentials("root", "");
  marketDataServiceLocatorClient->Open();
  m_marketDataServiceEnvironment.Initialize(
    std::move(marketDataServiceLocatorClient));
  m_marketDataServiceEnvironment->Open();
  m_serviceLocatorEnvironment->GetRoot().MakeAccount("order_execution_service", "",
    servicesDirectory);
  auto orderExecutionServiceLocatorClient =
    m_serviceLocatorEnvironment->BuildClient();
  auto orderExecutionUidClient = m_uidServiceEnvironment->BuildClient();
  orderExecutionServiceLocatorClient->SetCredentials("order_execution_service",
    "");
  orderExecutionServiceLocatorClient->Open();
  auto orderExecutionAdministrationClient =
    m_administrationServiceEnvironment->BuildClient(
    Ref(*orderExecutionServiceLocatorClient));
  m_orderExecutionServiceEnvironment.Initialize(
    std::move(orderExecutionServiceLocatorClient),
    std::move(orderExecutionUidClient),
    std::move(orderExecutionAdministrationClient));
  m_orderExecutionServiceEnvironment->Open();
  m_serviceLocatorEnvironment->GetRoot().MakeAccount("trader", "",
    servicesDirectory);
  m_serviceLocatorEnvironment->GetRoot().MakeAccount("risk_service", "",
    servicesDirectory);
  auto riskServiceLocatorClient = m_serviceLocatorEnvironment->BuildClient();
  riskServiceLocatorClient->SetCredentials("risk_service", "");
  riskServiceLocatorClient->Open();
  std::unique_ptr<AdministrationClient> riskAdministrationClient =
    m_administrationServiceEnvironment->BuildClient(
    Ref(*riskServiceLocatorClient));
  m_marketDataClient = m_marketDataServiceEnvironment->BuildClient(
    Ref(*riskServiceLocatorClient));
  m_marketDataClient->Open();
  std::shared_ptr<OrderExecutionClient> riskOrderExecutionClient =
    m_orderExecutionServiceEnvironment->BuildClient(
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
  m_orderExecutionServiceEnvironment.Reset();
  m_marketDataClient.reset();
  m_marketDataServiceEnvironment.Reset();
  m_administrationServiceEnvironment.Reset();
  m_uidServiceEnvironment.Reset();
  m_serviceLocatorEnvironment.Reset();
}

void RiskServletTester::TestOpenPosition() {}
