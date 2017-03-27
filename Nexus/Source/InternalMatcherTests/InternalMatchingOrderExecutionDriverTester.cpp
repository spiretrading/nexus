#include "Nexus/InternalMatcherTests/InternalMatchingOrderExecutionDriverTester.hpp"
#include <Beam/Queues/Queue.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

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
  DirectoryEntry TRADER_A{DirectoryEntry::Type::ACCOUNT, 123, "trader_a"};
  Security TST{"TST", DefaultMarkets::NASDAQ(), DefaultCountries::US()};

  //! No special condition.
  const auto NONE = 0;

  //! The order being matched was prematurely interrupted.
  const auto INTERRUPTED = 1;

  //! The order being matches is part of a multi-internal match.
  const auto MULTIPLE_INTERNAL_MATCHES = 2;
}

InternalMatchingOrderExecutionDriverTester::OrderEntry::OrderEntry()
  : m_order{nullptr},
    m_mockOrder{nullptr},
    m_remainingQuantity{0} {}

void InternalMatchingOrderExecutionDriverTester::setUp() {
  m_timerThreadPool.emplace();
  m_environment.emplace();
  m_environment->Open();
  m_serviceClients.emplace(Ref(*m_environment));
  m_serviceClients->Open();
  m_uidClient = m_environment->GetUidEnvironment().BuildClient();
  m_uidClient->Open();
  m_mockDriverMonitor = std::make_shared<Queue<const Order*>>();
  m_environment->MonitorOrderSubmissions(m_mockDriverMonitor);
  m_orderExecutionDriver.emplace(DirectoryEntry::GetRootAccount(),
    Initialize(), &m_serviceClients->GetMarketDataClient(),
    &m_serviceClients->GetTimeClient(),
    m_environment->GetUidEnvironment().BuildClient(),
    &m_environment->GetOrderExecutionEnvironment().GetDriver(),
    Ref(*m_timerThreadPool));
  m_orderExecutionDriver->Open();
}

void InternalMatchingOrderExecutionDriverTester::tearDown() {
  CPPUNIT_ASSERT(m_mockDriverMonitor->IsEmpty());
  m_orderExecutionDriver.reset();
  m_mockDriverMonitor.reset();
  m_uidClient.reset();
  m_serviceClients.reset();
  m_environment.reset();
  m_timerThreadPool.reset();
}

