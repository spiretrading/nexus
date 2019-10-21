#ifndef NEXUS_HKEX_FEE_HANDLING_TESTER_HPP
#define NEXUS_HKEX_FEE_HANDLING_TESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus::Tests {

  /** Tests fee handling for HKEX. */
  class HkexFeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      /** Tests a fee calculation with a quantity of 0. */
      void TestZeroQuantity();

      /** Tests a default execution. */
      void TestDefault();

    private:
      CPPUNIT_TEST_SUITE(HkexFeeHandlingTester);
        CPPUNIT_TEST(TestZeroQuantity);
        CPPUNIT_TEST(TestDefault);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}

#endif
