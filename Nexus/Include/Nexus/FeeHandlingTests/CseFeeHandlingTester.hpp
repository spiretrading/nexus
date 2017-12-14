#ifndef NEXUS_CSE_FEE_HANDLING_TESTER_HPP
#define NEXUS_CSE_FEE_HANDLING_TESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class CseFeeHandlingTester
      \brief Tests fee handling for CSE.
   */
  class CseFeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests fee calculations across the entire fee table.
      void TestFeeTableCalculations();

      //! Tests a fee calculation with a quantity of 0.
      void TestZeroQuantity();

      //! Tests a default active fee calculation.
      void TestDefaultActive();

      //! Tests a default passive fee calculation.
      void TestDefaultPassive();

      //! Tests a sub-dollar active fee calculation.
      void TestSubDollarActive();

      //! Tests a sub-dollar passive fee calculation.
      void TestSubDollarPassive();

      //! Tests a sub-dime active fee calculation.
      void TestSubDimeActive();

      //! Tests a sub-dime passive fee calculation.
      void TestSubDimePassive();

      //! Tests calculating fees with an unknown liquidity flag.
      void TestUnknownLiquidityFlag();

      //! Tests calculating fees with an empty liquidity flag.
      void TestEmptyLiquidityFlag();

    private:
      CPPUNIT_TEST_SUITE(CseFeeHandlingTester);
        CPPUNIT_TEST(TestFeeTableCalculations);
        CPPUNIT_TEST(TestZeroQuantity);
        CPPUNIT_TEST(TestDefaultActive);
        CPPUNIT_TEST(TestDefaultPassive);
        CPPUNIT_TEST(TestSubDollarActive);
        CPPUNIT_TEST(TestSubDollarPassive);
        CPPUNIT_TEST(TestSubDimeActive);
        CPPUNIT_TEST(TestSubDimePassive);
        CPPUNIT_TEST(TestUnknownLiquidityFlag);
        CPPUNIT_TEST(TestEmptyLiquidityFlag);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
