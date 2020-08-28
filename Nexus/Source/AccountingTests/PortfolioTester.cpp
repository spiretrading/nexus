#include <doctest/doctest.h>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Nexus;
using namespace Nexus::Accounting;

namespace {
  using TestPosition = Position<Security>;
  using TestInventory = Inventory<TestPosition>;
  using TestBookkeeper = TrueAverageBookkeeper<TestInventory>;

  const auto TST = Security("TST", DefaultMarkets::NYSE(),
    DefaultCountries::US());
}

TEST_SUITE("Portfolio") {
  TEST_CASE("bookkeeper_constructor") {
    auto bookkeeper = TestBookkeeper();
    bookkeeper.RecordTransaction(TST, DefaultCurrencies::USD(), 1, Money::ONE,
      Money::ZERO);
    auto portfolio = Portfolio(GetDefaultMarketDatabase(), bookkeeper);
    portfolio.Update(TST, 3 * Money::ONE, 2 * Money::ONE);
    REQUIRE(portfolio.GetUnrealizedProfitAndLosses().at(
      DefaultCurrencies::USD()) == Money::ONE);
    REQUIRE(portfolio.GetSecurityEntries().at(TST).m_unrealized == Money::ONE);
  }
}
