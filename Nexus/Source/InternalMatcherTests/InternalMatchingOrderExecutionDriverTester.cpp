#include "Nexus/InternalMatcherTests/InternalMatchingOrderExecutionDriverTester.hpp"
#include <Beam/Queues/Queue.hpp>
#include <boost/functional/value_factory.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::InternalMatcher;
using namespace Nexus::InternalMatcher::Tests;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace std;

namespace {
  DirectoryEntry TRADER_A(DirectoryEntry::Type::ACCOUNT, 123, "trader_a");
  Security TST("TST", DefaultMarkets::NASDAQ(), DefaultCountries::US());

  //! No special condition.
  const int NONE = 0;

  //! The order being matched was prematurely interrupted.
  const int INTERRUPTED = 1;

  //! The order being matches is part of a multi-internal match.
  const int MULTIPLE_INTERNAL_MATCHES = 2;
}

InternalMatchingOrderExecutionDriverTester::OrderEntry::OrderEntry()
  : m_order(nullptr),
    m_mockOrder(nullptr),
    m_remainingQuantity(0) {}

void InternalMatchingOrderExecutionDriverTester::setUp() {
  m_timerThreadPool.Initialize();
  m_serviceLocatorInstance.Initialize();
  m_serviceLocatorInstance->Open();
  m_serviceLocatorClient = m_serviceLocatorInstance->BuildClient();
  m_serviceLocatorClient->SetCredentials("root", "");
  m_serviceLocatorClient->Open();
  m_uidServiceInstance.Initialize();
  m_uidServiceInstance->Open();
  DirectoryEntry servicesDirectory =
    m_serviceLocatorInstance->GetRoot().MakeDirectory("services",
    DirectoryEntry::GetStarDirectory());
  std::unique_ptr<MarketDataServiceTestInstance::ServiceLocatorClient>
    marketDataServiceLocatorClient = m_serviceLocatorInstance->BuildClient();
  marketDataServiceLocatorClient->SetCredentials("root", "");
  marketDataServiceLocatorClient->Open();
  m_marketDataServiceInstance.Initialize(
    std::move(marketDataServiceLocatorClient));
  m_marketDataServiceInstance->Open();
  m_mockOrderExecutionDriver.Initialize();
  m_mockOrderExecutionDriver->SetOrderStatusNewOnSubmission(true);
  m_mockDriverMonitor = std::make_shared<Queue<PrimitiveOrder*>>();
  m_mockOrderExecutionDriver->GetPublisher().Monitor(m_mockDriverMonitor);
  unique_ptr<MarketDataClient> marketDataClient  =
    m_marketDataServiceInstance->BuildClient(Ref(*m_serviceLocatorClient));
  m_uidClient = m_uidServiceInstance->BuildClient();
  m_uidClient->Open();
  m_orderExecutionDriver.Initialize(DirectoryEntry::GetRootAccount(),
    Initialize(), std::move(marketDataClient), Initialize(),
    m_uidServiceInstance->BuildClient(), &*m_mockOrderExecutionDriver,
    Ref(*m_timerThreadPool));
  m_orderExecutionDriver->Open();
  m_serviceLocatorInstance->GetRoot().MakeAccount("market_data_feed_service",
    "", servicesDirectory);
  m_marketDataFeedServiceLocatorClient =
    m_serviceLocatorInstance->BuildClient();
  m_marketDataFeedServiceLocatorClient->SetCredentials(
    "market_data_feed_service", "");
  m_marketDataFeedServiceLocatorClient->Open();
}

void InternalMatchingOrderExecutionDriverTester::tearDown() {
  CPPUNIT_ASSERT(m_mockDriverMonitor->IsEmpty());
  m_marketDataFeedServiceLocatorClient.reset();
  m_orderExecutionDriver.Reset();
  m_uidClient.reset();
  m_mockDriverMonitor.reset();
  m_mockOrderExecutionDriver.Reset();
  m_marketDataServiceInstance.Reset();
  m_uidServiceInstance.Reset();
  m_serviceLocatorClient.reset();
  m_serviceLocatorInstance.Reset();
  m_timerThreadPool.Reset();
}