void InternalMatchingOrderExecutionDriverTester::
    TestBidAndAskWithoutMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  auto bidOrderEntry = Execute(Side::BID, Money::ONE, 100);
  auto askOrderEntry = Execute(Side::ASK, Money::ONE + Money::CENT, 100);
  Cancel(askOrderEntry);
  Cancel(bidOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddBidAndRemoveAskWithMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  auto bidOrderEntry = Execute(Side::BID, Money::ONE, 100);
  auto askOrderEntry = Submit(Side::ASK, Money::ONE, 100);
  ExpectPassiveInternalMatch(bidOrderEntry, 100);
  ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddAskAndRemoveBidWithMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  auto askOrderEntry = Execute(Side::ASK, Money::ONE + Money::CENT, 100);
  auto bidOrderEntry = Submit(Side::BID, Money::ONE + Money::CENT, 100);
  ExpectPassiveInternalMatch(askOrderEntry, 100);
  ExpectStatus(bidOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(bidOrderEntry, Money::ONE + Money::CENT, 100);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddMultipleBidsAndRemoveAskWithMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  auto bidOrderEntryA = Execute(Side::BID, Money::ONE, 100);
  auto bidOrderEntryB = Execute(Side::BID, Money::ONE, 100);
  auto askOrderEntry = Submit(Side::ASK, Money::ONE, 200);
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
  auto bidOrderEntry = Execute(Side::BID, 50 * Money::CENT, 100);
  auto askOrderEntry = Execute(Side::ASK, Money::CENT, 100);
  Cancel(askOrderEntry);
  Cancel(bidOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestFarAskAndCrossedBidWithoutMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  auto askOrderEntry = Execute(Side::ASK, 2 * Money::ONE, 100);
  auto bidOrderEntry = Execute(Side::BID, 4 * Money::ONE, 100);
  Cancel(bidOrderEntry);
  Cancel(askOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddBidAndPartialRemoveAskWithMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  auto bidOrderEntry = Execute(Side::BID, Money::ONE, 1000);
  auto askOrderEntry = Submit(Side::ASK, Money::ONE, 100);
  ExpectPassiveInternalMatch(bidOrderEntry, 100);
  ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100);
  Cancel(bidOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddAskAndPartialRemoveBidWithMatching() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  auto askOrderEntry = Execute(Side::ASK, Money::ONE + Money::CENT, 1000);
  auto bidOrderEntry = Submit(Side::BID, Money::ONE + Money::CENT, 100);
  ExpectPassiveInternalMatch(askOrderEntry, 100);
  ExpectStatus(bidOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(bidOrderEntry, Money::ONE + Money::CENT, 100);
  Cancel(askOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddBidAndRemoveAskWithMatchingLeftOver() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  auto bidOrderEntry = Execute(Side::BID, Money::ONE, 100);
  auto askOrderEntry = Submit(Side::ASK, Money::ONE, 1000);
  ExpectPassiveInternalMatch(bidOrderEntry, 100);
  ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100);
  Cancel(askOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::
    TestAddBidAndRemoveAskWithPrematureFill() {
  SetBbo(Money::ONE, Money::ONE + Money::CENT);
  auto bidOrderEntry = Execute(Side::BID, Money::ONE, 100);
  auto askOrderEntry = Submit(Side::ASK, Money::ONE, 1000);
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
  auto bidOrderEntry = Execute(Side::BID, Money::ONE, 1000);
  auto askOrderEntry = Submit(Side::ASK, Money::ONE, 100);
  ExpectStatus(bidOrderEntry.m_mockExecutionReportQueue,
    OrderStatus::PENDING_CANCEL);
  Fill(bidOrderEntry, Money::ONE, 100);
  ExpectPassiveInternalMatch(bidOrderEntry, 100, INTERRUPTED);
  ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
  ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100);
  Cancel(bidOrderEntry);
}

void InternalMatchingOrderExecutionDriverTester::SetBbo(Money bid, Money ask) {
  BboQuote bbo{Quote{bid, 100, Side::BID}, Quote{ask, 100, Side::ASK},
    not_a_date_time};
  m_environment->Update(TST, bbo);
}

InternalMatchingOrderExecutionDriverTester::OrderEntry
    InternalMatchingOrderExecutionDriverTester::Submit(Side side, Money price,
    Quantity quantity) {
  auto fields = OrderFields::BuildLimitOrder(TRADER_A, TST,
    DefaultCurrencies::USD(), side, "NASDAQ", quantity, price);
  OrderEntry orderEntry;
  orderEntry.m_fields = fields;
  orderEntry.m_remainingQuantity = fields.m_quantity;
  OrderInfo orderInfo{fields, m_uidClient->LoadNextUid(),
    second_clock::universal_time()};
  orderEntry.m_order = &m_orderExecutionDriver->Submit(orderInfo);
  orderEntry.m_executionReportQueue =
    std::make_shared<Queue<ExecutionReport>>();
  orderEntry.m_order->GetPublisher().Monitor(
    orderEntry.m_executionReportQueue);
  ExpectStatus(orderEntry.m_executionReportQueue, OrderStatus::PENDING_NEW);
  CPPUNIT_ASSERT(fields == orderEntry.m_order->GetInfo().m_fields);
  return orderEntry;
}

void InternalMatchingOrderExecutionDriverTester::Accept(
    OrderEntry& orderEntry) {
  auto matchedFields = orderEntry.m_order->GetInfo().m_fields;
  matchedFields.m_quantity = orderEntry.m_remainingQuantity;
  PullMockOrder(m_mockDriverMonitor, Store(orderEntry));
  CPPUNIT_ASSERT(orderEntry.m_mockOrder->GetInfo().m_fields == matchedFields);
  ExpectStatus(orderEntry.m_mockExecutionReportQueue,
    OrderStatus::PENDING_NEW);
  m_environment->AcceptOrder(*orderEntry.m_mockOrder);
  ExpectStatus(orderEntry.m_mockExecutionReportQueue, OrderStatus::NEW);
}

InternalMatchingOrderExecutionDriverTester::OrderEntry
    InternalMatchingOrderExecutionDriverTester::Execute(Side side,
    Money price, Quantity quantity) {
  auto orderEntry = Submit(side, price, quantity);
  Accept(orderEntry);
  ExpectStatus(orderEntry.m_executionReportQueue, OrderStatus::NEW);
  return orderEntry;
}

void InternalMatchingOrderExecutionDriverTester::Fill(OrderEntry& orderEntry,
    Money price, Quantity quantity) {
  m_environment->FillOrder(*orderEntry.m_mockOrder, price, quantity);
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
  m_environment->CancelOrder(*orderEntry.m_mockOrder);
  ExpectStatus(orderEntry.m_executionReportQueue, OrderStatus::CANCELED);
}

void InternalMatchingOrderExecutionDriverTester::ExpectStatus(
    const std::shared_ptr<Queue<ExecutionReport>>& queue, OrderStatus status) {
  auto report = queue->Top();
  CPPUNIT_ASSERT(report.m_status == status);
  queue->Pop();
}

void InternalMatchingOrderExecutionDriverTester::ExpectPassiveInternalMatch(
    OrderEntry& orderEntry, Quantity expectedQuantity, int condition) {
  if(condition == NONE) {
    ExpectStatus(orderEntry.m_mockExecutionReportQueue,
      OrderStatus::PENDING_CANCEL);
  }
  m_environment->CancelOrder(*orderEntry.m_mockOrder);

  // Check that the passive order was filled.
  auto fillReport = orderEntry.m_executionReportQueue->Top();
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
    auto matchedFields = orderEntry.m_order->GetInfo().m_fields;
    matchedFields.m_quantity = orderEntry.m_remainingQuantity;
    CPPUNIT_ASSERT(orderEntry.m_mockOrder->GetInfo().m_fields ==
      matchedFields);
  }
}

void InternalMatchingOrderExecutionDriverTester::ExpectActiveInternalMatch(
    OrderEntry& orderEntry, Money expectedPrice, Quantity expectedQuantity,
    int condition) {

  //! Check that the active order was filled.
  auto fillReport = orderEntry.m_executionReportQueue->Top();
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
    std::shared_ptr<Queue<const Order*>>& monitor,
    Out<OrderEntry> orderEntry) {
  orderEntry->m_mockOrder = monitor->Top();
  monitor->Pop();
  orderEntry->m_mockExecutionReportQueue =
    std::make_shared<Queue<ExecutionReport>>();
  orderEntry->m_mockOrder->GetPublisher().Monitor(
    orderEntry->m_mockExecutionReportQueue);
}
