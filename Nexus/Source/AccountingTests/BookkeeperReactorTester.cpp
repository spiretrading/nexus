#include "Nexus/AccountingTests/BookkeeperReactorTester.hpp"
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
using namespace Nexus::Accounting::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  using TestBookkeeper = TrueAverageBookkeeper<Inventory<Position<Security>>>;

  const auto TST_SECURITY = Security("TST", DefaultMarkets::NYSE(),
    DefaultCountries::US());
}

void BookkeeperReactorTester::TestSingleOrder() {
  auto commits = Beam::Queue<bool>();
  auto trigger = Trigger(
    [&] {
      commits.Push(true);
    });
  Trigger::set_trigger(trigger);
  auto order = PrimitiveOrder(OrderInfo(OrderFields::BuildLimitOrder(
    TST_SECURITY, DefaultCurrencies::USD(), Side::BID, "NYSE", 1000,
    Money::ONE), 10, ptime(date(2019, 10, 3))));
  auto bookkeeper = BookkeeperReactor<TestBookkeeper>(constant(&order));
  SetOrderStatus(order, OrderStatus::NEW, ptime(date(2019, 10, 3)));
  FillOrder(order, 100, ptime(date(2019, 10, 3)));
  CPPUNIT_ASSERT(!Aspen::has_evaluation(bookkeeper.commit(0)));
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(!Aspen::has_evaluation(bookkeeper.commit(1)));
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(!Aspen::has_evaluation(bookkeeper.commit(2)));
  commits.Top();
  commits.Pop();
  CPPUNIT_ASSERT(Aspen::has_evaluation(bookkeeper.commit(3)));
  auto inventory = bookkeeper.eval();
  CPPUNIT_ASSERT(inventory.m_volume == 100);
  CPPUNIT_ASSERT(inventory.m_transactionCount == 1);
  CPPUNIT_ASSERT(inventory.m_fees == Money::ZERO);
  CPPUNIT_ASSERT(inventory.m_position.m_key.m_index == TST_SECURITY);
  CPPUNIT_ASSERT(inventory.m_position.m_key.m_currency ==
    DefaultCurrencies::USD());
  CPPUNIT_ASSERT(inventory.m_position.m_quantity == 100);
  CPPUNIT_ASSERT(inventory.m_position.m_costBasis == 100 * Money::ONE);
}
