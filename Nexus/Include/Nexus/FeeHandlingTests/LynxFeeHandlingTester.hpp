#ifndef NEXUS_LYNXFEEHANDLINGTESTER_HPP
#define NEXUS_LYNXFEEHANDLINGTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class LynxFeeHandlingTester
      \brief Tests fee handling for LYNX.
   */
  class LynxFeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests fee calculations across the entire fee table.
      void TestFeeTableCalculations();

      //! Tests a fee calculation with a quantity of 0.
      void TestZeroQuantity();

      //! Tests active fee calculations on all liquidity levels.
      void TestActive();

      //! Tests passive fee calculations on all liquidity levels.
      void TestPassive();

      //! Tests sub-dollar active fee calculations on all liquidity levels.
      void TestSubDollarActive();

      //! Tests sub-dollar passive fee calculations on all liquidity levels.
      void TestSubDollarPassive();

      //! Tests odd-lot active fee calculations on all liquidity levels.
      void TestOddLotActive();

      //! Tests odd-lot passive fee calculations on all liquidity levels.
      void TestOddLotPassive();

      //! Tests sub-dollar odd-lot active fee calculations on all liquidity
      //! levels.
      void TestSubDollarOddLotActive();

      //! Tests sub-dollar odd-lot passive fee calculations on all liquidity
      //! levels.
      void TestSubDollarOddLotPassive();

      //! Tests calculating fees with an unknown liquidity flag.
      void TestUnknownLiquidityFlag();

      //! Tests calculating fees with an empty liquidity flag.
      void TestEmptyLiquidityFlag();

    private:
      CPPUNIT_TEST_SUITE(LynxFeeHandlingTester);
        CPPUNIT_TEST(TestFeeTableCalculations);
        CPPUNIT_TEST(TestZeroQuantity);
        CPPUNIT_TEST(TestActive);
        CPPUNIT_TEST(TestPassive);
        CPPUNIT_TEST(TestSubDollarActive);
        CPPUNIT_TEST(TestSubDollarPassive);
        CPPUNIT_TEST(TestOddLotActive);
        CPPUNIT_TEST(TestOddLotPassive);
        CPPUNIT_TEST(TestSubDollarOddLotActive);
        CPPUNIT_TEST(TestSubDollarOddLotPassive);
        CPPUNIT_TEST(TestUnknownLiquidityFlag);
        CPPUNIT_TEST(TestEmptyLiquidityFlag);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
