#include <Beam/Queues/Queue.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/InternalMatcher/InternalMatchingOrderExecutionDriver.hpp"
#include "Nexus/InternalMatcher/NullMatchReportBuilder.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionDriver.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::UidService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::InternalMatcher;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  const auto TRADER_A = DirectoryEntry::MakeAccount(123, "trader_a");
  const auto TST = Security("TST", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());

  /** No special condition. */
  const auto NONE = 0;

  /** The order being matched was prematurely interrupted. */
  const auto INTERRUPTED = 1;

  /** The order being matches is part of a multi-internal match. */
  const auto MULTIPLE_INTERNAL_MATCHES = 2;

  struct OrderEntry {
    OrderFields m_fields;
    std::shared_ptr<Queue<ExecutionReport>> m_executionReportQueue;
    const Order* m_order;
    std::shared_ptr<Queue<ExecutionReport>> m_mockExecutionReportQueue;
    const Order* m_mockOrder;
    Quantity m_remainingQuantity;

    OrderEntry()
      : m_order(nullptr),
        m_mockOrder(nullptr),
        m_remainingQuantity(0) {}
  };

  struct Fixture {
    using TestInternalMatchingOrderExecutionDriver =
      InternalMatchingOrderExecutionDriver<NullMatchReportBuilder,
        MarketDataClientBox, TimeClientBox, UidClientBox,
        VirtualOrderExecutionDriver*>;
    TestEnvironment m_environment;
    TestServiceClients m_serviceClients;
    UidClientBox m_uidClient;
    std::shared_ptr<Queue<const Order*>> m_mockDriverMonitor;
    TestInternalMatchingOrderExecutionDriver m_orderExecutionDriver;

    Fixture()
        : m_serviceClients(Ref(m_environment)),
          m_uidClient(m_environment.GetUidEnvironment().MakeClient()),
          m_mockDriverMonitor(std::make_shared<Queue<const Order*>>()),
          m_orderExecutionDriver(DirectoryEntry::GetRootAccount(),
            Initialize(), m_serviceClients.GetMarketDataClient(),
            &m_serviceClients.GetTimeClient(),
            m_environment.GetUidEnvironment().MakeClient(),
            &m_environment.GetOrderExecutionEnvironment().GetDriver()) {
      m_environment.MonitorOrderSubmissions(m_mockDriverMonitor);
    }

    ~Fixture() {
      REQUIRE(!m_mockDriverMonitor->TryPop());
    }

    void SetBbo(Money bid, Money ask) {
      auto bbo = BboQuote(Quote(bid, 100, Side::BID),
        Quote(ask, 100, Side::ASK), not_a_date_time);
      m_environment.Publish(TST, bbo);
    }

    auto Submit(Side side, Money price, Quantity quantity) {
      auto fields = OrderFields::BuildLimitOrder(TRADER_A, TST,
        DefaultCurrencies::USD(), side, "NASDAQ", quantity, price);
      auto orderEntry = OrderEntry();
      orderEntry.m_fields = fields;
      orderEntry.m_remainingQuantity = fields.m_quantity;
      auto orderInfo = OrderInfo(fields, m_uidClient.LoadNextUid(),
        second_clock::universal_time());
      orderEntry.m_order = &m_orderExecutionDriver.Submit(orderInfo);
      orderEntry.m_executionReportQueue =
        std::make_shared<Queue<ExecutionReport>>();
      orderEntry.m_order->GetPublisher().Monitor(
        orderEntry.m_executionReportQueue);
      ExpectStatus(orderEntry.m_executionReportQueue, OrderStatus::PENDING_NEW);
      REQUIRE(fields == orderEntry.m_order->GetInfo().m_fields);
      return orderEntry;
    }

    void Accept(OrderEntry& orderEntry) {
      auto matchedFields = orderEntry.m_order->GetInfo().m_fields;
      matchedFields.m_quantity = orderEntry.m_remainingQuantity;
      PullMockOrder(m_mockDriverMonitor, Store(orderEntry));
      REQUIRE(orderEntry.m_mockOrder->GetInfo().m_fields == matchedFields);
      ExpectStatus(orderEntry.m_mockExecutionReportQueue,
        OrderStatus::PENDING_NEW);
      m_environment.Accept(*orderEntry.m_mockOrder);
      ExpectStatus(orderEntry.m_mockExecutionReportQueue, OrderStatus::NEW);
    }

    auto Execute(Side side, Money price, Quantity quantity) {
      auto orderEntry = Submit(side, price, quantity);
      Accept(orderEntry);
      ExpectStatus(orderEntry.m_executionReportQueue, OrderStatus::NEW);
      return orderEntry;
    }

    void Fill(OrderEntry& orderEntry, Money price, Quantity quantity) {
      m_environment.Fill(*orderEntry.m_mockOrder, price, quantity);
      if(orderEntry.m_remainingQuantity > quantity) {
        ExpectStatus(orderEntry.m_mockExecutionReportQueue,
          OrderStatus::PARTIALLY_FILLED);
        ExpectStatus(orderEntry.m_executionReportQueue,
          OrderStatus::PARTIALLY_FILLED);
      } else {
        ExpectStatus(orderEntry.m_mockExecutionReportQueue,
          OrderStatus::FILLED);
        ExpectStatus(orderEntry.m_executionReportQueue, OrderStatus::FILLED);
      }
      orderEntry.m_remainingQuantity -= quantity;
    }

    void Cancel(OrderEntry& orderEntry) {
      auto session = OrderExecutionSession();
      session.SetAccount(TRADER_A);
      m_orderExecutionDriver.Cancel(session,
        orderEntry.m_order->GetInfo().m_orderId);
      ExpectStatus(orderEntry.m_executionReportQueue,
        OrderStatus::PENDING_CANCEL);
      m_environment.Cancel(*orderEntry.m_mockOrder);
      ExpectStatus(orderEntry.m_executionReportQueue, OrderStatus::CANCELED);
    }

    void ExpectStatus(const std::shared_ptr<Queue<ExecutionReport>>& queue,
        OrderStatus status) {
      auto report = queue->Pop();
      REQUIRE(report.m_status == status);
    }

    void ExpectPassiveInternalMatch(OrderEntry& orderEntry,
        Quantity expectedQuantity, int condition = 0) {
      if(condition == NONE) {
        ExpectStatus(orderEntry.m_mockExecutionReportQueue,
          OrderStatus::PENDING_CANCEL);
      }
      m_environment.Cancel(*orderEntry.m_mockOrder);

      // Check that the passive order was filled.
      auto fillReport = orderEntry.m_executionReportQueue->Pop();
      REQUIRE(fillReport.m_lastPrice ==
        orderEntry.m_order->GetInfo().m_fields.m_price);
      REQUIRE(fillReport.m_lastQuantity == expectedQuantity);
      orderEntry.m_remainingQuantity -= fillReport.m_lastQuantity;
      if(orderEntry.m_remainingQuantity == 0) {
        REQUIRE(fillReport.m_status == OrderStatus::FILLED);
      } else {
        REQUIRE(fillReport.m_status == OrderStatus::PARTIALLY_FILLED);
      }

      // Check for any remaining quantity left over and that an Order is
      // executed for the remaining amount.
      if(orderEntry.m_remainingQuantity > 0) {
        Accept(orderEntry);
        auto matchedFields = orderEntry.m_order->GetInfo().m_fields;
        matchedFields.m_quantity = orderEntry.m_remainingQuantity;
        REQUIRE(orderEntry.m_mockOrder->GetInfo().m_fields == matchedFields);
      }
    }

    void ExpectActiveInternalMatch(OrderEntry& orderEntry, Money expectedPrice,
        Quantity expectedQuantity, int condition = 0) {

      //! Check that the active order was filled.
      auto fillReport = orderEntry.m_executionReportQueue->Pop();
      REQUIRE(fillReport.m_lastPrice == expectedPrice);
      REQUIRE(fillReport.m_lastQuantity == expectedQuantity);
      orderEntry.m_remainingQuantity -= fillReport.m_lastQuantity;
      if(orderEntry.m_remainingQuantity == 0) {
        REQUIRE(fillReport.m_status == OrderStatus::FILLED);
      } else {
        REQUIRE(fillReport.m_status == OrderStatus::PARTIALLY_FILLED);
        if(condition != MULTIPLE_INTERNAL_MATCHES) {
          Accept(orderEntry);
        }
      }
    }

    void PullMockOrder(std::shared_ptr<Queue<const Order*>>& monitor,
        Out<OrderEntry> orderEntry) {
      orderEntry->m_mockOrder = monitor->Pop();
      orderEntry->m_mockExecutionReportQueue =
        std::make_shared<Queue<ExecutionReport>>();
      orderEntry->m_mockOrder->GetPublisher().Monitor(
        orderEntry->m_mockExecutionReportQueue);
    }
  };
}

