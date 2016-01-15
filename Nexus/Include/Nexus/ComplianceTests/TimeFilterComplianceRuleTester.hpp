#ifndef NEXUS_TIMEFILTERCOMPLIANCERULETESTER_HPP
#define NEXUS_TIMEFILTERCOMPLIANCERULETESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class TimeFilterComplianceRuleTester
      \brief Tests the TimeFilterComplianceRule class.
   */
  class TimeFilterComplianceRuleTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests adding an order during the compliance period.
      void TestAddDuringCompliancePeriod();

      //! Tests adding an order outside of the compliance period.
      void TestAddOutsideCompliancePeriod();

      //! Tests submitting an order during the compliance period.
      void TestSubmitDuringCompliancePeriod();

      //! Tests submitting an order outside of the compliance period.
      void TestSubmitOutsideCompliancePeriod();

      //! Tests canceling an order during the compliance period.
      void TestCancelDuringCompliancePeriod();

      //! Tests canceling an order outside of the compliance period.
      void TestCancelOutsideCompliancePeriod();

      //! Tests submittng an order when the start time comes after the end
      //! time.
      void TestStartTimeFollowsEndTime();

    private:
      CPPUNIT_TEST_SUITE(TimeFilterComplianceRuleTester);
        CPPUNIT_TEST(TestAddDuringCompliancePeriod);
        CPPUNIT_TEST(TestAddOutsideCompliancePeriod);
        CPPUNIT_TEST(TestSubmitDuringCompliancePeriod);
        CPPUNIT_TEST(TestSubmitOutsideCompliancePeriod);
        CPPUNIT_TEST(TestCancelDuringCompliancePeriod);
        CPPUNIT_TEST(TestCancelOutsideCompliancePeriod);
        CPPUNIT_TEST(TestStartTimeFollowsEndTime);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
