#ifndef NEXUS_PUREFEEHANDLINGTESTER_HPP
#define NEXUS_PUREFEEHANDLINGTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class PureFeeHandlingTester
      \brief Tests fee handling for PURE.
   */
  class PureFeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests fee calculations across the entire fee table.
      void TestFeeTableCalculations();

      //! Tests a fee calculation with a quantity of 0.
      void TestZeroQuantity();

      //! Tests a TSX default active fee calculation.
      void TestTsxDefaultActive();

      //! Tests a TSX default passive fee calculation.
      void TestTsxDefaultPassive();

      //! Tests a TSX sub-dollar active fee calculation.
      void TestTsxSubDollarActive();

      //! Tests a TSX sub-dollar passive fee calculation.
      void TestTsxSubDollarPassive();

      //! Tests a TSX Venture default active fee calculation.
      void TestTsxVentureDefaultActive();

      //! Tests a TSX Venture default passive fee calculation.
      void TestTsxVentureDefaultPassive();

      //! Tests a TSX Venture sub-dollar active fee calculation.
      void TestTsxVentureSubDollarActive();

      //! Tests a TSX Venture sub-dollar passive fee calculation.
      void TestTsxVentureSubDollarPassive();

      //! Tests a TSX Venture sub-dime active fee calculation.
      void TestTsxVentureSubDimeActive();

      //! Tests a TSX Venture sub-dime passive fee calculation.
      void TestTsxVentureSubDimePassive();

      //! Tests an odd-lot fee calculation.
      void TestOddLot();

      //! Tests calculating fees with an unknown liquidity flag.
      void TestUnknownLiquidityFlag();

      //! Tests calculating fees with an empty liquidity flag.
      void TestEmptyLiquidityFlag();

    private:
      CPPUNIT_TEST_SUITE(PureFeeHandlingTester);
        CPPUNIT_TEST(TestFeeTableCalculations);
        CPPUNIT_TEST(TestZeroQuantity);
        CPPUNIT_TEST(TestTsxDefaultActive);
        CPPUNIT_TEST(TestTsxDefaultPassive);
        CPPUNIT_TEST(TestTsxSubDollarActive);
        CPPUNIT_TEST(TestTsxSubDollarPassive);
        CPPUNIT_TEST(TestTsxVentureDefaultActive);
        CPPUNIT_TEST(TestTsxVentureDefaultPassive);
        CPPUNIT_TEST(TestTsxVentureSubDollarActive);
        CPPUNIT_TEST(TestTsxVentureSubDollarPassive);
        CPPUNIT_TEST(TestTsxVentureSubDimeActive);
        CPPUNIT_TEST(TestTsxVentureSubDimePassive);
        CPPUNIT_TEST(TestOddLot);
        CPPUNIT_TEST(TestUnknownLiquidityFlag);
        CPPUNIT_TEST(TestEmptyLiquidityFlag);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
