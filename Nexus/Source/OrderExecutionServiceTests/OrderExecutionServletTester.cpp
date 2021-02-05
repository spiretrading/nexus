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
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
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
  const auto TST = Security("TST", DefaultMarkets::NYSE(),
    DefaultCountries::US());

  struct Fixture {
    using TestServletContainer =
      TestAuthenticatedServiceProtocolServletContainer<
        MetaOrderExecutionServlet<IncrementalTimeClient,
          ServiceLocatorClientBox, UidClientBox, AdministrationClientBox,
          std::shared_ptr<MockOrderExecutionDriver>,
          std::shared_ptr<TestOrderExecutionDataStore>>>;
    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    UidServiceTestEnvironment m_uidServiceEnvironment;
    AdministrationServiceTestEnvironment m_administrationEnvironment;
    DirectoryEntry m_clientAccount;
    std::shared_ptr<MockOrderExecutionDriver> m_driver;
    std::shared_ptr<LocalOrderExecutionDataStore> m_baseDataStore;
    std::shared_ptr<TestOrderExecutionDataStore> m_dataStore;
    std::shared_ptr<TestServerConnection> m_serverConnection;
    optional<TestServletContainer> m_container;
    std::unique_ptr<TestServiceProtocolClient> m_protocolClient;
    std::shared_ptr<Queue<PrimitiveOrder*>> m_serverOrders;

    Fixture()
        : m_administrationEnvironment(MakeAdministrationServiceTestEnvironment(
            m_serviceLocatorEnvironment)) {
      m_driver = std::make_shared<MockOrderExecutionDriver>();
      m_serverOrders = std::make_shared<Queue<PrimitiveOrder*>>();
      m_driver->GetPublisher().Monitor(m_serverOrders);
      m_baseDataStore = std::make_shared<LocalOrderExecutionDataStore>();
      m_dataStore = std::make_shared<TestOrderExecutionDataStore>(
        OrderExecutionDataStoreBox(m_baseDataStore));
      m_serverConnection = std::make_shared<TestServerConnection>();
      auto orderExecutionAccount =
        m_serviceLocatorEnvironment.GetRoot().MakeAccount(
          "order_execution_service", "",
          m_administrationEnvironment.GetClient().LoadServicesRootEntry());
      m_administrationEnvironment.MakeAdministrator(orderExecutionAccount);
      auto clientEntry = m_serviceLocatorEnvironment.GetRoot().MakeAccount(
        "client", "", DirectoryEntry::GetStarDirectory());
      auto servletServiceLocatorClient = m_serviceLocatorEnvironment.MakeClient(
        "order_execution_service", "");
      m_container.emplace(Initialize(servletServiceLocatorClient,
        Initialize(pos_infin, GetDefaultMarketDatabase(),
          GetDefaultDestinationDatabase(), Initialize(),
          servletServiceLocatorClient, m_uidServiceEnvironment.MakeClient(),
          m_administrationEnvironment.MakeClient(servletServiceLocatorClient),
          m_driver, m_dataStore)), m_serverConnection,
        factory<std::unique_ptr<TriggerTimer>>());
      std::tie(m_protocolClient, m_clientAccount) = MakeClient("client");
      m_protocolClient->SendRequest<QueryOrderSubmissionsService>(
        MakeRealTimeQuery(m_clientAccount));
    }

    std::tuple<std::unique_ptr<TestServiceProtocolClient>, DirectoryEntry>
        MakeClient(const std::string& name) {
      auto serviceLocatorClient = m_serviceLocatorEnvironment.MakeClient(
        name, "");
      auto authenticator = SessionAuthenticator(serviceLocatorClient);
      auto protocolClient = std::make_unique<TestServiceProtocolClient>(
        Initialize(name, *m_serverConnection), Initialize());
      Nexus::Queries::RegisterQueryTypes(
        Store(protocolClient->GetSlots().GetRegistry()));
      RegisterOrderExecutionServices(Store(protocolClient->GetSlots()));
      RegisterOrderExecutionMessages(Store(protocolClient->GetSlots()));
      authenticator(*protocolClient);
      protocolClient->SpawnMessageHandler();
      return std::tuple(std::move(protocolClient),
        serviceLocatorClient.GetAccount());
    }
  };
}