void InternalMatchingOrderExecutionDriverTester::
    TestBidAndAskWithoutMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  OrderEntry bidOrderEntry = Execute(Side::BID, Money::ONE, 100);
  OrderEntry askOrderEntry = Execute(Side::ASK,
    Money::ONE + Money::CENT, 100);
  Cancel(askOrderEntry);
  Cancel(bidOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddBidAndRemoveAskWithMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  OrderEntry bidOrderEntry = Execute(Side::BID, Money::ONE, 100);
  OrderEntry askOrderEntry = Submit(Side::ASK, Money::ONE, 100);
  ExpectPassiveInternalMatch(bidOrderEntry, 100);
  ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddAskAndRemoveBidWithMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  OrderEntry askOrderEntry = Execute(Side::ASK,
    Money::ONE + Money::CENT, 100);
  OrderEntry bidOrderEntry = Submit(Side::BID,
    Money::ONE + Money::CENT, 100);
  ExpectPassiveInternalMatch(askOrderEntry, 100);
  ExpectStatus(bidOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(bidOrderEntry, Money::ONE + Money::CENT, 100);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddMultipleBidsAndRemoveAskWithMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  OrderEntry bidOrderEntryA = Execute(Side::BID, Money::ONE, 100);
  OrderEntry bidOrderEntryB = Execute(Side::BID, Money::ONE, 100);
  OrderEntry askOrderEntry = Submit(Side::ASK, Money::ONE, 200);
  ExpectPassiveInternalMatch(bidOrderEntryA, 100);
  ExpectPassiveInternalMatch(bidOrderEntryB, 100);
  ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100,
    MULTIPLE_INTERNAL_MATCHES);
  ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100);
}

