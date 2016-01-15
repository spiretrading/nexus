#include "Nexus/OrderExecutionServiceTests/OrderExecutionClientTester.hpp"
#include <Beam/ServiceLocator/NullAuthenticator.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::Queries;
using namespace std;

void OrderExecutionClientTester::setUp() {
  m_serverConnection.Initialize();
  m_server.Initialize(&*m_serverConnection,
    factory<std::shared_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
  Nexus::Queries::RegisterQueryTypes(Store(m_server->GetSlots().GetRegistry()));
  RegisterOrderExecutionServices(Store(m_server->GetSlots()));
  RegisterOrderExecutionMessages(Store(m_server->GetSlots()));
  ServiceProtocolClientBuilder builder(
    [&] {
      return std::make_unique<ServiceProtocolClientBuilder::Channel>(("test"),
        Ref(*m_serverConnection));
    },
    [&] {
      return std::make_unique<ServiceProtocolClientBuilder::Timer>();
    });
  m_client.Initialize(builder);
  m_server->Open();
  m_client->Open();
}

void OrderExecutionClientTester::tearDown() {
  m_client.Reset();
  m_server.Reset();
  m_serverConnection.Reset();
}

void OrderExecutionClientTester::TestSubmitOrder() {
  Security security("TST", DefaultMarkets::NYSE(), DefaultCountries::US());
  bool sentSubmitOrderRequest = false;
  OrderFields orderFields;
  ServiceProtocolServer::ServiceProtocolClient* serverClient;
  NewOrderSingleService::AddSlot(Store(m_server->GetSlots()),
    [&] (ServiceProtocolServer::ServiceProtocolClient& client,
        const OrderFields& requestedOrderFields) {
      CPPUNIT_ASSERT(requestedOrderFields == orderFields);
      sentSubmitOrderRequest = true;
      serverClient = &client;
      SequencedAccountOrderInfo order;
      order.GetSequence() = Beam::Queries::Sequence(5);
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
  const Order& order = m_client->Submit(orderFields);
  CPPUNIT_ASSERT(sentSubmitOrderRequest);
  std::shared_ptr<Queue<ExecutionReport>> updates =
    std::make_shared<Queue<ExecutionReport>>();
  order.GetPublisher().Monitor(updates);
  ExecutionReport pendingNewReportOut = ExecutionReport::BuildInitialReport(
    order.GetInfo().m_orderId, microsec_clock::universal_time());
  SendRecordMessage<OrderUpdateMessage>(*serverClient, pendingNewReportOut);
  ExecutionReport pendingNewReportIn = updates->Top();
  updates->Pop();
  CPPUNIT_ASSERT(pendingNewReportIn.m_status == OrderStatus::PENDING_NEW);
  CPPUNIT_ASSERT(pendingNewReportIn.m_id == 1);
  CPPUNIT_ASSERT(pendingNewReportIn.m_additionalTags.empty());
  ExecutionReport newReportOut = ExecutionReport::BuildUpdatedReport(
    pendingNewReportOut, OrderStatus::NEW, microsec_clock::universal_time());
  SendRecordMessage<OrderUpdateMessage>(*serverClient, newReportOut);
  ExecutionReport newReportIn = updates->Top();
  CPPUNIT_ASSERT(newReportIn.m_status == OrderStatus::NEW);
  CPPUNIT_ASSERT(newReportIn.m_id == 1);
  CPPUNIT_ASSERT(newReportIn.m_additionalTags.empty());
}
