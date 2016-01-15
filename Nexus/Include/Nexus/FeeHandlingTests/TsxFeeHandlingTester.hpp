#ifndef NEXUS_TSXFEEHANDLINGTESTER_HPP
#define NEXUS_TSXFEEHANDLINGTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class TsxFeeHandlingTester
      \brief Tests fee handling for TSX.
   */
  class TsxFeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests fee calculations across the entire fee table.
      void TestFeeTableCalculations();

      //! Tests a fee calculation with a quantity of 0.
      void TestZeroQuantity();

      //! Tests an active sub-dollar interlisted calculation.
      void TestActiveInterlistedSubDollar();

      //! Tests a passive sub-dollar interlisted calculation.
      void TestPassiveInterlistedSubDollar();

    private:
      CPPUNIT_TEST_SUITE(TsxFeeHandlingTester);
        CPPUNIT_TEST(TestFeeTableCalculations);
        CPPUNIT_TEST(TestZeroQuantity);
        CPPUNIT_TEST(TestActiveInterlistedSubDollar);
        CPPUNIT_TEST(TestPassiveInterlistedSubDollar);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
