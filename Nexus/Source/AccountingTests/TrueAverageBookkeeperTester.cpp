#include <doctest/doctest.h>
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/AccountingTests/BookkeeperTestHelper.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"

using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::Accounting::Tests;

namespace {
  using TestPosition = Position<std::string>;
  using TestInventory = Inventory<TestPosition>;
  using TestTrueAverageBookkeeper = TrueAverageBookkeeper<TestInventory>;
}

TEST_SUITE("TrueAverageBookkeeper") {
  TEST_CASE("add_transaction_no_fees") {
    auto bookkeeper = TestTrueAverageBookkeeper();

    // Buy 1 unit of Coke for $1.
    bookkeeper.RecordTransaction("Coke", DefaultCurrencies::USD(), 1,
      Money::ONE, Money::ZERO);

    // Pull it out and run Tests on it.
    auto cokeEntry = bookkeeper.GetInventory("Coke", DefaultCurrencies::USD());
    REQUIRE(cokeEntry.m_position.m_costBasis == Money::ONE);
    REQUIRE(cokeEntry.m_fees == Money::ZERO);
    REQUIRE(cokeEntry.m_grossProfitAndLoss == Money::ZERO);
    REQUIRE(cokeEntry.m_position.m_quantity == 1);
  }

  TEST_CASE("add_transaction_with_fees") {
    auto bookkeeper = TestTrueAverageBookkeeper();

    // Buy 1 unit of Coke for $1 with $1 worth of fees.
    bookkeeper.RecordTransaction("Coke", DefaultCurrencies::USD(), 1,
      Money::ONE, Money::ONE);

    // Pull it out and run Tests on it.
    auto cokeEntry = bookkeeper.GetInventory("Coke", DefaultCurrencies::USD());
    REQUIRE(cokeEntry.m_position.m_costBasis == Money::ONE);
    REQUIRE(cokeEntry.m_fees == Money::ONE);
    REQUIRE(cokeEntry.m_grossProfitAndLoss == Money::ZERO);
    REQUIRE(cokeEntry.m_position.m_quantity == 1);
  }

  TEST_CASE("add_remove_transaction_at_flat") {
    TestAddRemoveHelper<TestTrueAverageBookkeeper>(1, Money::ONE, Money::ZERO,
      -1, Money::ONE, Money::ZERO, Money::ZERO, Money::ZERO);
  }

  TEST_CASE("add_remove_transaction_at_profit") {
    TestAddRemoveHelper<TestTrueAverageBookkeeper>(1, Money::ONE, Money::ZERO,
      -1, 2 * Money::ONE + 50 * Money::CENT, Money::ZERO, Money::ZERO,
      Money::ONE + 50 * Money::CENT);
  }

  TEST_CASE("add_remove_transaction_at_loss") {
    TestAddRemoveHelper<TestTrueAverageBookkeeper>(1, Money::ONE, Money::ZERO,
      -1, 50 * Money::CENT, Money::ZERO, Money::ZERO, -50 * Money::CENT);
  }

  TEST_CASE("add_two_remove_one_transaction_at_flat") {
    TestAddRemoveHelper<TestTrueAverageBookkeeper>(2, 2 * Money::ONE,
      Money::ZERO, -1, Money::ONE, Money::ZERO, Money::ONE, Money::ZERO);
  }

  TEST_CASE("add_two_remove_one_transaction_at_profit") {
    TestAddRemoveHelper<TestTrueAverageBookkeeper>(2, 2 * Money::ONE,
      Money::ZERO, -1, 2 * Money::ONE, Money::ZERO, Money::ONE, Money::ONE);
  }

  TEST_CASE("add_two_remove_one_transaction_at_loss") {
    TestAddRemoveHelper<TestTrueAverageBookkeeper>(2, 2 * Money::ONE,
      Money::ZERO, -1, 50 * Money::CENT, Money::ZERO, Money::ONE,
      -50 * Money::CENT);
  }

  TEST_CASE("add_one_remove_two_transaction_at_flat") {
    TestAddRemoveHelper<TestTrueAverageBookkeeper>(1, Money::ONE, Money::ZERO,
      -2, 2 * Money::ONE, Money::ZERO, -Money::ONE, Money::ZERO);
  }

  TEST_CASE("add_one_remove_two_transaction_at_profit") {
    TestAddRemoveHelper<TestTrueAverageBookkeeper>(1, Money::ONE, Money::ZERO,
      -2, 4 * Money::ONE, Money::ZERO, -2 * Money::ONE, Money::ONE);
  }

  TEST_CASE("add_one_remove_two_transaction_at_loss") {
    TestAddRemoveHelper<TestTrueAverageBookkeeper>(1, Money::ONE, Money::ZERO,
      -2, Money::ONE, Money::ZERO, -50 * Money::CENT, -50 * Money::CENT);
  }

  TEST_CASE("reset_cost_basis_when_flat") {
    auto bookkeeper = TestTrueAverageBookkeeper();
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.835"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.82"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.83"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.83"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.825"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.825"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.825"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.825"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.825"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.825"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.825"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.825"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.825"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.825"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.825"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -100,
      -100 * *Money::FromValue("32.825"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -700,
      -700 * *Money::FromValue("32.83"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -800,
      -800 * *Money::FromValue("32.83"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 100,
      100 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 100,
      100 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 100,
      100 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 100,
      100 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 200,
      200 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 100,
      100 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 300,
      300 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 200,
      200 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 200,
      200 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 200,
      200 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 200,
      200 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 900,
      900 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 100,
      100 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 100,
      100 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 100,
      100 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 100,
      100 * *Money::FromValue("32.84"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), -200,
      -200 * *Money::FromValue("32.885"), Money::ZERO);
    bookkeeper.RecordTransaction("TST", DefaultCurrencies::CAD(), 300,
      300 * *Money::FromValue("32.89"), Money::ZERO);
    auto inventory = bookkeeper.GetInventory("TST", DefaultCurrencies::CAD());
    REQUIRE(inventory.m_position.m_costBasis == Money::ZERO);
  }
}
