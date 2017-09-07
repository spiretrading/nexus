#include "Nexus/OrderExecutionServiceTests/OrderExecutionClientTester.hpp"
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::Queries;
using namespace std;

void OrderExecutionClientTester::setUp() {
  auto serverConnection = std::make_shared<TestServerConnection>();
  m_server.emplace(serverConnection, factory<std::unique_ptr<TriggerTimer>>(),
    NullSlot{}, NullSlot{});
  Nexus::Queries::RegisterQueryTypes(Store(m_server->GetSlots().GetRegistry()));
  RegisterOrderExecutionServices(Store(m_server->GetSlots()));
  RegisterOrderExecutionMessages(Store(m_server->GetSlots()));
  TestServiceProtocolClientBuilder builder{
    [=] {
      return std::make_unique<TestServiceProtocolClientBuilder::Channel>("test",
        Ref(*serverConnection));
    }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>()};
  m_client.emplace(builder);
  m_server->Open();
  m_client->Open();
}

void OrderExecutionClientTester::tearDown() {
  m_client.reset();
  m_server.reset();
}

void OrderExecutionClientTester::TestSubmitOrder() {
  Security security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()};
  auto sentSubmitOrderRequest = false;
  OrderFields orderFields;
  TestServiceProtocolServer::ServiceProtocolClient* serverClient;
  QueryOrderSubmissionsService::AddSlot(Store(m_server->GetSlots()),
    [&] (auto& client, auto& query) {
      CPPUNIT_ASSERT(query.GetIndex() == DirectoryEntry::GetRootAccount());
      OrderSubmissionQueryResult result;
      result.m_queryId = -1;
      return result;
    });
  NewOrderSingleService::AddSlot(Store(m_server->GetSlots()),
    [&] (auto& client, auto& requestedOrderFields) {
      CPPUNIT_ASSERT(requestedOrderFields == orderFields);
      sentSubmitOrderRequest = true;
      serverClient = &client;
      SequencedAccountOrderInfo order;
      order.GetSequence() = Beam::Queries::Sequence{5};
      order->GetIndex() = requestedOrderFields.m_account;
      (*order)->m_orderId = 1;
      return order;
    });
  orderFields.m_account = DirectoryEntry::GetRootAccount();
  orderFields.m_security = security;
  orderFields.m_quantity = 100;
  orderFields.m_price = Money::CENT;
  orderFields.m_side = Side::BID;
  orderFields.m_type = OrderType::LIMIT;
  orderFields.m_destination = "TST";
  auto& order = m_client->Submit(orderFields);
  CPPUNIT_ASSERT(sentSubmitOrderRequest);
  auto updates = std::make_shared<Queue<ExecutionReport>>();
  order.GetPublisher().Monitor(updates);
  auto pendingNewReportOut = ExecutionReport::BuildInitialReport(
    order.GetInfo().m_orderId, microsec_clock::universal_time());
  SendRecordMessage<OrderUpdateMessage>(*serverClient, pendingNewReportOut);
  auto pendingNewReportIn = updates->Top();
  updates->Pop();
  CPPUNIT_ASSERT(pendingNewReportIn.m_status == OrderStatus::PENDING_NEW);
  CPPUNIT_ASSERT(pendingNewReportIn.m_id == 1);
  CPPUNIT_ASSERT(pendingNewReportIn.m_additionalTags.empty());
  auto newReportOut = ExecutionReport::BuildUpdatedReport(pendingNewReportOut,
    OrderStatus::NEW, microsec_clock::universal_time());
  SendRecordMessage<OrderUpdateMessage>(*serverClient, newReportOut);
  auto newReportIn = updates->Top();
  CPPUNIT_ASSERT(newReportIn.m_status == OrderStatus::NEW);
  CPPUNIT_ASSERT(newReportIn.m_id == 1);
  CPPUNIT_ASSERT(newReportIn.m_additionalTags.empty());
}
