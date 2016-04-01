#ifndef NEXUS_OPPOSINGORDERSUBMISSIONCOMPLIANCERULETESTER_HPP
#define NEXUS_OPPOSINGORDERSUBMISSIONCOMPLIANCERULETESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class OpposingOrderSubmissionComplianceRuleTester
      \brief Tests the OpposingOrderSubmissionComplianceRule class.
   */
  class OpposingOrderSubmissionComplianceRuleTester :
      public CPPUNIT_NS::TestFixture {
    public:

      //! Submit a limit ask.
      //! Cancel the ask.
      //! Submit a bid.
      void TestLimitAskCancel();

      //! Submit a limit bid.
      //! Cancel the bid.
      //! Submit an ask.
      void TestLimitBidCancel();

      //! Submit a limit bid.
      //! Cancel the bid.
      //! Submit a limit ask within the valid price.
      //! Submit a limit bid with a better price.
      //! Submit a limit ask, expect a rejection.
      void TestBetterPrice();

    private:
      CPPUNIT_TEST_SUITE(OpposingOrderSubmissionComplianceRuleTester);
        CPPUNIT_TEST(TestLimitAskCancel);
        CPPUNIT_TEST(TestLimitBidCancel);
        CPPUNIT_TEST(TestBetterPrice);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
