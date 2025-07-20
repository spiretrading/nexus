#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Accounting/Inventory.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/Definitions/Security.hpp"

using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  using TestInventory = Inventory<Position<Security>>;

  auto TST = Security("TST", TSX);
}

TEST_SUITE("Inventory") {
  TEST_CASE("constructor") {
    auto inventory = TestInventory();
    REQUIRE(inventory.m_position.m_quantity == 0);
    REQUIRE(inventory.m_position.m_cost_basis == Money::ZERO);
    REQUIRE(inventory.m_gross_profit_and_loss == Money::ZERO);
    REQUIRE(inventory.m_fees == Money::ZERO);
    REQUIRE(inventory.m_volume == 0);
    REQUIRE(inventory.m_transaction_count == 0);
  }

  TEST_CASE("key_constructor") {
    auto key = TestInventory::Position::Key(TST, CAD);
    auto inventory = TestInventory(key);
    REQUIRE(inventory.m_position.m_key == key);
    REQUIRE(inventory.m_position.m_quantity == 0);
    REQUIRE(inventory.m_position.m_cost_basis == Money::ZERO);
    REQUIRE(inventory.m_gross_profit_and_loss == Money::ZERO);
    REQUIRE(inventory.m_fees == Money::ZERO);
    REQUIRE(inventory.m_volume == 0);
    REQUIRE(inventory.m_transaction_count == 0);
  }

  TEST_CASE("full_constructor") {
    auto key = TestInventory::Position::Key(TST, CAD);
    auto position = TestInventory::Position(key, 100, 1000 * Money::ONE);
    auto pnl = 50 * Money::ONE;
    auto fees = Money::CENT;
    auto volume = 200;
    auto transaction_count = 2;
    auto inventory =
      TestInventory(position, pnl, fees, volume, transaction_count);
    REQUIRE(inventory.m_position == position);
    REQUIRE(inventory.m_gross_profit_and_loss == pnl);
    REQUIRE(inventory.m_fees == fees);
    REQUIRE(inventory.m_volume == volume);
    REQUIRE(inventory.m_transaction_count == transaction_count);
  }

  TEST_CASE("is_empty") {
    auto key = TestInventory::Position::Key(TST, CAD);
    auto empty_by_default = TestInventory();
    REQUIRE(is_empty(empty_by_default));
    auto empty_by_key = TestInventory(key);
    REQUIRE(is_empty(empty_by_key));
    auto non_empty = TestInventory(key);
    non_empty.m_volume = 100;
    REQUIRE(!is_empty(non_empty));
  }

  TEST_CASE("shuttle") {
    auto key = TestInventory::Position::Key(TST, CAD);
    auto position = TestInventory::Position(key, 100, 1000 * Money::ONE);
    auto pnl = 50 * Money::ONE;
    auto fees = Money::CENT;
    auto volume = 200;
    auto transaction_count = 2;
    auto inventory =
      TestInventory(position, pnl, fees, volume, transaction_count);
    Beam::Serialization::Tests::TestRoundTripShuttle(inventory);
  }

  TEST_CASE("output_stream") {
    auto key = TestInventory::Position::Key(TST, DefaultCurrencies::CAD);
    auto position = TestInventory::Position(key, 100, 1000 * Money::ONE);
    auto pnl = 50 * Money::ONE;
    auto fees = Money::CENT;
    auto volume = 200;
    auto transaction_count = 2;
    auto inventory =
      TestInventory(position, pnl, fees, volume, transaction_count);
    auto stream = std::stringstream();
    stream << inventory;
    REQUIRE(stream.str() == "(((TST.TSX CAD) 100 1000.00) 50.00 0.01 200 2)");
  }
}
