#include <doctest/doctest.h>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"

using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  using TestPosition = Position<Security>;
  using TestInventory = Inventory<TestPosition>;
  using TestBookkeeper = TrueAverageBookkeeper<TestInventory>;

  const auto TST = Security("TST", NYSE);
}

TEST_SUITE("Portfolio") {
  TEST_CASE("bookkeeper_constructor") {
    auto bookkeeper = TestBookkeeper();
    bookkeeper.RecordTransaction(TST, USD, 1, Money::ONE, Money::ZERO);
    auto portfolio = Portfolio(bookkeeper);
    REQUIRE(portfolio.GetSecurityEntries().at(TST).m_unrealized == Money::ZERO);
    portfolio.Update(TST, 3 * Money::ONE, 2 * Money::ONE);
    REQUIRE(portfolio.GetUnrealizedProfitAndLosses().at(USD) == Money::ONE);
    REQUIRE(portfolio.GetSecurityEntries().at(TST).m_unrealized == Money::ONE);
  }
}
