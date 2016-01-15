#ifndef NEXUS_SECURITYPARSERTESTER_HPP
#define NEXUS_SECURITYPARSERTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/ParsersTests/ParserTests.hpp"

namespace Nexus {
namespace Tests {

  /*! \class SecurityParserTester
      \brief Tests the SecurityParserTracker class.
   */
  class SecurityParserTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests parsing well formed Securities.
      void TestWellFormedSecurity();

      //! Tests that parsed Securities are upper-cased.
      void TestUpperCaseSecurity();

    private:
      CPPUNIT_TEST_SUITE(SecurityParserTester);
        CPPUNIT_TEST(TestWellFormedSecurity);
        CPPUNIT_TEST(TestUpperCaseSecurity);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
