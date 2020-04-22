#include <doctest/doctest.h>
#include "Nexus/Accounting/BookkeeperReactor.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Aspen;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  using TestBookkeeper = TrueAverageBookkeeper<Inventory<Position<Security>>>;

  const auto TST_SECURITY = Security("TST", DefaultMarkets::NYSE(),
    DefaultCountries::US());
}

TEST_SUITE("BookkeeperReactor") {
  TEST_CASE("single_order") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto order = PrimitiveOrder(OrderInfo(OrderFields::BuildLimitOrder(
      TST_SECURITY, DefaultCurrencies::USD(), Side::BID, "NYSE", 1000,
      Money::ONE), 10, ptime(date(2019, 10, 3))));
    SetOrderStatus(order, OrderStatus::NEW, ptime(date(2019, 10, 3)));
    FillOrder(order, 100, ptime(date(2019, 10, 3)));
    auto bookkeeper = BookkeeperReactor<TestBookkeeper>(constant(&order));
    for(auto i = 0; i < 10; ++i) {
      auto state = bookkeeper.commit(i);
      if(Aspen::has_evaluation(state)) {
        break;
      } else if(Aspen::has_continuation(state)) {
        continue;
      } else {
        commits.Top();
        commits.Pop();
      }
    }
    auto inventory = bookkeeper.eval();
    REQUIRE(inventory.m_volume == 100);
    REQUIRE(inventory.m_transactionCount == 1);
    REQUIRE(inventory.m_fees == Money::ZERO);
    REQUIRE(inventory.m_position.m_key.m_index == TST_SECURITY);
    REQUIRE(inventory.m_position.m_key.m_currency ==
      DefaultCurrencies::USD());
    REQUIRE(inventory.m_position.m_quantity == 100);
    REQUIRE(inventory.m_position.m_costBasis == 100 * Money::ONE);
  }
}
