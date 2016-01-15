#ifndef NEXUS_CANCELRESTRICTIONPERIODCOMPLIANCERULETESTER_HPP
#define NEXUS_CANCELRESTRICTIONPERIODCOMPLIANCERULETESTER_HPP
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/Compliance/CancelRestrictionPeriodComplianceRule.hpp"
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class CancelRestrictionPeriodComplianceRuleTester
      \brief Tests the CancelRestrictionPeriodComplianceRule class.
   */
  class CancelRestrictionPeriodComplianceRuleTester :
      public CPPUNIT_NS::TestFixture {
    public:

      //! The type of TimeClient used for tests.
      using TimeClient = Beam::TimeService::IncrementalTimeClient;

      //! The type of compliance rule to test.
      using TestCancelRestrictionComplianceRule =
        CancelRestrictionPeriodComplianceRule<TimeClient*>;

      //! Construct the rule with a period from 00:00:01 to 00:00:00.
      //! Cancel an order with a timestamp of 00:00:00, the cancel should be
      //! valid.
      void TestEmptyPeriod();

    private:
      CPPUNIT_TEST_SUITE(CancelRestrictionPeriodComplianceRuleTester);
        CPPUNIT_TEST(TestEmptyPeriod);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
