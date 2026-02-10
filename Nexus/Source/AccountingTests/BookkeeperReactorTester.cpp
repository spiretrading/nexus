#include <doctest/doctest.h>
#include "Nexus/Accounting/BookkeeperReactor.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Aspen;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

namespace {
  const auto TST = parse_ticker("TST.TSX");
}

TEST_SUITE("BookkeeperReactor") {
  TEST_CASE("single_order") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.push(true);
    });
    Trigger::set_trigger(trigger);
    auto order = std::make_shared<PrimitiveOrder>(
      OrderInfo(make_limit_order_fields(TST, CAD, Side::BID, "TSX", 1000,
        Money::ONE), 10, ptime(date(2019, 10, 3))));
    set_order_status(*order, OrderStatus::NEW, ptime(date(2019, 10, 3)));
    fill(*order, 100, ptime(date(2019, 10, 3)));
    auto bookkeeper = make_bookkeeper_reactor<TrueAverageBookkeeper>(
      constant(std::static_pointer_cast<Order>(order)));
    for(auto i = 0; i < 10; ++i) {
      auto state = bookkeeper.commit(i);
      if(has_evaluation(state)) {
        break;
      } else if(has_continuation(state)) {
        continue;
      } else {
        commits.pop();
      }
    }
    auto inventory = bookkeeper.eval();
    REQUIRE(inventory.m_volume == 100);
    REQUIRE(inventory.m_transaction_count == 1);
    REQUIRE(inventory.m_fees == Money::ZERO);
    REQUIRE(inventory.m_position.m_ticker == TST);
    REQUIRE(inventory.m_position.m_currency == CAD);
    REQUIRE(inventory.m_position.m_quantity == 100);
    REQUIRE(inventory.m_position.m_cost_basis == 100 * Money::ONE);
  }
}
