#ifndef NEXUS_NSDQFEEHANDLINGTESTER_HPP
#define NEXUS_NSDQFEEHANDLINGTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class NsdqFeeHandlingTester
      \brief Tests fee handling for NSDQ.
   */
  class NsdqFeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a fee calculation with a quantity of 0.
      void TestZeroQuantity();

    private:
      CPPUNIT_TEST_SUITE(NsdqFeeHandlingTester);
        CPPUNIT_TEST(TestZeroQuantity);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
