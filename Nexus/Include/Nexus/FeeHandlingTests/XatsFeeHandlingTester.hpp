#ifndef NEXUS_XATSFEEHANDLINGTESTER_HPP
#define NEXUS_XATSFEEHANDLINGTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class XatsFeeHandlingTester
      \brief Tests fee handling for XATS.
   */
  class XatsFeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests fee calculations across the entire fee table.
      void TestFeeTableCalculations();

      //! Tests a fee calculation with a quantity of 0.
      void TestZeroQuantity();

    private:
      CPPUNIT_TEST_SUITE(XatsFeeHandlingTester);
        CPPUNIT_TEST(TestFeeTableCalculations);
        CPPUNIT_TEST(TestZeroQuantity);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
