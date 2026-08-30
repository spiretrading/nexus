#include <Aspen/Aspen.hpp>
#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/OrderExecutionService/OrderCancellationReactor.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionClient.hpp"

using namespace Aspen;
using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Tests;
using namespace Nexus::Venues;

TEST_SUITE("OrderCancellationReactor") {
  TEST_CASE("empty_series") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestOrderExecutionClient::Operation>>>();
    auto client = TestOrderExecutionClient(operations);
    auto series = Aspen::none<std::shared_ptr<Order>>();
    auto reactor = OrderCancellationReactor(&client, series);
    auto state = reactor.commit(0);
    REQUIRE(Aspen::is_complete(state));
  }

  TEST_CASE("single_order_cancellation") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.push(true);
    });
    auto flag = CommitFlag();
    flag.set_trigger(&trigger);
    auto scope = CommitFlagScope(flag);
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestOrderExecutionClient::Operation>>>();
    auto client = TestOrderExecutionClient(operations);
    auto ticker = parse_ticker("TST.TSX");
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto order = std::make_shared<PrimitiveOrder>(OrderInfo(
      fields, 123, false, time_from_string("2024-07-21 10:00:00.000")));
    auto series = Shared<Aspen::Queue<std::shared_ptr<Order>>>();
    auto reactor = OrderCancellationReactor(&client, series);
    series->push(order);
    flag.clear();
    REQUIRE(reactor.commit(0) == Aspen::State::EVALUATED);
    series->set_complete();
    flag.clear();
    REQUIRE(reactor.commit(1) == Aspen::State::NONE);
    auto operation = operations->pop();
    auto cancellation =
      std::get_if<TestOrderExecutionClient::CancelOperation>(operation.get());
    REQUIRE(cancellation);
    REQUIRE(cancellation->m_id == 123);
    set_order_status(*order, OrderStatus::PENDING_CANCEL);
    cancel(*order);
    while(!flag.is_raised()) {
      commits.pop();
    }
    flag.clear();
    REQUIRE(reactor.commit(2) == Aspen::State::COMPLETE);
  }
}