TEST_SUITE("InternalMatchingOrderExecutionDriver") {
  TEST_CASE_FIXTURE(Fixture, "bid_and_ask_without_matching") {
    SetBbo(Money::ONE, Money::ONE + Money::CENT);
    auto bidOrderEntry = Execute(Side::BID, Money::ONE, 100);
    auto askOrderEntry = Execute(Side::ASK, Money::ONE + Money::CENT, 100);
    Cancel(askOrderEntry);
    Cancel(bidOrderEntry);
  }

  TEST_CASE_FIXTURE(Fixture, "add_bid_and_remove_ask_with_matching") {
    SetBbo(Money::ONE, Money::ONE + Money::CENT);
    auto bidOrderEntry = Execute(Side::BID, Money::ONE, 100);
    auto askOrderEntry = Submit(Side::ASK, Money::ONE, 100);
    ExpectPassiveInternalMatch(bidOrderEntry, 100);
    ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
    ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100);
  }

  TEST_CASE_FIXTURE(Fixture, "add_ask_and_remove_bid_with_matching") {
    SetBbo(Money::ONE, Money::ONE + Money::CENT);
    auto askOrderEntry = Execute(Side::ASK, Money::ONE + Money::CENT, 100);
    auto bidOrderEntry = Submit(Side::BID, Money::ONE + Money::CENT, 100);
    ExpectPassiveInternalMatch(askOrderEntry, 100);
    ExpectStatus(bidOrderEntry.m_executionReportQueue, OrderStatus::NEW);
    ExpectActiveInternalMatch(bidOrderEntry, Money::ONE + Money::CENT, 100);
  }

  TEST_CASE_FIXTURE(Fixture, "add_multiple_bids_and_remove_ask_with_matching") {
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

  TEST_CASE_FIXTURE(Fixture, "far_bid_and_crossed_ask_without_matching") {
    SetBbo(Money::ONE, Money::ONE + Money::CENT);
    auto bidOrderEntry = Execute(Side::BID, 50 * Money::CENT, 100);
    auto askOrderEntry = Execute(Side::ASK, Money::CENT, 100);
    Cancel(askOrderEntry);
    Cancel(bidOrderEntry);
  }

  TEST_CASE_FIXTURE(Fixture, "far_ask_and_crossed_bid_without_matching") {
    SetBbo(Money::ONE, Money::ONE + Money::CENT);
    auto askOrderEntry = Execute(Side::ASK, 2 * Money::ONE, 100);
    auto bidOrderEntry = Execute(Side::BID, 4 * Money::ONE, 100);
    Cancel(bidOrderEntry);
    Cancel(askOrderEntry);
  }

  TEST_CASE_FIXTURE(Fixture, "add_bid_and_partial_remove_ask_with_matching") {
    SetBbo(Money::ONE, Money::ONE + Money::CENT);
    auto bidOrderEntry = Execute(Side::BID, Money::ONE, 1000);
    auto askOrderEntry = Submit(Side::ASK, Money::ONE, 100);
    ExpectPassiveInternalMatch(bidOrderEntry, 100);
    ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
    ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100);
    Cancel(bidOrderEntry);
  }

  TEST_CASE_FIXTURE(Fixture, "add_ask_and_partial_remove_bid_with_matching") {
    SetBbo(Money::ONE, Money::ONE + Money::CENT);
    auto askOrderEntry = Execute(Side::ASK, Money::ONE + Money::CENT, 1000);
    auto bidOrderEntry = Submit(Side::BID, Money::ONE + Money::CENT, 100);
    ExpectPassiveInternalMatch(askOrderEntry, 100);
    ExpectStatus(bidOrderEntry.m_executionReportQueue, OrderStatus::NEW);
    ExpectActiveInternalMatch(bidOrderEntry, Money::ONE + Money::CENT, 100);
    Cancel(askOrderEntry);
  }

  TEST_CASE_FIXTURE(Fixture, "add_bid_and_remove_ask_with_matching_left_over") {
    SetBbo(Money::ONE, Money::ONE + Money::CENT);
    auto bidOrderEntry = Execute(Side::BID, Money::ONE, 100);
    auto askOrderEntry = Submit(Side::ASK, Money::ONE, 1000);
    ExpectPassiveInternalMatch(bidOrderEntry, 100);
    ExpectStatus(askOrderEntry.m_executionReportQueue, OrderStatus::NEW);
    ExpectActiveInternalMatch(askOrderEntry, Money::ONE, 100);
    Cancel(askOrderEntry);
  }

  TEST_CASE_FIXTURE(Fixture, "add_bid_and_remove_ask_with_premature_fill") {
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

  TEST_CASE_FIXTURE(Fixture,
      "add_bid_and_remove_ask_with_premature_partial_fill") {
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
}
