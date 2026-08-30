#include <Aspen/Aspen.hpp>
#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/OrderExecutionService/OrderWrapperReactor.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Aspen;
using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Tests;
using namespace Nexus::Venues;

TEST_SUITE("OrderWrapperReactor") {
  TEST_CASE("single_order_evaluation") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.push(true);
    });
    auto flag = CommitFlag();
    flag.set_trigger(&trigger);
    auto scope = CommitFlagScope(flag);
    auto ticker = parse_ticker("TST.TSX");
    auto fields =
      make_limit_order_fields(ticker, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto order = std::make_shared<PrimitiveOrder>(
      OrderInfo(fields, 1, false, time_from_string("2024-07-21 10:00:00.000")));
    auto reactor = OrderWrapperReactor(order);
    auto sequence = 0;
    auto require_evaluation = [&] {
      while(true) {
        flag.clear();
        auto state = reactor.commit(sequence);
        ++sequence;
        if(has_evaluation(state)) {
          return;
        }
        if(!has_continuation(state)) {
          commits.pop();
        }
      }
    };
    require_evaluation();
    REQUIRE(reactor.eval() == order);
    accept(*order);
    require_evaluation();
    REQUIRE(reactor.eval() == order);
  }
}
