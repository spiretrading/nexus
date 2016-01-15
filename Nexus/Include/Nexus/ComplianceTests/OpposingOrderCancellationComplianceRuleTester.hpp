#ifndef NEXUS_OPPOSINGORDERCANCELLATIONCOMPLIANCERULETESTER_HPP
#define NEXUS_OPPOSINGORDERCANCELLATIONCOMPLIANCERULETESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class OpposingOrderCancellationComplianceRuleTester
      \brief Tests the OpposingOrderCancellationComplianceRule class.
   */
  class OpposingOrderCancellationComplianceRuleTester :
      public CPPUNIT_NS::TestFixture {
    public:

      //! Submit an ask.
      //! Cancel the ask.
      //! Submit a bid.
      //! Cancel the bid.
      void TestCancelWithoutFill();

      //! Submit an ask.
      //! Submit a bid.
      //! Cancel the bid.
      //! Cancel the ask.
      void TestInOrderCancelOpposingOrderWithoutFill();

      //! Submit an ask.
      //! Submit a bid.
      //! Cancel the ask.
      //! Cancel the bid.
      void TestReverseOrderCancelOpposingOrderWithoutFill();

      //! Submit an ask.
      //! Submit a bid.
      //! Partially fill the ask.
      //! Cancel the bid, expect a rejection.
      //! Cancel the ask.
      //! Set time beyond the timeout period.
      //! Cancel the bid.
      void TestInOrderCancelOpposingOrderWithFillInsidePeriod();

      //! Submit an ask.
      //! Submit a bid.
      //! Partially fill the ask.
      //! Cancel the ask.
      //! Cancel the bid, expect a rejection.
      //! Set time beyond the timeout period.
      //! Cancel the bid.
      void TestReverseOrderCancelOpposingOrderWithFillInsidePeriod();

      //! Submit an ask.
      //! Submit a bid.
      //! Partially fill the ask.
      //! Set time beyond the timeout period.
      //! Cancel the bid.
      //! Cancel the ask.
      void TestInOrderCancelOpposingOrderWithFillOutsidePeriod();

      //! Submit an ask.
      //! Submit a bid.
      //! Partially fill the ask.
      //! Cancel the ask.
      //! Set time beyond the timeout period.
      //! Cancel the bid.
      void TestReverseOrderCancelOpposingOrderWithFillOutsidePeriod();

    private:
      CPPUNIT_TEST_SUITE(OpposingOrderCancellationComplianceRuleTester);
        CPPUNIT_TEST(TestCancelWithoutFill);
        CPPUNIT_TEST(TestInOrderCancelOpposingOrderWithoutFill);
        CPPUNIT_TEST(TestReverseOrderCancelOpposingOrderWithoutFill);
        CPPUNIT_TEST(TestInOrderCancelOpposingOrderWithFillInsidePeriod);
        CPPUNIT_TEST(TestReverseOrderCancelOpposingOrderWithFillInsidePeriod);
        CPPUNIT_TEST(TestInOrderCancelOpposingOrderWithFillOutsidePeriod);
        CPPUNIT_TEST(TestReverseOrderCancelOpposingOrderWithFillOutsidePeriod);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
