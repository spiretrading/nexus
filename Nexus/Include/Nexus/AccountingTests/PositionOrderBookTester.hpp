#ifndef NEXUS_POSITIONORDERBOOKTESTER_HPP
#define NEXUS_POSITIONORDERBOOKTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/AccountingTests/AccountingTests.hpp"

namespace Nexus {
namespace Accounting {
namespace Tests {

  /*  \class PositionOrderBookTester
      \brief Tests the PositionOrderBook class.
   */
  class PositionOrderBookTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Test a bid for 100 shares.
      //! Expect the bid to count as an opening order submission.
      //! Add the bid to the order book.
      //! Fill the bid for 100 shares.
      //! Test a bid for 100 shares.
      //! Expect the bid to count as an opening order submission.
      //! Test an ask for 100 shares.
      //! Expect the ask NOT to count as an opening order submission.
      //! Add the ask to the order book.
      //! Test an ask for 100 shares.
      //! Expect the ask to count as an opening order submission.
      //! Fill the ask for 100 shares.
      //! Test a bid for 100 shares.
      //! Expect the bid to count as an opening order submission.
      //! Test an ask for 100 shares.
      //! Expect the ask to count as an opening order submission.
      void TestBidOpeningOrderSubmission();

      //! Perform the above test but flipping bids with asks.
      void TestAskOpeningOrderSubmission();

      //! Submit ask A for 100 shares.
      //! Submit ask B for 100 shares.
      //! Fill A for 100 shares.
      //! Cancel B.
      //! Submit bid C for 200 shares.
      //! Fill C for 200 shares.
      //! Test ask D for 200 shares.
      //! Expect ask D to count as an opening order.
      void TestRemovingOrders();

      //! Tests retrieving the list of opening orders.
      void TestGetOpeningOrders();

    private:
      CPPUNIT_TEST_SUITE(PositionOrderBookTester);
        CPPUNIT_TEST(TestBidOpeningOrderSubmission);
        CPPUNIT_TEST(TestAskOpeningOrderSubmission);
        CPPUNIT_TEST(TestRemovingOrders);
        CPPUNIT_TEST(TestGetOpeningOrders);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