void InternalMatchingOrderExecutionDriverTester::
    TestFarBidAndCrossedAskWithoutMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  OrderEntry bidOrderEntry = Execute(Side::BID,
    50 * Money::CENT, 100);
  OrderEntry askOrderEntry = Execute(Side::ASK, Money::CENT, 100);
  Cancel(askOrderEntry);
  Cancel(bidOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestFarAskAndCrossedBidWithoutMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  OrderEntry askOrderEntry = Execute(Side::ASK, 2 * Money::ONE, 100);
  OrderEntry bidOrderEntry = Execute(Side::BID, 4 * Money::ONE, 100);
  Cancel(bidOrderEntry);
  Cancel(askOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddBidAndPartialRemoveAskWithMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  OrderEntry bidOrderEntry = Execute(Side::BID, Money::ONE, 1000);
  OrderEntry askOrderEntry = Submit(Side::ASK, Money::ONE, 100);
  ExpectPassiveInternalMatch(bidOrderEntry, 100);
  ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100);
  Cancel(bidOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddAskAndPartialRemoveBidWithMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  OrderEntry askOrderEntry = Execute(Side::ASK,
    Money::ONE + Money::CENT, 1000);
  OrderEntry bidOrderEntry = Submit(Side::BID,
    Money::ONE + Money::CENT, 100);
  ExpectPassiveInternalMatch(askOrderEntry, 100);
  ExpectStatus(bidOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(bidOrderEntry, Money::ONE + Money::CENT, 100);
  Cancel(askOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddBidAndRemoveAskWithMatchingLeftOver() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  OrderEntry bidOrderEntry = Execute(Side::BID, Money::ONE, 100);
  OrderEntry askOrderEntry = Submit(Side::ASK, Money::ONE, 1000);
  ExpectPassiveInternalMatch(bidOrderEntry, 100);
  ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100);
  Cancel(askOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddBidAndRemoveAskWithPrematureFill() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  OrderEntry bidOrderEntry = Execute(Side::BID, Money::ONE, 100);
  OrderEntry askOrderEntry = Submit(Side::ASK, Money::ONE, 1000);
  ExpectStatus(bidOrderEntry.m_mockExecutionReportQueue,
    OrderStatus::PENDING_CANCEL);
  Fill(bidOrderEntry, Money::ONE, 100);
  Accept(askOrderEntry);
  ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  Cancel(askOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddBidAndRemoveAskWithPrematurePartialFill() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  OrderEntry bidOrderEntry = Execute(Side::BID, Money::ONE, 1000);
  OrderEntry askOrderEntry = Submit(Side::ASK, Money::ONE, 100);
  ExpectStatus(bidOrderEntry.m_mockExecutionReportQueue,
    OrderStatus::PENDING_CANCEL);
  Fill(bidOrderEntry, Money::ONE, 100);
  ExpectPassiveInternalMatch(bidOrderEntry, 100, INTERRUPTED);
  ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100);
  Cancel(bidOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::SetBbo(Money bid, Money ask) {
  BboQuote bbo(Quote(bid, 100, Side::BID), Quote(ask, 100, Side::ASK),
    second_clock::universal_time());
  m_marketDataServiceInstance->SetBbo(TST, bbo);
}

InternalMatchingOrderExecutionDriverTester::OrderEntry
    InternalMatchingOrderExecutionDriverTester::Submit(Side side, Money price,
    Quantity quantity) {
  OrderFields fields = OrderFields::BuildLimitOrder(TRADER_A, TST,
    DefaultCurrencies::USD(), side, "NASDAQ", quantity, price);
  OrderEntry orderEntry;
  orderEntry.m_fields = fields;
  orderEntry.m_remainingQuantity = fields.m_quantity;
  OrderInfo orderInfo{fields, m_uidClient->LoadNextUid(),
    second_clock::universal_time()};
  orderEntry.m_order = &m_orderExecutionDriver->Submit(orderInfo);
  orderEntry.m_executionReportQueue =
    std::make_shared<Queue<ExecutionReport>>();
  orderEntry.m_order->GetPublisher().Monitor(orderEntry.m_executionReportQueue);
  ExpectStatus(orderEntry.m_executionReportQueue, OrderStatus::PENDING_NEW);
  CPPUNIT_ASSERT(fields == orderEntry.m_order->GetInfo().m_fields);
  return orderEntry;
}

void InternalMatchingOrderExecutionDriverTester::Accept(
    OrderEntry& orderEntry) {
  OrderFields matchedFields = orderEntry.m_order->GetInfo().m_fields;
  matchedFields.m_quantity = orderEntry.m_remainingQuantity;
  PullMockOrder(m_mockDriverMonitor, Store(orderEntry));
  CPPUNIT_ASSERT(orderEntry.m_mockOrder->GetInfo().m_fields == matchedFields);
  ExpectStatus(orderEntry.m_mockExecutionReportQueue, OrderStatus::PENDING_NEW);
  ExpectStatus(orderEntry.m_mockExecutionReportQueue, OrderStatus::NEW);
}

InternalMatchingOrderExecutionDriverTester::OrderEntry
    InternalMatchingOrderExecutionDriverTester::Execute(Side side,
    Money price, Quantity quantity) {
  OrderEntry orderEntry = Submit(side, price, quantity);
  Accept(orderEntry);
  ExpectStatus(orderEntry.m_executionReportQueue, OrderStatus::NEW);
  return orderEntry;
}

void InternalMatchingOrderExecutionDriverTester::Fill(OrderEntry& orderEntry,
    Money price, Quantity quantity) {
  FillOrder(*orderEntry.m_mockOrder, price, quantity,
    second_clock::universal_time());
  if(orderEntry.m_remainingQuantity > quantity) {
    ExpectStatus(orderEntry.m_mockExecutionReportQueue,
      OrderStatus::PARTIALLY_FILLED);
    ExpectStatus(orderEntry.m_executionReportQueue,
      OrderStatus::PARTIALLY_FILLED);
  } else {
    ExpectStatus(orderEntry.m_mockExecutionReportQueue, OrderStatus::FILLED);
    ExpectStatus(orderEntry.m_executionReportQueue, OrderStatus::FILLED);
  }
  orderEntry.m_remainingQuantity -= quantity;
}

void InternalMatchingOrderExecutionDriverTester::Cancel(
    OrderEntry& orderEntry) {
  OrderExecutionSession session;
  session.SetAccount(TRADER_A);
  m_orderExecutionDriver->Cancel(session,
    orderEntry.m_order->GetInfo().m_orderId);
  ExpectStatus(orderEntry.m_executionReportQueue, OrderStatus::PENDING_CANCEL);
  CancelOrder(*orderEntry.m_mockOrder, second_clock::universal_time());
  ExpectStatus(orderEntry.m_executionReportQueue, OrderStatus::CANCELED);
}

void InternalMatchingOrderExecutionDriverTester::ExpectStatus(
    const std::shared_ptr<Queue<ExecutionReport>>& queue, OrderStatus status) {
  ExecutionReport report = queue->Top();
  CPPUNIT_ASSERT(report.m_status == status);
  queue->Pop();
}

void InternalMatchingOrderExecutionDriverTester::ExpectPassiveInternalMatch(
    OrderEntry& orderEntry, Quantity expectedQuantity, int condition) {
  if(condition == NONE) {
    ExpectStatus(orderEntry.m_mockExecutionReportQueue,
      OrderStatus::PENDING_CANCEL);
  }
  CancelOrder(*orderEntry.m_mockOrder, second_clock::universal_time());

  // Check that the passive order was filled.
  ExecutionReport fillReport = orderEntry.m_executionReportQueue->Top();
  orderEntry.m_executionReportQueue->Pop();
  CPPUNIT_ASSERT(fillReport.m_lastPrice ==
    orderEntry.m_order->GetInfo().m_fields.m_price);
  CPPUNIT_ASSERT(fillReport.m_lastQuantity == expectedQuantity);
  orderEntry.m_remainingQuantity -= fillReport.m_lastQuantity;
  if(orderEntry.m_remainingQuantity == 0) {
    CPPUNIT_ASSERT(fillReport.m_status == OrderStatus::FILLED);
  } else {
    CPPUNIT_ASSERT(fillReport.m_status == OrderStatus::PARTIALLY_FILLED);
  }

  // Check for any remaining quantity left over and that an Order is executed
  // for the remaining amount.
  if(orderEntry.m_remainingQuantity > 0) {
    Accept(orderEntry);
    OrderFields matchedFields = orderEntry.m_order->GetInfo().m_fields;
    matchedFields.m_quantity = orderEntry.m_remainingQuantity;
    CPPUNIT_ASSERT(orderEntry.m_mockOrder->GetInfo().m_fields == matchedFields);
  }
}

void InternalMatchingOrderExecutionDriverTester::ExpectActiveInternalMatch(
    OrderEntry& orderEntry, Money expectedPrice, Quantity expectedQuantity,
    int condition) {

  //! Check that the active order was filled.
  ExecutionReport fillReport = orderEntry.m_executionReportQueue->Top();
  orderEntry.m_executionReportQueue->Pop();
  CPPUNIT_ASSERT(fillReport.m_lastPrice == expectedPrice);
  CPPUNIT_ASSERT(fillReport.m_lastQuantity == expectedQuantity);
  orderEntry.m_remainingQuantity -= fillReport.m_lastQuantity;
  if(orderEntry.m_remainingQuantity == 0) {
    CPPUNIT_ASSERT(fillReport.m_status == OrderStatus::FILLED);
  } else {
    CPPUNIT_ASSERT(fillReport.m_status == OrderStatus::PARTIALLY_FILLED);
    if(condition != MULTIPLE_INTERNAL_MATCHES) {
      Accept(orderEntry);
    }
  }
}

void InternalMatchingOrderExecutionDriverTester::PullMockOrder(
    std::shared_ptr<Queue<PrimitiveOrder*>>& monitor,
    Out<OrderEntry> orderEntry) {
  orderEntry->m_mockOrder = monitor->Top();
  monitor->Pop();
  orderEntry->m_mockExecutionReportQueue =
    std::make_shared<Queue<ExecutionReport>>();
  orderEntry->m_mockOrder->GetPublisher().Monitor(
    orderEntry->m_mockExecutionReportQueue);
}
