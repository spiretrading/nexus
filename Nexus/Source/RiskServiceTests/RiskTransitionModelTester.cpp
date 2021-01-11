#include <doctest/doctest.h>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/RiskService/RiskTransitionModel.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;

namespace {
  auto ACCOUNT = DirectoryEntry::MakeAccount(153, "simba");
  auto TSLA = Security("TSLA", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  auto XIU = Security("XIU", DefaultMarkets::TSX(), DefaultCountries::CA());

  struct Fixture {
    using TestOrderExecutionClient = OrderExecutionClient<
      TestServiceProtocolClientBuilder>;
    boost::optional<TestServiceProtocolServer> m_protocolServer;
    boost::optional<TestOrderExecutionClient> m_serviceClient;

    Fixture() {
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_protocolServer.emplace(serverConnection,
        factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
      RegisterQueryTypes(Store(m_protocolServer->GetSlots().GetRegistry()));
      RegisterOrderExecutionServices(Store(m_protocolServer->GetSlots()));
      RegisterOrderExecutionMessages(Store(m_protocolServer->GetSlots()));
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
            "test", *serverConnection);
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_serviceClient.emplace(builder);
      QueryOrderSubmissionsService::AddSlot(Store(m_protocolServer->GetSlots()),
        [&] (auto& client, auto& query) {
          return OrderSubmissionQueryResult();
        });
    }
  };
}

TEST_SUITE("RiskTransitionModel") {
  TEST_CASE_FIXTURE(Fixture, "cancel_opening_orders") {
    auto cancelQueue = std::make_shared<Queue<OrderId>>();
    AddMessageSlot<CancelOrderMessage>(Store(m_protocolServer->GetSlots()),
      [&] (auto& client, auto orderId) {
        cancelQueue->Push(orderId);
      });
    auto model = RiskTransitionModel(ACCOUNT, {}, RiskState::Type::ACTIVE,
      &*m_serviceClient, GetDefaultDestinationDatabase());
    auto bidOrder = std::make_shared<PrimitiveOrder>(
      OrderInfo(OrderFields::MakeLimitOrder(TSLA, DefaultCurrencies::USD(),
      Side::BID, 100, Money::ONE), 112, time_from_string(
      "2020-11-17 12:22:06")));
    model.Add(*bidOrder);
    auto bidReport = ExecutionReport();
    bidOrder->With(
      [&] (auto& state, auto& reports) {
        bidReport = reports.front();
      });
    model.Update(bidReport);
    bidReport = ExecutionReport::MakeUpdatedReport(bidReport, OrderStatus::NEW,
      bidReport.m_timestamp);
    model.Update(bidReport);
    auto askOrder = std::make_shared<PrimitiveOrder>(
      OrderInfo(OrderFields::MakeLimitOrder(TSLA, DefaultCurrencies::USD(),
      Side::ASK, 100, Money::ONE + Money::CENT), 113, time_from_string(
      "2020-11-17 12:22:06")));
    model.Add(*askOrder);
    auto askReport = ExecutionReport();
    askOrder->With(
      [&] (auto& state, auto& reports) {
        askReport = reports.front();
      });
    model.Update(askReport);
    askReport = ExecutionReport::MakeUpdatedReport(askReport, OrderStatus::NEW,
      askReport.m_timestamp);
    model.Update(askReport);
    model.Update(RiskState::Type::CLOSE_ORDERS);
    auto cancelIds = std::vector<OrderId>();
    cancelIds.push_back(cancelQueue->Pop());
    cancelIds.push_back(cancelQueue->Pop());
    auto expectedCancelIds = std::vector<OrderId>{112, 113};
    REQUIRE(std::is_permutation(cancelIds.begin(), cancelIds.end(),
      expectedCancelIds.begin(), expectedCancelIds.end()));
  }

  TEST_CASE_FIXTURE(Fixture, "flatten_disabled") {
    auto cancelQueue = std::make_shared<Queue<OrderId>>();
    AddMessageSlot<CancelOrderMessage>(Store(m_protocolServer->GetSlots()),
      [&] (auto& client, auto orderId) {
        cancelQueue->Push(orderId);
      });
    auto submissionQueue = std::make_shared<Queue<OrderFields>>();
    NewOrderSingleService::AddSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& client, const auto& fields) {
        submissionQueue->Push(fields);
        return SequencedValue(IndexedValue(
          OrderInfo(fields, 100, time_from_string("2020-11-17 12:22:06")),
          ACCOUNT), Beam::Queries::Sequence(100));
      });
    auto model = RiskTransitionModel(ACCOUNT, {}, RiskState::Type::ACTIVE,
      &*m_serviceClient, GetDefaultDestinationDatabase());
    auto bidOrder = std::make_shared<PrimitiveOrder>(
      OrderInfo(OrderFields::MakeLimitOrder(TSLA, DefaultCurrencies::USD(),
      Side::BID, 100, Money::ONE), 112, time_from_string(
      "2020-11-17 12:22:06")));
    model.Add(*bidOrder);
    auto bidReport = ExecutionReport();
    bidOrder->With(
      [&] (auto& state, auto& reports) {
        bidReport = reports.front();
      });
    model.Update(bidReport);
    bidReport = ExecutionReport::MakeUpdatedReport(bidReport, OrderStatus::NEW,
      bidReport.m_timestamp);
    model.Update(bidReport);
    bidReport = ExecutionReport::MakeUpdatedReport(bidReport,
      OrderStatus::FILLED, bidReport.m_timestamp);
    bidReport.m_lastPrice = Money::ONE;
    bidReport.m_lastQuantity = 100;
    model.Update(bidReport);
    auto bidOrder2 = std::make_shared<PrimitiveOrder>(
      OrderInfo(OrderFields::MakeLimitOrder(TSLA, DefaultCurrencies::USD(),
      Side::BID, 100, Money::ONE), 127, time_from_string(
      "2020-11-17 12:22:06")));
    model.Add(*bidOrder2);
    auto bidReport2 = ExecutionReport();
    bidOrder2->With(
      [&] (auto& state, auto& reports) {
        bidReport2 = reports.front();
      });
    model.Update(bidReport2);
    bidReport2 = ExecutionReport::MakeUpdatedReport(bidReport2,
      OrderStatus::NEW, bidReport2.m_timestamp);
    model.Update(bidReport2);
    auto askOrder = std::make_shared<PrimitiveOrder>(
      OrderInfo(OrderFields::MakeLimitOrder(TSLA, DefaultCurrencies::USD(),
      Side::ASK, 100, Money::ONE + Money::CENT), 143, time_from_string(
      "2020-11-17 12:22:06")));
    model.Add(*askOrder);
    auto askReport = ExecutionReport();
    askOrder->With(
      [&] (auto& state, auto& reports) {
        askReport = reports.front();
      });
    model.Update(askReport);
    askReport = ExecutionReport::MakeUpdatedReport(askReport, OrderStatus::NEW,
      askReport.m_timestamp);
    model.Update(askReport);
    model.Update(RiskState::Type::CLOSE_ORDERS);
    auto cancelId = cancelQueue->Pop();
    REQUIRE(cancelId == 127);
    bidReport2 = ExecutionReport::MakeUpdatedReport(bidReport2,
      OrderStatus::CANCELED, bidReport2.m_timestamp);
    bidReport2.m_id = 127;
    model.Update(bidReport2);
    model.Update(RiskState::Type::DISABLED);
    cancelId = cancelQueue->Pop();
    REQUIRE(cancelId == 143);
    askReport = ExecutionReport::MakeUpdatedReport(askReport,
      OrderStatus::CANCELED, askReport.m_timestamp);
    model.Update(askReport);
    auto flatteningOrder = submissionQueue->Pop();
    REQUIRE(flatteningOrder.m_security == TSLA);
    REQUIRE(flatteningOrder.m_side == Side::ASK);
    REQUIRE(flatteningOrder.m_quantity == 100);
    REQUIRE(flatteningOrder.m_type == OrderType::MARKET);
    REQUIRE(flatteningOrder.m_destination == "NASDAQ");
  }

  TEST_CASE_FIXTURE(Fixture, "initial_inventory") {
    auto cancelQueue = std::make_shared<Queue<OrderId>>();
    AddMessageSlot<CancelOrderMessage>(Store(m_protocolServer->GetSlots()),
      [&] (auto& client, auto orderId) {
        cancelQueue->Push(orderId);
      });
    auto submissionQueue = std::make_shared<Queue<OrderFields>>();
    NewOrderSingleService::AddSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& client, const auto& fields) {
        submissionQueue->Push(fields);
        return SequencedValue(IndexedValue(
          OrderInfo(fields, 100, time_from_string("2020-11-17 12:22:06")),
          ACCOUNT), Beam::Queries::Sequence(100));
      });
    auto inventory = std::vector<RiskInventory>();
    inventory.push_back(Inventory(Position<Security>(
      Position<Security>::Key(XIU, DefaultCurrencies::CAD()), -300,
      300 * Money::ONE), Money::ONE, Money::ZERO, 300, 1));
    auto model = RiskTransitionModel(ACCOUNT, inventory,
      RiskState::Type::ACTIVE, &*m_serviceClient,
      GetDefaultDestinationDatabase());
    auto bidOrder = std::make_shared<PrimitiveOrder>(
      OrderInfo(OrderFields::MakeLimitOrder(XIU, DefaultCurrencies::CAD(),
      Side::BID, 300, Money::ONE), 113,
      time_from_string("2020-11-17 12:22:06")));
    model.Add(*bidOrder);
    auto bidReport = ExecutionReport();
    bidOrder->With(
      [&] (auto& state, auto& reports) {
        bidReport = reports.front();
      });
    model.Update(bidReport);
    bidReport = ExecutionReport::MakeUpdatedReport(bidReport, OrderStatus::NEW,
      bidReport.m_timestamp);
    model.Update(bidReport);
    auto bidOrder2 = std::make_shared<PrimitiveOrder>(
      OrderInfo(OrderFields::MakeLimitOrder(XIU, DefaultCurrencies::CAD(),
      Side::BID, 100, 2 * Money::ONE), 114,
      time_from_string("2020-11-17 12:22:06")));
    model.Add(*bidOrder2);
    auto bidReport2 = ExecutionReport();
    bidOrder2->With(
      [&] (auto& state, auto& reports) {
        bidReport2 = reports.front();
      });
    model.Update(bidReport2);
    bidReport2 = ExecutionReport::MakeUpdatedReport(bidReport2,
      OrderStatus::NEW, bidReport2.m_timestamp);
    model.Update(bidReport2);
    model.Update(RiskState::Type::CLOSE_ORDERS);
    auto cancelId = cancelQueue->Pop();
    REQUIRE(cancelId == 114);
    bidReport2 = ExecutionReport::MakeUpdatedReport(bidReport2,
      OrderStatus::CANCELED, bidReport2.m_timestamp);
    model.Update(bidReport2);
    auto syncOrder = std::make_shared<PrimitiveOrder>(
      OrderInfo(OrderFields::MakeLimitOrder(XIU, DefaultCurrencies::CAD(),
      Side::ASK, 100, Money::ONE + Money::CENT), 1000,
      time_from_string("2020-11-17 12:22:06")));
    m_serviceClient->Cancel(*syncOrder);
    cancelId = cancelQueue->Pop();
    REQUIRE(cancelId == 1000);
    model.Update(RiskState::Type::DISABLED);
    cancelId = cancelQueue->Pop();
    REQUIRE(cancelId == 113);
    bidReport = ExecutionReport::MakeUpdatedReport(bidReport,
      OrderStatus::CANCELED, bidReport.m_timestamp);
    model.Update(bidReport);
    auto flatteningOrder = submissionQueue->Pop();
    REQUIRE(flatteningOrder.m_security == XIU);
    REQUIRE(flatteningOrder.m_side == Side::BID);
    REQUIRE(flatteningOrder.m_quantity == 300);
    REQUIRE(flatteningOrder.m_type == OrderType::MARKET);
    REQUIRE(flatteningOrder.m_destination == "TSX");
  }
}
