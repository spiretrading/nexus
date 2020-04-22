#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/functional/factory.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

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
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::Queries;

namespace {
  struct Fixture {
    using TestOrderExecutionClient = OrderExecutionClient<
      TestServiceProtocolClientBuilder>;

    boost::optional<Beam::Services::Tests::TestServiceProtocolServer>
      m_server;
    boost::optional<TestOrderExecutionClient> m_client;

    Fixture() {
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_server.emplace(serverConnection,
        factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
      Nexus::Queries::RegisterQueryTypes(
        Store(m_server->GetSlots().GetRegistry()));
      RegisterOrderExecutionServices(Store(m_server->GetSlots()));
      RegisterOrderExecutionMessages(Store(m_server->GetSlots()));
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
            "test", Ref(*serverConnection));
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_client.emplace(builder);
      m_server->Open();
      m_client->Open();
    }
  };
}

TEST_SUITE("OrderExecutionClient") {
  TEST_CASE_FIXTURE(Fixture, "submit_order") {
    auto security = Security("TST", DefaultMarkets::NYSE(),
      DefaultCountries::US());
    auto sentSubmitOrderRequest = false;
    auto orderFields = OrderFields();
    auto serverClient = (TestServiceProtocolServer::ServiceProtocolClient*)(
      nullptr);
    QueryOrderSubmissionsService::AddSlot(Store(m_server->GetSlots()),
      [&] (auto& client, auto& query) {
        REQUIRE(query.GetIndex() == DirectoryEntry::GetRootAccount());
        auto result = OrderSubmissionQueryResult();
        result.m_queryId = -1;
        return result;
      });
    NewOrderSingleService::AddSlot(Store(m_server->GetSlots()),
      [&] (auto& client, auto& requestedOrderFields) {
        REQUIRE(requestedOrderFields == orderFields);
        sentSubmitOrderRequest = true;
        serverClient = &client;
        auto order = SequencedAccountOrderInfo();
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
    auto& order = m_client->Submit(orderFields);
    REQUIRE(sentSubmitOrderRequest);
    auto updates = std::make_shared<Queue<ExecutionReport>>();
    order.GetPublisher().Monitor(updates);
    auto pendingNewReportOut = ExecutionReport::BuildInitialReport(
      order.GetInfo().m_orderId, microsec_clock::universal_time());
    SendRecordMessage<OrderUpdateMessage>(*serverClient, pendingNewReportOut);
    auto pendingNewReportIn = updates->Top();
    updates->Pop();
    REQUIRE(pendingNewReportIn.m_status == OrderStatus::PENDING_NEW);
    REQUIRE(pendingNewReportIn.m_id == 1);
    REQUIRE(pendingNewReportIn.m_additionalTags.empty());
    auto newReportOut = ExecutionReport::BuildUpdatedReport(pendingNewReportOut,
      OrderStatus::NEW, microsec_clock::universal_time());
    SendRecordMessage<OrderUpdateMessage>(*serverClient, newReportOut);
    auto newReportIn = updates->Top();
    REQUIRE(newReportIn.m_status == OrderStatus::NEW);
    REQUIRE(newReportIn.m_id == 1);
    REQUIRE(newReportIn.m_additionalTags.empty());
  }
}
