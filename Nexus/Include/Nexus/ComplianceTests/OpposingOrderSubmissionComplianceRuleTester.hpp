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

    private:
      CPPUNIT_TEST_SUITE(OpposingOrderSubmissionComplianceRuleTester);
        CPPUNIT_TEST(TestLimitAskCancel);
        CPPUNIT_TEST(TestLimitBidCancel);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
