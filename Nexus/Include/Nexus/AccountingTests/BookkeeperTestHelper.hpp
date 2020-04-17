#ifndef NEXUS_BOOKKEEPER_TEST_HELPER_HPP
#define NEXUS_BOOKKEEPER_TEST_HELPER_HPP
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"

namespace Nexus::Accounting::Tests {

  //! Helper test for adding and then removing inventory from a Bookkeeper.
  /*!
    \param addQuantity The inventory quantity to add.
    \param addValue The value of the inventory added.
    \param addFees The transaction fees to add the inventory.
    \param removeQuantity The inventory quantity to remove.
    \param removeValue The value of the inventory removed.
    \param removeFees The transaction fees to remove the inventory.
    \param expectedCostBasis The expected cost basis.
    \param expectedEarnings The expected gross earnings.
  */
  template<typename BookkeeperType>
  void TestAddRemoveHelper(Quantity addQuantity, Money addValue, Money addFees,
      Quantity removeQuantity, Money removeValue, Money removeFees,
      Money expectedCostBasis, Money expectedEarnings) {
    auto bookkeeper = BookkeeperType();

    // Execute the two transactions.
    bookkeeper.RecordTransaction("Coke", DefaultCurrencies::USD(), addQuantity,
      addValue, addFees);
    bookkeeper.RecordTransaction("Coke", DefaultCurrencies::USD(),
      removeQuantity, removeValue, removeFees);

    // Pull out the inventory and run tests on it.
    auto cokeEntry = bookkeeper.GetInventory("Coke", DefaultCurrencies::USD());
    REQUIRE(cokeEntry.m_position.m_costBasis == expectedCostBasis);
    REQUIRE(cokeEntry.m_fees == addFees + removeFees);
    REQUIRE(cokeEntry.m_grossProfitAndLoss == expectedEarnings);
    REQUIRE(cokeEntry.m_position.m_quantity == addQuantity + removeQuantity);

    // Do it in reverse order.
    auto reverseBookkeeper = BookkeeperType();
    reverseBookkeeper.RecordTransaction("Coke", DefaultCurrencies::USD(),
      removeQuantity, removeValue, removeFees);
    reverseBookkeeper.RecordTransaction("Coke", DefaultCurrencies::USD(),
      addQuantity, addValue, addFees);
    auto reverseCokeEntry = reverseBookkeeper.GetInventory("Coke",
      DefaultCurrencies::USD());
    REQUIRE(reverseCokeEntry.m_position.m_costBasis == expectedCostBasis);
    REQUIRE(reverseCokeEntry.m_fees == addFees + removeFees);
    REQUIRE(reverseCokeEntry.m_grossProfitAndLoss == expectedEarnings);
    REQUIRE(reverseCokeEntry.m_position.m_quantity ==
      addQuantity + removeQuantity);
  }
}

#endif
