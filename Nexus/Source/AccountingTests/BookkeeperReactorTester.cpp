#include <doctest/doctest.h>
#include "Nexus/Accounting/BookkeeperReactor.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Aspen;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  using TestBookkeeper = TrueAverageBookkeeper<Inventory<Position<Security>>>;

  const auto TST = Security("TST", NYSE);
}

TEST_SUITE("BookkeeperReactor") {
  TEST_CASE("single_order") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto order = PrimitiveOrder(OrderInfo(make_limit_order_fields(
      TST, USD, Side::BID, "NYSE", 1000, Money::ONE), 10, ptime(date(2019, 10, 3))));
    set_order_status(order, OrderStatus::NEW, ptime(date(2019, 10, 3)));
    fill(order, 100, ptime(date(2019, 10, 3)));
    auto bookkeeper = make_bookkeeper_reactor<TestBookkeeper>(constant(&order));
    for(auto i = 0; i < 10; ++i) {
      auto state = bookkeeper.commit(i);
      if(has_evaluation(state)) {
        break;
      } else if(has_continuation(state)) {
        continue;
      } else {
        commits.Pop();
      }
    }
    auto inventory = bookkeeper.eval();
    REQUIRE(inventory.m_volume == 100);
    REQUIRE(inventory.m_transaction_count == 1);
    REQUIRE(inventory.m_fees == Money::ZERO);
    REQUIRE(inventory.m_position.m_key.m_index == TST);
    REQUIRE(inventory.m_position.m_key.m_currency == USD);
    REQUIRE(inventory.m_position.m_quantity == 100);
    REQUIRE(inventory.m_position.m_cost_basis == 100 * Money::ONE);
  }
}
