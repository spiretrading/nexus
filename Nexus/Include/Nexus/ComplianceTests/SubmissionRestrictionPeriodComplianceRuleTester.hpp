#ifndef NEXUS_SUBMISSIONRESTRICTIONPERIODCOMPLIANCERULETESTER_HPP
#define NEXUS_SUBMISSIONRESTRICTIONPERIODCOMPLIANCERULETESTER_HPP
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/Compliance/SubmissionRestrictionPeriodComplianceRule.hpp"
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class SubmissionRestrictionPeriodComplianceRuleTester
      \brief Tests the SubmissionRestrictionPeriodComplianceRule class.
   */
  class SubmissionRestrictionPeriodComplianceRuleTester :
      public CPPUNIT_NS::TestFixture {
    public:

      //! The type of TimeClient used for tests.
      using TimeClient = Beam::TimeService::IncrementalTimeClient;

      //! The type of compliance rule to test.
      using TestSubmissionRestrictionComplianceRule =
        SubmissionRestrictionPeriodComplianceRule<TimeClient*>;

      //! Construct the rule with a period from 00:00:01 to 00:00:00.
      //! Submit an order with a timestamp of 00:00:00, the submission should be
      //! valid.
      void TestEmptyPeriod();

    private:
      CPPUNIT_TEST_SUITE(SubmissionRestrictionPeriodComplianceRuleTester);
        CPPUNIT_TEST(TestEmptyPeriod);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
