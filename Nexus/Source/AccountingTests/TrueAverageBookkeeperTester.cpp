#include <doctest/doctest.h>
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"

using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  auto TST = parse_ticker("TST.TSX");
  auto S32 = parse_ticker("S32.ASX");
}

TEST_SUITE("TrueAverageBookkeeper") {
  TEST_CASE("empty_bookkeeper") {
    auto bookkeeper = TrueAverageBookkeeper();
    auto cad_inventory = bookkeeper.get_inventory(TST);
    REQUIRE(is_empty(cad_inventory));
    auto aud_inventory = bookkeeper.get_inventory(S32);
    REQUIRE(is_empty(aud_inventory));
    auto cad_total = bookkeeper.get_total(CAD);
    REQUIRE(is_empty(cad_total));
    auto aud_total = bookkeeper.get_total(AUD);
    REQUIRE(is_empty(aud_total));
  }

  TEST_CASE("buy_and_sell_to_flat") {
    auto bookkeeper = TrueAverageBookkeeper();
    bookkeeper.record(TST, CAD, 100, 1000, Quantity::CENT);
    auto& inventory1 = bookkeeper.get_inventory(TST);
    REQUIRE(inventory1.m_position.m_quantity == 100);
    REQUIRE(inventory1.m_position.m_cost_basis == 1000);
    REQUIRE(inventory1.m_fees == Quantity::CENT);
    REQUIRE(inventory1.m_gross_profit_and_loss == 0);
    bookkeeper.record(TST, CAD, -100, -1200, Quantity::CENT);
    auto& inventory2 = bookkeeper.get_inventory(TST);
    REQUIRE(inventory2.m_position.m_quantity == 0);
    REQUIRE(inventory2.m_position.m_cost_basis == 0);
    REQUIRE(inventory2.m_fees == 2 * Quantity::CENT);
    REQUIRE(inventory2.m_gross_profit_and_loss == 200);
    auto& total = bookkeeper.get_total(CAD);
    REQUIRE(total.m_position.m_quantity == 0);
    REQUIRE(total.m_position.m_cost_basis == 0);
    REQUIRE(total.m_fees == 2 * Quantity::CENT);
    REQUIRE(total.m_gross_profit_and_loss == 200);
  }

  TEST_CASE("sell_to_short_and_buy_to_flat") {
    auto bookkeeper = TrueAverageBookkeeper();
    bookkeeper.record(TST, CAD, -100, -1000, Quantity::CENT);
    auto& inventory1 = bookkeeper.get_inventory(TST);
    REQUIRE(inventory1.m_position.m_quantity == -100);
    REQUIRE(inventory1.m_position.m_cost_basis == -1000);
    REQUIRE(inventory1.m_fees == Quantity::CENT);
    REQUIRE(inventory1.m_gross_profit_and_loss == 0);
    bookkeeper.record(TST, CAD, 100, 1200, Quantity::CENT);
    auto& inventory2 = bookkeeper.get_inventory(TST);
    REQUIRE(inventory2.m_position.m_quantity == 0);
    REQUIRE(inventory2.m_position.m_cost_basis == 0);
    REQUIRE(inventory2.m_fees == 2 * Quantity::CENT);
    REQUIRE(inventory2.m_gross_profit_and_loss == -200);
    auto& total = bookkeeper.get_total(CAD);
    REQUIRE(total.m_position.m_quantity == 0);
    REQUIRE(total.m_position.m_cost_basis == 0);
    REQUIRE(total.m_fees == 2 * Quantity::CENT);
    REQUIRE(total.m_gross_profit_and_loss == -200);
  }

  TEST_CASE("multiple_buys_and_sells") {
    auto bookkeeper = TrueAverageBookkeeper();
    bookkeeper.record(TST, CAD, 100, 1000, 0);
    bookkeeper.record(TST, CAD, 100, 1200, 0);
    auto& inventory1 = bookkeeper.get_inventory(TST);
    REQUIRE(inventory1.m_position.m_quantity == 200);
    REQUIRE(inventory1.m_position.m_cost_basis == 2200);
    bookkeeper.record(TST, CAD, -150, -1950, 0);
    auto& inventory2 = bookkeeper.get_inventory(TST);
    REQUIRE(inventory2.m_position.m_quantity == 50);
    REQUIRE(inventory2.m_position.m_cost_basis == 550);
    REQUIRE(inventory2.m_gross_profit_and_loss == 300);
  }

  TEST_CASE("multiple_inventories_and_currencies") {
    auto bookkeeper = TrueAverageBookkeeper();
    bookkeeper.record(TST, CAD, 100, 1000, Quantity::CENT);
    bookkeeper.record(S32, AUD, 50, 7500, Quantity::CENT);
    auto& tst_inventory = bookkeeper.get_inventory(TST);
    REQUIRE(tst_inventory.m_position.m_quantity == 100);
    REQUIRE(tst_inventory.m_position.m_cost_basis == 1000);
    auto& msft_inventory = bookkeeper.get_inventory(S32);
    REQUIRE(msft_inventory.m_position.m_quantity == 50);
    REQUIRE(msft_inventory.m_position.m_cost_basis == 7500);
    auto& cad_total = bookkeeper.get_total(CAD);
    REQUIRE(cad_total.m_position.m_quantity == 100);
    REQUIRE(cad_total.m_position.m_cost_basis == 1000);
    REQUIRE(cad_total.m_fees == Quantity::CENT);
    auto& aud_total = bookkeeper.get_total(AUD);
    REQUIRE(aud_total.m_position.m_quantity == 50);
    REQUIRE(aud_total.m_position.m_cost_basis == 7500);
    REQUIRE(aud_total.m_fees == Quantity::CENT);
  }
}
