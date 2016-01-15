#ifndef NEXUS_REJECTSUBMISSIONSCOMPLIANCERULETESTER_HPP
#define NEXUS_REJECTSUBMISSIONSCOMPLIANCERULETESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class RejectSubmissionsComplianceRuleTester
      \brief Tests the RejectSubmissionsComplianceRule class.
   */
  class RejectSubmissionsComplianceRuleTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests adding an order.
      void TestAdd();

      //! Tests submitting an order.
      void TestSubmit();

      //! Tests canceling an order.
      void TestCancel();

    private:
      CPPUNIT_TEST_SUITE(RejectSubmissionsComplianceRuleTester);
        CPPUNIT_TEST(TestAdd);
        CPPUNIT_TEST(TestSubmit);
        CPPUNIT_TEST(TestCancel);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
