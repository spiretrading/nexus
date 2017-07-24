#include "Nexus/OrderExecutionServiceTests/OrderExecutionServletTester.hpp"
#include <boost/functional/factory.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::Queries;
using namespace std;

void OrderExecutionServletTester::setUp() {
  m_serviceLocatorEnvironment.emplace();
  m_serviceLocatorEnvironment->Open();
  auto servicesDirectory =
    m_serviceLocatorEnvironment->GetRoot().MakeDirectory("services",
    DirectoryEntry::GetStarDirectory());
  auto administratorsDirectory =
    m_serviceLocatorEnvironment->GetRoot().MakeDirectory("administrators",
    DirectoryEntry::GetStarDirectory());
  auto administrationAccount =
    m_serviceLocatorEnvironment->GetRoot().MakeAccount(
    "administration_service", "", servicesDirectory);
  m_serviceLocatorEnvironment->GetRoot().StorePermissions(
    administrationAccount, DirectoryEntry::GetStarDirectory(),
    Permissions(~0));
  m_serviceLocatorEnvironment->GetRoot().MakeAccount("order_execution_service",
    "", servicesDirectory);
  auto clientEntry = m_serviceLocatorEnvironment->GetRoot().MakeAccount(
    "client", "", DirectoryEntry::GetStarDirectory());
  m_uidServiceEnvironment.emplace();
  m_uidServiceEnvironment->Open();
  auto administationServiceLocatorClient =
    m_serviceLocatorEnvironment->BuildClient();
  administationServiceLocatorClient->SetCredentials("administration_service",
    "");
  administationServiceLocatorClient->Open();
  m_administrationServiceEnvironment.emplace(
    std::move(administationServiceLocatorClient));
  m_administrationServiceEnvironment->Open();
  std::shared_ptr<VirtualServiceLocatorClient> servletServiceLocatorClient =
    m_serviceLocatorEnvironment->BuildClient();
  servletServiceLocatorClient->SetCredentials("order_execution_service", "");
  servletServiceLocatorClient->Open();
  auto serverConnection = std::make_shared<TestServerConnection>();
  m_clientProtocol.emplace(Initialize("test", Ref(*serverConnection)),
    Initialize());
  RegisterQueryTypes(Store(m_clientProtocol->GetSlots().GetRegistry()));
  RegisterOrderExecutionServices(Store(m_clientProtocol->GetSlots()));
  RegisterOrderExecutionMessages(Store(m_clientProtocol->GetSlots()));
  m_driver = std::make_shared<MockOrderExecutionDriver>();
  m_dataStore = std::make_shared<LocalOrderExecutionDataStore>();
  m_container.emplace(Initialize(servletServiceLocatorClient,
    Initialize(pos_infin, GetDefaultMarketDatabase(),
    GetDefaultDestinationDatabase(), Initialize(), servletServiceLocatorClient,
    m_uidServiceEnvironment->BuildClient(),
    m_administrationServiceEnvironment->BuildClient(
    Ref(*servletServiceLocatorClient)), m_driver, m_dataStore)),
    serverConnection, factory<std::unique_ptr<TriggerTimer>>());
  m_container->Open();
  m_clientServiceLocatorClient = m_serviceLocatorEnvironment->BuildClient();
  m_clientServiceLocatorClient->SetCredentials("client", "");
  m_clientServiceLocatorClient->Open();
  m_clientProtocol->Open();
  SessionAuthenticator<VirtualServiceLocatorClient> authenticator{
    Ref(*m_clientServiceLocatorClient)};
  authenticator(*m_clientProtocol);
  m_clientProtocol->SpawnMessageHandler();
  AccountQuery orderSubmissionQuery;
  orderSubmissionQuery.SetIndex(m_clientServiceLocatorClient->GetAccount());
  orderSubmissionQuery.SetRange(Beam::Queries::Range::RealTime());
  m_clientProtocol->SendRequest<QueryOrderSubmissionsService>(
    orderSubmissionQuery);
}

void OrderExecutionServletTester::tearDown() {
  m_clientProtocol.reset();
  m_clientServiceLocatorClient.reset();
  m_container.reset();
  m_dataStore.reset();
  m_driver.reset();
  m_administrationServiceEnvironment.reset();
  m_uidServiceEnvironment.reset();
  m_serviceLocatorEnvironment.reset();
}

void OrderExecutionServletTester::TestNewOrderSingle() {
  auto orderFields = OrderFields::BuildLimitOrder(
    m_clientServiceLocatorClient->GetAccount(),
    Security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()},
    DefaultCurrencies::USD(), Side::BID, "TEST", 100, Money::CENT);
  ExecutionReport report;
  Async<void> messageAsync;
  AddMessageSlot<OrderUpdateMessage>(Store(m_clientProtocol->GetSlots()),
    [&] (auto& client, auto& receivedReport) {
      report = receivedReport;
      messageAsync.GetEval().SetResult();
    });
  auto newOrder = m_clientProtocol->SendRequest<NewOrderSingleService>(
    orderFields);
  auto driverMonitor = std::make_shared<Queue<PrimitiveOrder*>>();
  m_driver->GetPublisher().Monitor(driverMonitor);
  auto receivedOrder = driverMonitor->Top();
  driverMonitor->Pop();
  messageAsync.Get();
  messageAsync.Reset();
  CPPUNIT_ASSERT(report.m_status == OrderStatus::PENDING_NEW);
  ExecutionReport receivedOrderReport;
  receivedOrderReport.m_id = receivedOrder->GetInfo().m_orderId;
  receivedOrderReport = ExecutionReport::BuildUpdatedReport(report,
    OrderStatus::NEW, microsec_clock::universal_time());
  receivedOrder->Update(receivedOrderReport);
  messageAsync.Get();
  messageAsync.Reset();
  CPPUNIT_ASSERT(report.m_status == OrderStatus::NEW);
  receivedOrderReport = ExecutionReport::BuildUpdatedReport(report,
    OrderStatus::EXPIRED, microsec_clock::universal_time());
  receivedOrder->Update(receivedOrderReport);
  messageAsync.Get();
  CPPUNIT_ASSERT(report.m_status == OrderStatus::EXPIRED);
}
