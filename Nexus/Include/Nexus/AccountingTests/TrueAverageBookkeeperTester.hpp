#ifndef NEXUS_TRUEAVERAGEBOOKKEEPERTESTER_HPP
#define NEXUS_TRUEAVERAGEBOOKKEEPERTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/AccountingTests/AccountingTests.hpp"

namespace Nexus {
namespace Accounting {
namespace Tests {

  /** \class TrueAverageBookkeeperTester
      \brief Tests the TrueAverageBookkeeper class.
   */
  class TrueAverageBookkeeperTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of Position to record.
      typedef Nexus::Accounting::Position<std::string> Position;

      //! The type of Inventory to use.
      typedef Nexus::Accounting::Inventory<Position> Inventory;

      //! The type of TrueAverageBookkeeper used for testing.
      typedef Nexus::Accounting::TrueAverageBookkeeper<Inventory>
        TrueAverageBookkeeper;

      //! Test adding via a transaction without any fees.
      void TestAddTransactionNoFees();

      //! Test adding via a transaction with any fees.
      void TestAddTransactionWithFees();

      //! Test adding and removing at no loss/gain.
      void TestAddRemoveTransactionAtFlat();

      //! Test adding and removing at a profit.
      void TestAddRemoveTransactionAtProfit();

      //! Test adding and removing at a loss.
      void TestAddRemoveTransactionAtLoss();

      //! Test adding two and removing one at no loss/gain.
      void TestAddTwoRemoveOneTransactionAtFlat();

      //! Test adding two and removing one at a profit.
      void TestAddTwoRemoveOneTransactionAtProfit();

      //! Test adding two and removing one at a loss.
      void TestAddTwoRemoveOneTransactionAtLoss();

      //! Test adding one and removing two at no loss/gain.
      void TestAddOneRemoveTwoTransactionAtFlat();

      //! Test adding one and removing two at a profit.
      void TestAddOneRemoveTwoTransactionAtProfit();

      //! Test adding one and removing two at a loss.
      void TestAddOneRemoveTwoTransactionAtLoss();

    private:
      CPPUNIT_TEST_SUITE(TrueAverageBookkeeperTester);
        CPPUNIT_TEST(TestAddTransactionNoFees);
        CPPUNIT_TEST(TestAddTransactionWithFees);
        CPPUNIT_TEST(TestAddRemoveTransactionAtFlat);
        CPPUNIT_TEST(TestAddRemoveTransactionAtProfit);
        CPPUNIT_TEST(TestAddRemoveTransactionAtLoss);
        CPPUNIT_TEST(TestAddTwoRemoveOneTransactionAtFlat);
        CPPUNIT_TEST(TestAddTwoRemoveOneTransactionAtProfit);
        CPPUNIT_TEST(TestAddTwoRemoveOneTransactionAtLoss);
        CPPUNIT_TEST(TestAddOneRemoveTwoTransactionAtFlat);
        CPPUNIT_TEST(TestAddOneRemoveTwoTransactionAtProfit);
        CPPUNIT_TEST(TestAddOneRemoveTwoTransactionAtLoss);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