TEST_SUITE("OrderExecutionServlet") {
  TEST_CASE_FIXTURE(Fixture, "new_order_single") {
    auto clientReportAsync = Async<ExecutionReport>();
    AddMessageSlot<OrderUpdateMessage>(Store(m_protocolClient->GetSlots()),
      [&] (auto& client, auto& report) {
        clientReportAsync.GetEval().SetResult(report);
      });
    auto orderFields = OrderFields::MakeLimitOrder(m_clientAccount, TST,
      DefaultCurrencies::USD(), Side::BID, "TEST", 100, Money::CENT);
    m_protocolClient->SendRequest<NewOrderSingleService>(orderFields);
    auto serverOrder = m_serverOrders->Pop();
    auto clientInitialReport = clientReportAsync.Get();
    clientReportAsync.Reset();
    REQUIRE(clientInitialReport.m_status == OrderStatus::PENDING_NEW);
    auto serverNewReport = ExecutionReport();
    serverNewReport.m_id = serverOrder->GetInfo().m_orderId;
    serverNewReport = ExecutionReport::MakeUpdatedReport(clientInitialReport,
      OrderStatus::NEW, microsec_clock::universal_time());
    serverOrder->Update(serverNewReport);
    auto clientNewReport = clientReportAsync.Get();
    clientReportAsync.Reset();
    REQUIRE(clientNewReport.m_status == OrderStatus::NEW);
    auto serverExpiredReport = ExecutionReport::MakeUpdatedReport(
      clientNewReport, OrderStatus::EXPIRED, microsec_clock::universal_time());
    serverOrder->Update(serverExpiredReport);
    auto clientExpiredReport = clientReportAsync.Get();
    REQUIRE(clientExpiredReport.m_status == OrderStatus::EXPIRED);
  }

  TEST_CASE_FIXTURE(Fixture, "update_order") {
    auto orderFields = OrderFields::MakeLimitOrder(TST, Side::BID, 100,
      Money::ONE);
    auto newOrder = m_protocolClient->SendRequest<NewOrderSingleService>(
      orderFields);
    auto serverOrder = m_serverOrders->Pop();
    auto serverReports = std::make_shared<Queue<ExecutionReport>>();
    serverOrder->GetPublisher().Monitor(serverReports);
    auto serverInitialReport = serverReports->Pop();
    auto newReport = ExecutionReport::MakeUpdatedReport(
      serverInitialReport, OrderStatus::NEW,
      time_from_string("2020-03-12 16:06:12"));
    REQUIRE_THROWS_AS(m_protocolClient->SendRequest<UpdateOrderService>(
      (*newOrder)->m_orderId, newReport), ServiceRequestException);
    auto [adminClient, adminAccount] = MakeClient("order_execution_service");
    adminClient->SendRequest<UpdateOrderService>(
      (*newOrder)->m_orderId, newReport);
    auto serverNewReport = serverReports->Pop();
    REQUIRE(serverNewReport == newReport);
  }

  TEST_CASE_FIXTURE(Fixture, "query_order_ids") {
    auto orderA = OrderInfo(OrderFields::MakeLimitOrder(TST, Side::BID, 100,
      Money::ONE), 12, time_from_string("2020-03-12 16:06:12"));
    m_dataStore->Store(SequencedValue(IndexedValue(orderA, m_clientAccount),
      Beam::Queries::Sequence(44)));
    auto orderB = OrderInfo(OrderFields::MakeLimitOrder(TST, Side::ASK, 200,
      Money::ONE), 17, time_from_string("2020-03-12 16:06:13"));
    m_dataStore->Store(SequencedValue(IndexedValue(orderB, m_clientAccount),
      Beam::Queries::Sequence(45)));
    auto orderC = OrderInfo(OrderFields::MakeLimitOrder(TST, Side::ASK, 300,
      Money::ONE), 36, time_from_string("2020-03-12 16:06:14"));
    m_dataStore->Store(SequencedValue(IndexedValue(orderC, m_clientAccount),
      Beam::Queries::Sequence(46)));
    auto query = AccountQuery();
    query.SetIndex(m_clientAccount);
    query.SetRange(Range::Historical());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto info = ParameterExpression(0, OrderInfoType());
    auto field = MemberAccessExpression("order_id", NativeDataType<OrderId>(),
      info);
    auto clauses = std::vector<Expression>();
    auto ids = std::vector<OrderId>();
    ids.push_back(12);
    ids.push_back(36);
    std::transform(ids.begin(), ids.end(), std::back_inserter(clauses),
      [&] (auto& id) {
        return MakeEqualsExpression(field, ConstantExpression(NativeValue(id)));
      });
    query.SetFilter(MakeOrExpression(clauses.begin(), clauses.end()));
    auto snapshot = m_protocolClient->SendRequest<QueryOrderSubmissionsService>(
      query);
    REQUIRE(snapshot.m_snapshot.size() == 2);
    REQUIRE(snapshot.m_snapshot[0]->m_info.m_orderId == 12);
    REQUIRE(snapshot.m_snapshot[1]->m_info.m_orderId == 36);
  }

  TEST_CASE_FIXTURE(Fixture, "load_order_without_permission") {
    auto account = DirectoryEntry::MakeAccount(599, "sephi");
    auto orderA = OrderInfo(OrderFields::MakeLimitOrder(TST, Side::BID, 100,
      Money::ONE), 12, time_from_string("2020-03-12 16:06:12"));
    m_dataStore->Store(SequencedValue(IndexedValue(orderA, account),
      Beam::Queries::Sequence(44)));
    auto order = m_protocolClient->SendRequest<LoadOrderByIdService>(12);
    REQUIRE(!order);
  }

  TEST_CASE_FIXTURE(Fixture, "load_order_with_permission") {
    auto orderA = OrderInfo(OrderFields::MakeLimitOrder(TST, Side::BID, 100,
      Money::ONE), 12, time_from_string("2020-03-12 16:06:12"));
    m_dataStore->Store(SequencedValue(IndexedValue(orderA, m_clientAccount),
      Beam::Queries::Sequence(44)));
    auto order = m_protocolClient->SendRequest<LoadOrderByIdService>(12);
    REQUIRE(order.has_value());
    REQUIRE((*order)->GetValue().m_info == orderA);
  }

  TEST_CASE_FIXTURE(Fixture, "load_order_with_updates") {
    auto reportAsync = Async<ExecutionReport>();
    AddMessageSlot<OrderUpdateMessage>(Store(m_protocolClient->GetSlots()),
      [&] (auto& client, auto& report) {
        reportAsync.GetEval().SetResult(report);
      });
    auto orderFields = OrderFields::MakeLimitOrder(TST,
      DefaultCurrencies::USD(), Side::BID, "TEST", 100, Money::CENT);
    auto newOrder = m_protocolClient->SendRequest<NewOrderSingleService>(
      orderFields);
    auto receivedOrder = m_serverOrders->Pop();
    auto initialReport = reportAsync.Get();
    REQUIRE(initialReport.m_status == OrderStatus::PENDING_NEW);
    m_dataStore->SetMode(TestOrderExecutionDataStore::Mode::SUPERVISED);
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestOrderExecutionDataStore::Operation>>>();
    m_dataStore->GetPublisher().Monitor(operations);
    auto orderId = (*newOrder)->m_orderId;
    auto loadResult = Async<optional<SequencedAccountOrderRecord>>();
    Spawn([&] {
      return m_protocolClient->SendRequest<LoadOrderByIdService>(orderId);
    }, loadResult.GetEval());
    auto loadOperation = std::dynamic_pointer_cast<
      TestOrderExecutionDataStore::LoadOrderOperation>(operations->Pop());
    REQUIRE(loadOperation);
    REQUIRE(*loadOperation->m_id == orderId);
    auto initialLoadResult = SequencedValue(IndexedValue(OrderRecord(OrderInfo(
      orderFields, m_clientAccount, orderId, false,
      time_from_string("2016-11-30 08:11:53")), std::vector{initialReport}),
      m_clientAccount), Beam::Queries::Sequence(417));
    loadOperation->m_result.SetResult(initialLoadResult);
    auto reloadOperation = std::dynamic_pointer_cast<
      TestOrderExecutionDataStore::LoadOrderOperation>(operations->Pop());
    REQUIRE(reloadOperation);
    REQUIRE(*reloadOperation->m_id == orderId);
    auto newReport = ExecutionReport::MakeUpdatedReport(initialReport,
      OrderStatus::NEW, time_from_string("2016-11-30 08:11:54"));
    receivedOrder->Update(newReport);
    auto storeOperation = std::dynamic_pointer_cast<
      TestOrderExecutionDataStore::StoreExecutionReportOperation>(
        operations->Pop());
    REQUIRE(storeOperation);
    REQUIRE((**storeOperation->m_executionReport)->m_status ==
      OrderStatus::NEW);
    storeOperation->m_result.SetResult();
    FlushPendingRoutines();
    reloadOperation->m_result.SetResult(initialLoadResult);
    auto receivedRecord = loadResult.Get();
    REQUIRE((**receivedRecord)->m_executionReports.size() == 2);
    m_dataStore->SetMode(TestOrderExecutionDataStore::Mode::UNSUPERVISED);
  }
}
