#include <doctest/doctest.h>
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/Security.hpp"

using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  auto TST = Security("TST", TSX);
  auto S32 = Security("S32", ASX);
}

TEST_SUITE("TrueAverageBookkeeper") {
  TEST_CASE("empty_bookkeeper") {
    auto bookkeeper = TrueAverageBookkeeper();
    auto cad_inventory = bookkeeper.get_inventory(TST, CAD);
    REQUIRE(is_empty(cad_inventory));
    auto aud_inventory = bookkeeper.get_inventory(S32, AUD);
    REQUIRE(is_empty(aud_inventory));
    auto cad_total = bookkeeper.get_total(CAD);
    REQUIRE(is_empty(cad_total));
    auto aud_total = bookkeeper.get_total(AUD);
    REQUIRE(is_empty(aud_total));
  }

  TEST_CASE("buy_and_sell_to_flat") {
    auto bookkeeper = TrueAverageBookkeeper();
    bookkeeper.record(TST, CAD, 100, 1000 * Money::ONE, Money::CENT);
    auto& inventory1 = bookkeeper.get_inventory(TST, CAD);
    REQUIRE(inventory1.m_position.m_quantity == 100);
    REQUIRE(inventory1.m_position.m_cost_basis == 1000 * Money::ONE);
    REQUIRE(inventory1.m_fees == Money::CENT);
    REQUIRE(inventory1.m_gross_profit_and_loss == Money::ZERO);
    bookkeeper.record(TST, CAD, -100, -1200 * Money::ONE, Money::CENT);
    auto& inventory2 = bookkeeper.get_inventory(TST, CAD);
    REQUIRE(inventory2.m_position.m_quantity == 0);
    REQUIRE(inventory2.m_position.m_cost_basis == Money::ZERO);
    REQUIRE(inventory2.m_fees == 2 * Money::CENT);
    REQUIRE(inventory2.m_gross_profit_and_loss == 200 * Money::ONE);
    auto& total = bookkeeper.get_total(CAD);
    REQUIRE(total.m_position.m_quantity == 0);
    REQUIRE(total.m_position.m_cost_basis == Money::ZERO);
    REQUIRE(total.m_fees == 2 * Money::CENT);
    REQUIRE(total.m_gross_profit_and_loss == 200 * Money::ONE);
  }

  TEST_CASE("sell_to_short_and_buy_to_flat") {
    auto bookkeeper = TrueAverageBookkeeper();
    bookkeeper.record(TST, CAD, -100, -1000 * Money::ONE, Money::CENT);
    auto& inventory1 = bookkeeper.get_inventory(TST, CAD);
    REQUIRE(inventory1.m_position.m_quantity == -100);
    REQUIRE(inventory1.m_position.m_cost_basis == -1000 * Money::ONE);
    REQUIRE(inventory1.m_fees == Money::CENT);
    REQUIRE(inventory1.m_gross_profit_and_loss == Money::ZERO);
    bookkeeper.record(TST, CAD, 100, 1200 * Money::ONE, Money::CENT);
    auto& inventory2 = bookkeeper.get_inventory(TST, CAD);
    REQUIRE(inventory2.m_position.m_quantity == 0);
    REQUIRE(inventory2.m_position.m_cost_basis == Money::ZERO);
    REQUIRE(inventory2.m_fees == 2 * Money::CENT);
    REQUIRE(inventory2.m_gross_profit_and_loss == -200 * Money::ONE);
    auto& total = bookkeeper.get_total(CAD);
    REQUIRE(total.m_position.m_quantity == 0);
    REQUIRE(total.m_position.m_cost_basis == Money::ZERO);
    REQUIRE(total.m_fees == 2 * Money::CENT);
    REQUIRE(total.m_gross_profit_and_loss == -200 * Money::ONE);
  }

  TEST_CASE("multiple_buys_and_sells") {
    auto bookkeeper = TrueAverageBookkeeper();
    bookkeeper.record(TST, CAD, 100, 1000 * Money::ONE, Money::ZERO);
    bookkeeper.record(TST, CAD, 100, 1200 * Money::ONE, Money::ZERO);
    auto& inventory1 = bookkeeper.get_inventory(TST, CAD);
    REQUIRE(inventory1.m_position.m_quantity == 200);
    REQUIRE(inventory1.m_position.m_cost_basis == 2200 * Money::ONE);
    bookkeeper.record(TST, CAD, -150, -1950 * Money::ONE, Money::ZERO);
    auto& inventory2 = bookkeeper.get_inventory(TST, CAD);
    REQUIRE(inventory2.m_position.m_quantity == 50);
    REQUIRE(inventory2.m_position.m_cost_basis == 550 * Money::ONE);
    REQUIRE(inventory2.m_gross_profit_and_loss == 300 * Money::ONE);
  }

  TEST_CASE("multiple_inventories_and_currencies") {
    auto bookkeeper = TrueAverageBookkeeper();
    bookkeeper.record(TST, CAD, 100, 1000 * Money::ONE, Money::CENT);
    bookkeeper.record(S32, AUD, 50, 7500 * Money::ONE, Money::CENT);
    auto& tst_inventory = bookkeeper.get_inventory(TST, CAD);
    REQUIRE(tst_inventory.m_position.m_quantity == 100);
    REQUIRE(tst_inventory.m_position.m_cost_basis == 1000 * Money::ONE);
    auto& msft_inventory = bookkeeper.get_inventory(S32, AUD);
    REQUIRE(msft_inventory.m_position.m_quantity == 50);
    REQUIRE(msft_inventory.m_position.m_cost_basis == 7500 * Money::ONE);
    auto& cad_total = bookkeeper.get_total(CAD);
    REQUIRE(cad_total.m_position.m_quantity == 100);
    REQUIRE(cad_total.m_position.m_cost_basis == 1000 * Money::ONE);
    REQUIRE(cad_total.m_fees == Money::CENT);
    auto& aud_total = bookkeeper.get_total(AUD);
    REQUIRE(aud_total.m_position.m_quantity == 50);
    REQUIRE(aud_total.m_position.m_cost_basis == 7500 * Money::ONE);
    REQUIRE(aud_total.m_fees == Money::CENT);
  }
}
