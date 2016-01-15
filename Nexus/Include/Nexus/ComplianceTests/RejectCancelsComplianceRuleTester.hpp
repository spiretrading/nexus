#ifndef NEXUS_REJECTCANCELSCOMPLIANCERULETESTER_HPP
#define NEXUS_REJECTCANCELSCOMPLIANCERULETESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class RejectCancelsComplianceRuleTester
      \brief Tests the RejectCancelsComplianceRule class.
   */
  class RejectCancelsComplianceRuleTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests adding an order.
      void TestAdd();

      //! Tests submitting an order.
      void TestSubmit();

      //! Tests canceling an order.
      void TestCancel();

    private:
      CPPUNIT_TEST_SUITE(RejectCancelsComplianceRuleTester);
        CPPUNIT_TEST(TestAdd);
        CPPUNIT_TEST(TestSubmit);
        CPPUNIT_TEST(TestCancel);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
