#ifndef NEXUS_ASXTFEEHANDLINGTESTER_HPP
#define NEXUS_ASXTFEEHANDLINGTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class AsxtFeeHandlingTester
      \brief Tests fee handling for ASXT.
   */
  class AsxtFeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a fee calculation with a quantity of 0.
      void TestZeroQuantity();

      //! Tests a calculation with a 0 GST rate.
      void TestNoGst();

    private:
      CPPUNIT_TEST_SUITE(AsxtFeeHandlingTester);
        CPPUNIT_TEST(TestZeroQuantity);
        CPPUNIT_TEST(TestNoGst);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
