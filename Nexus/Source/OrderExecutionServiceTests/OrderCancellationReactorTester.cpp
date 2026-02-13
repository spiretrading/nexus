#include <Aspen/Aspen.hpp>
#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/OrderCancellationReactor.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionClient.hpp"

using namespace Aspen;
using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

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
    auto state = reactor.commit(0);
    series->set_complete();
    state = reactor.commit(1);
    bool found_cancel = false;
    auto operation = operations->pop();
    auto cancel =
      std::get_if<TestOrderExecutionClient::CancelOperation>(operation.get());
    REQUIRE(cancel);
    REQUIRE(cancel->m_id == 123);
  }
}
