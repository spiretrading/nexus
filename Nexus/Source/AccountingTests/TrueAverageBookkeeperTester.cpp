#include "Nexus/AccountingTests/TrueAverageBookkeeperTester.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/AccountingTests/BookkeeperTestHelper.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"

using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::Accounting::Tests;
using namespace std;

void TrueAverageBookkeeperTester::TestAddTransactionNoFees() {
  TrueAverageBookkeeper bookkeeper;

  // Buy 1 unit of Coke for $1.
  bookkeeper.RecordTransaction("Coke", DefaultCurrencies::USD(), 1, Money::ONE,
    Money::ZERO);

  // Pull it out and run Tests on it.
  auto cokeEntry = bookkeeper.GetInventory("Coke", DefaultCurrencies::USD());
  CPPUNIT_ASSERT(cokeEntry.m_position.m_costBasis == Money::ONE);
  CPPUNIT_ASSERT(cokeEntry.m_fees == Money::ZERO);
  CPPUNIT_ASSERT(cokeEntry.m_grossProfitAndLoss == Money::ZERO);
  CPPUNIT_ASSERT(cokeEntry.m_position.m_quantity == 1);
}

void TrueAverageBookkeeperTester::TestAddTransactionWithFees() {
  TrueAverageBookkeeper bookkeeper;

  // Buy 1 unit of Coke for $1 with $1 worth of fees.
  bookkeeper.RecordTransaction("Coke", DefaultCurrencies::USD(), 1, Money::ONE,
    Money::ONE);

  // Pull it out and run Tests on it.
  auto cokeEntry = bookkeeper.GetInventory("Coke", DefaultCurrencies::USD());
  CPPUNIT_ASSERT(cokeEntry.m_position.m_costBasis == Money::ONE);
  CPPUNIT_ASSERT(cokeEntry.m_fees == Money::ONE);
  CPPUNIT_ASSERT(cokeEntry.m_grossProfitAndLoss == Money::ZERO);
  CPPUNIT_ASSERT(cokeEntry.m_position.m_quantity == 1);
}

void TrueAverageBookkeeperTester::TestAddRemoveTransactionAtFlat() {
  TestAddRemoveHelper<TrueAverageBookkeeper>(1, Money::ONE, Money::ZERO, -1,
    Money::ONE, Money::ZERO, Money::ZERO, Money::ZERO);
}

void TrueAverageBookkeeperTester::TestAddRemoveTransactionAtProfit() {
  TestAddRemoveHelper<TrueAverageBookkeeper>(1, Money::ONE, Money::ZERO, -1,
    2 * Money::ONE + 50 * Money::CENT, Money::ZERO, Money::ZERO,
    Money::ONE + 50 * Money::CENT);
}

void TrueAverageBookkeeperTester::TestAddRemoveTransactionAtLoss() {
  TestAddRemoveHelper<TrueAverageBookkeeper>(1, Money::ONE, Money::ZERO, -1,
    50 * Money::CENT, Money::ZERO, Money::ZERO, -50 * Money::CENT);
}

void TrueAverageBookkeeperTester::TestAddTwoRemoveOneTransactionAtFlat() {
  TestAddRemoveHelper<TrueAverageBookkeeper>(2, 2 * Money::ONE, Money::ZERO, -1,
    Money::ONE, Money::ZERO, Money::ONE, Money::ZERO);
}

void TrueAverageBookkeeperTester::TestAddTwoRemoveOneTransactionAtProfit() {
  TestAddRemoveHelper<TrueAverageBookkeeper>(2, 2 * Money::ONE, Money::ZERO, -1,
    2 * Money::ONE, Money::ZERO, Money::ONE, Money::ONE);
}

void TrueAverageBookkeeperTester::TestAddTwoRemoveOneTransactionAtLoss() {
  TestAddRemoveHelper<TrueAverageBookkeeper>(2, 2 * Money::ONE, Money::ZERO, -1,
    50 * Money::CENT, Money::ZERO, Money::ONE, -50 * Money::CENT);
}

void TrueAverageBookkeeperTester::TestAddOneRemoveTwoTransactionAtFlat() {
  TestAddRemoveHelper<TrueAverageBookkeeper>(1, Money::ONE, Money::ZERO, -2,
    2 * Money::ONE, Money::ZERO, -Money::ONE, Money::ZERO);
}

void TrueAverageBookkeeperTester::TestAddOneRemoveTwoTransactionAtProfit() {
  TestAddRemoveHelper<TrueAverageBookkeeper>(1, Money::ONE, Money::ZERO, -2,
    4 * Money::ONE, Money::ZERO, -2 * Money::ONE, Money::ONE);
}

void TrueAverageBookkeeperTester::TestAddOneRemoveTwoTransactionAtLoss() {
 TestAddRemoveHelper<TrueAverageBookkeeper>(1, Money::ONE, Money::ZERO, -2,
    Money::ONE, Money::ZERO, -50 * Money::CENT, -50 * Money::CENT);
}
