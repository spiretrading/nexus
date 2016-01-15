#ifndef NEXUS_SECURITYFILTERCOMPLIANCERULETESTER_HPP
#define NEXUS_SECURITYFILTERCOMPLIANCERULETESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/ComplianceTests/ComplianceTests.hpp"

namespace Nexus {
namespace Compliance {
namespace Tests {

  /*! \class SecurityFilterComplianceRuleTester
      \brief Tests the SecurityFilterComplianceRule class.
   */
  class SecurityFilterComplianceRuleTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests adding an Order whose Security matches.
      void TestMatchingAdd();

      //! Tests adding an Order whose Security doesn't match.
      void TestUnmatchingAdd();

      //! Tests submitting an Order whose Security matches.
      void TestMatchingSubmit();

      //! Tests submitting an Order whose Security doesn't match.
      void TestUnmatchingSubmit();

      //! Tests canceling an Order whose Security matches.
      void TestMatchingCancel();

      //! Tests canceling an Order whose Security doesn't match.
      void TestUnmatchingCancel();

    private:
      CPPUNIT_TEST_SUITE(SecurityFilterComplianceRuleTester);
        CPPUNIT_TEST(TestMatchingAdd);
        CPPUNIT_TEST(TestUnmatchingAdd);
        CPPUNIT_TEST(TestMatchingSubmit);
        CPPUNIT_TEST(TestUnmatchingSubmit);
        CPPUNIT_TEST(TestMatchingCancel);
        CPPUNIT_TEST(TestUnmatchingCancel);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
