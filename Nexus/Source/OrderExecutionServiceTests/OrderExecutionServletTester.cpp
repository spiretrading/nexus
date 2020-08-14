#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <boost/functional/factory.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServlet.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::UidService;
using namespace Beam::UidService::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::Queries;

namespace {
  struct Fixture {
    using TestServletContainer =
      TestAuthenticatedServiceProtocolServletContainer<
      MetaOrderExecutionServlet<IncrementalTimeClient,
      std::shared_ptr<VirtualServiceLocatorClient>,
      std::unique_ptr<VirtualUidClient>,
      std::unique_ptr<VirtualAdministrationClient>,
      std::shared_ptr<MockOrderExecutionDriver>,
      std::shared_ptr<LocalOrderExecutionDataStore>>>;

    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    UidServiceTestEnvironment m_uidServiceEnvironment;
    boost::optional<AdministrationServiceTestEnvironment>
      m_administrationServiceEnvironment;
    std::unique_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
      m_clientServiceLocatorClient;
    std::shared_ptr<MockOrderExecutionDriver> m_driver;
    std::shared_ptr<LocalOrderExecutionDataStore> m_dataStore;
    boost::optional<TestServletContainer> m_container;
    boost::optional<Beam::Services::Tests::TestServiceProtocolClient>
      m_clientProtocol;

    Fixture() {
      m_serviceLocatorEnvironment.Open();
      auto servicesDirectory =
        m_serviceLocatorEnvironment.GetRoot().MakeDirectory("services",
        DirectoryEntry::GetStarDirectory());
      auto administratorsDirectory =
        m_serviceLocatorEnvironment.GetRoot().MakeDirectory("administrators",
        DirectoryEntry::GetStarDirectory());
      auto administrationAccount =
        m_serviceLocatorEnvironment.GetRoot().MakeAccount(
        "administration_service", "", servicesDirectory);
      m_serviceLocatorEnvironment.GetRoot().StorePermissions(
        administrationAccount, DirectoryEntry::GetStarDirectory(),
        Permissions(~0));
      m_serviceLocatorEnvironment.GetRoot().MakeAccount(
        "order_execution_service", "", servicesDirectory);
      auto clientEntry = m_serviceLocatorEnvironment.GetRoot().MakeAccount(
        "client", "", DirectoryEntry::GetStarDirectory());
      m_uidServiceEnvironment.Open();
      auto administationServiceLocatorClient =
        m_serviceLocatorEnvironment.BuildClient();
      administationServiceLocatorClient->SetCredentials(
        "administration_service", "");
      administationServiceLocatorClient->Open();
      m_administrationServiceEnvironment.emplace(
        std::move(administationServiceLocatorClient));
      m_administrationServiceEnvironment->Open();
      auto servletServiceLocatorClient =
        std::shared_ptr(m_serviceLocatorEnvironment.BuildClient());
      servletServiceLocatorClient->SetCredentials("order_execution_service",
        "");
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
        GetDefaultDestinationDatabase(), Initialize(),
        servletServiceLocatorClient, m_uidServiceEnvironment.BuildClient(),
        m_administrationServiceEnvironment->BuildClient(
        Ref(*servletServiceLocatorClient)), m_driver, m_dataStore)),
        serverConnection, factory<std::unique_ptr<TriggerTimer>>());
      m_container->Open();
      m_clientServiceLocatorClient = m_serviceLocatorEnvironment.BuildClient();
      m_clientServiceLocatorClient->SetCredentials("client", "");
      m_clientServiceLocatorClient->Open();
      m_clientProtocol->Open();
      auto authenticator = SessionAuthenticator(
        Ref(*m_clientServiceLocatorClient));
      authenticator(*m_clientProtocol);
      m_clientProtocol->SpawnMessageHandler();
      auto orderSubmissionQuery = AccountQuery();
      orderSubmissionQuery.SetIndex(m_clientServiceLocatorClient->GetAccount());
      orderSubmissionQuery.SetRange(Beam::Queries::Range::RealTime());
      m_clientProtocol->SendRequest<QueryOrderSubmissionsService>(
        orderSubmissionQuery);
    }
  };
}

TEST_SUITE("OrderExecutionServlet") {
  TEST_CASE_FIXTURE(Fixture, "new_order_single") {
    auto orderFields = OrderFields::BuildLimitOrder(
      m_clientServiceLocatorClient->GetAccount(),
      Security("TST", DefaultMarkets::NYSE(), DefaultCountries::US()),
      DefaultCurrencies::USD(), Side::BID, "TEST", 100, Money::CENT);
    auto report = ExecutionReport();
    auto messageAsync = Async<void>();
    AddMessageSlot<OrderUpdateMessage>(Store(m_clientProtocol->GetSlots()),
      [&] (auto& client, auto& receivedReport) {
        report = receivedReport;
        messageAsync.GetEval().SetResult();
      });
    auto newOrder = m_clientProtocol->SendRequest<NewOrderSingleService>(
      orderFields);
    auto driverMonitor = std::make_shared<Queue<PrimitiveOrder*>>();
    m_driver->GetPublisher().Monitor(driverMonitor);
    auto receivedOrder = driverMonitor->Pop();
    messageAsync.Get();
    messageAsync.Reset();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    auto receivedOrderReport = ExecutionReport();
    receivedOrderReport.m_id = receivedOrder->GetInfo().m_orderId;
    receivedOrderReport = ExecutionReport::BuildUpdatedReport(report,
      OrderStatus::NEW, microsec_clock::universal_time());
    receivedOrder->Update(receivedOrderReport);
    messageAsync.Get();
    messageAsync.Reset();
    REQUIRE(report.m_status == OrderStatus::NEW);
    receivedOrderReport = ExecutionReport::BuildUpdatedReport(report,
      OrderStatus::EXPIRED, microsec_clock::universal_time());
    receivedOrder->Update(receivedOrderReport);
    messageAsync.Get();
    REQUIRE(report.m_status == OrderStatus::EXPIRED);
  }
}
