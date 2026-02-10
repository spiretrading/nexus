#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Accounting/Inventory.hpp"
#include "Nexus/Definitions/Ticker.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

namespace {
  auto TST = parse_ticker("TST.TSX");
}

TEST_SUITE("Inventory") {
  TEST_CASE("constructor") {
    auto inventory = Inventory();
    REQUIRE(inventory.m_position.m_quantity == 0);
    REQUIRE(inventory.m_position.m_cost_basis == Money::ZERO);
    REQUIRE(inventory.m_gross_profit_and_loss == Money::ZERO);
    REQUIRE(inventory.m_fees == Money::ZERO);
    REQUIRE(inventory.m_volume == 0);
    REQUIRE(inventory.m_transaction_count == 0);
  }

  TEST_CASE("key_constructor") {
    auto inventory = Inventory(TST, CAD);
    REQUIRE(inventory.m_position.m_ticker == TST);
    REQUIRE(inventory.m_position.m_currency == CAD);
    REQUIRE(inventory.m_position.m_quantity == 0);
    REQUIRE(inventory.m_position.m_cost_basis == Money::ZERO);
    REQUIRE(inventory.m_gross_profit_and_loss == Money::ZERO);
    REQUIRE(inventory.m_fees == Money::ZERO);
    REQUIRE(inventory.m_volume == 0);
    REQUIRE(inventory.m_transaction_count == 0);
  }

  TEST_CASE("full_constructor") {
    auto position = Position(TST, CAD, 100, 1000 * Money::ONE);
    auto pnl = 50 * Money::ONE;
    auto fees = Money::CENT;
    auto volume = 200;
    auto transaction_count = 2;
    auto inventory = Inventory(position, pnl, fees, volume, transaction_count);
    REQUIRE(inventory.m_position == position);
    REQUIRE(inventory.m_gross_profit_and_loss == pnl);
    REQUIRE(inventory.m_fees == fees);
    REQUIRE(inventory.m_volume == volume);
    REQUIRE(inventory.m_transaction_count == transaction_count);
  }

  TEST_CASE("is_empty") {
    auto empty_by_default = Inventory();
    REQUIRE(is_empty(empty_by_default));
    auto empty_by_key = Inventory(TST, CAD);
    REQUIRE(is_empty(empty_by_key));
    auto non_empty = Inventory(TST, CAD);
    non_empty.m_volume = 100;
    REQUIRE(!is_empty(non_empty));
  }

  TEST_CASE("shuttle") {
    auto position = Position(TST, CAD, 100, 1000 * Money::ONE);
    auto pnl = 50 * Money::ONE;
    auto fees = Money::CENT;
    auto volume = 200;
    auto transaction_count = 2;
    auto inventory = Inventory(position, pnl, fees, volume, transaction_count);
    test_round_trip_shuttle(inventory);
    REQUIRE(
      to_string(inventory) == "((TST.TSX CAD 100 1000.00) 50.00 0.01 200 2)");
  }
}
