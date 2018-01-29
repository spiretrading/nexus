#ifndef NEXUS_NEX_FEE_HANDLING_TESTER_HPP
#define NEXUS_NEX_FEE_HANDLING_TESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class NexFeeHandlingTester
      \brief Tests fee handling for NEX.
   */
  class NexFeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a fee calculation with a quantity of 0.
      void TestZeroQuantity();

      //! Tests a non-zero fee calculation.
      void TestExecution();

    private:
      CPPUNIT_TEST_SUITE(NexFeeHandlingTester);
        CPPUNIT_TEST(TestZeroQuantity);
        CPPUNIT_TEST(TestExecution);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
