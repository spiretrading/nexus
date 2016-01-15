#ifndef NEXUS_BUYINGPOWERTRACKERTESTER_HPP
#define NEXUS_BUYINGPOWERTRACKERTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/AccountingTests/AccountingTests.hpp"

namespace Nexus {
namespace Accounting {
namespace Tests {

  /*  \class BuyingPowerTrackerTester
      \brief Tests the BuyingPowerTracker class.
   */
  class BuyingPowerTrackerTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests submitting two bids.
      void TestTwoBids();

      //! Tests submitting a bid and then an ask with less buying power.
      void TestBidAndSmallerAsk();

      //! Tests submitting a bid and then an ask with greater buying power.
      void TestBidAndGreaterAsk();

      //! Tests submitting a bid and then canceling it.
      void TestBidAndCancel();

      //! Tests submitting a bid and then filling it.
      void TestBidAndFill();

      //! Tests submitting a bid and then a partial fill and then a cancel.
      void TestBidAndPartialFillAndCancel();

      //! Tests filling a bid order, and then filling an ask for an equal
      //! amount.
      void TestBidAndOffsettingAsk();

      //! Tests filling a bid order, and then filling an ask for only a portion
      //! of the amount.
      void TestBidAndPartialOffsettingAsk();

      //! Tests partially filling a bid order, and then filling an ask for an
      //! equal amount.
      void TestPartialBidAndOffsettingAsk();

      //! Tests partially filling a bid order, and then partially filling an
      //! ask.
      void TestPartialBidAndPartialOffsettingAsk();

      //! Tests filling a bid order, and then submitting ask orders that exceed
      //! the bid quantity.
      void TestBidAndMultipleAsks();

    private:
      CPPUNIT_TEST_SUITE(BuyingPowerTrackerTester);
        CPPUNIT_TEST(TestTwoBids);
        CPPUNIT_TEST(TestBidAndSmallerAsk);
        CPPUNIT_TEST(TestBidAndGreaterAsk);
        CPPUNIT_TEST(TestBidAndCancel);
        CPPUNIT_TEST(TestBidAndFill);
        CPPUNIT_TEST(TestBidAndPartialFillAndCancel);
        CPPUNIT_TEST(TestBidAndOffsettingAsk);
        CPPUNIT_TEST(TestBidAndPartialOffsettingAsk);
        CPPUNIT_TEST(TestPartialBidAndOffsettingAsk);
        CPPUNIT_TEST(TestPartialBidAndPartialOffsettingAsk);
        CPPUNIT_TEST(TestBidAndMultipleAsks);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
