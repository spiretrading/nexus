#ifndef NEXUS_NYSEFEEHANDLINGTESTER_HPP
#define NEXUS_NYSEFEEHANDLINGTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class NyseFeeHandlingTester
      \brief Tests fee handling for NYSE.
   */
  class NyseFeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a fee calculation with a quantity of 0.
      void TestZeroQuantity();

    private:
      CPPUNIT_TEST_SUITE(NyseFeeHandlingTester);
        CPPUNIT_TEST(TestZeroQuantity);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
