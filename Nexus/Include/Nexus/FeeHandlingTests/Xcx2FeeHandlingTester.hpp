#ifndef NEXUS_XCX2FEEHANDLINGTESTER_HPP
#define NEXUS_XCX2FEEHANDLINGTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class Xcx2FeeHandlingTester
      \brief Tests fee handling for XCX2.
   */
  class Xcx2FeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests fee calculations across the entire fee table.
      void TestFeeTableCalculations();

      //! Tests calculating fees with a quantity of 0.
      void TestZeroQuantity();

      //! Tests an active fee calculation.
      void TestActiveDefault();

      //! Tests a passive fee calculation.
      void TestPassiveDefault();

      //! Tests a large passive fee calculation.
      void TestLargeActiveDefault();

      //! Tests a large passive fee calculation.
      void TestLargePassiveDefault();

      //! Tests a hidden active fee calculation.
      void TestHiddenActiveOverDollar();

      //! Tests a hidden passive fee calculation.
      void TestHiddenPassiveOverDollar();

      //! Tests an active odd-lot.
      void TestActiveOddLotOverDollar();

      //! Tests a passive odd-lot.
      void TestPassiveOddLotOverDollar();

      //! Tests a sub-dollar active fee calculation.
      void TestActiveSubDollar();

      //! Tests a sub-dollar passive fee calculation.
      void TestPassiveSubDollar();

      //! Tests a sub-dime active fee calculation.
      void TestActiveSubDime();

      //! Tests a sub-dime passive fee calculation.
      void TestPassiveSubDime();

      //! Tests a sub-dollar large passive fee calculation.
      void TestLargeActiveSubDollar();

      //! Tests a sub-dollar large passive fee calculation.
      void TestLargePassiveSubDollar();

      //! Tests a sub-dime large passive fee calculation.
      void TestLargeActiveSubDime();

      //! Tests a sub-dime large passive fee calculation.
      void TestLargePassiveSubDime();

      //! Tests a sub-dollar hidden active fee calculation.
      void TestHiddenActiveSubDollar();

      //! Tests a sub-dollar hidden passive fee calculation.
      void TestHiddenPassiveSubDollar();

      //! Tests a sub-dime hidden active fee calculation.
      void TestHiddenActiveSubDime();

      //! Tests a sub-dime hidden passive fee calculation.
      void TestHiddenPassiveSubDime();

      //! Tests an active sub-dollar odd-lot.
      void TestActiveOddLotSubDollar();

      //! Tests a passive sub-dollar odd-lot.
      void TestPassiveOddLotSubDollar();

      //! Tests an active sub-dime odd-lot.
      void TestActiveOddLotSubDime();

      //! Tests a passive sub-dime odd-lot.
      void TestPassiveOddLotSubDime();

      //! Tests a hidden odd-lot.
      void TestHiddenOddLot();

      //! Tests calculating fees with an unknown liquidity flag.
      void TestUnknownLiquidityFlag();

      //! Tests calculating fees with an empty liquidity flag.
      void TestEmptyLiquidityFlag();

    private:
      CPPUNIT_TEST_SUITE(Xcx2FeeHandlingTester);
        CPPUNIT_TEST(TestFeeTableCalculations);
        CPPUNIT_TEST(TestZeroQuantity);
        CPPUNIT_TEST(TestActiveDefault);
        CPPUNIT_TEST(TestPassiveDefault);
        CPPUNIT_TEST(TestLargeActiveDefault);
        CPPUNIT_TEST(TestLargePassiveDefault);
        CPPUNIT_TEST(TestHiddenActiveOverDollar);
        CPPUNIT_TEST(TestHiddenPassiveOverDollar);
        CPPUNIT_TEST(TestActiveOddLotOverDollar);
        CPPUNIT_TEST(TestPassiveOddLotOverDollar);
        CPPUNIT_TEST(TestActiveSubDollar);
        CPPUNIT_TEST(TestPassiveSubDollar);
        CPPUNIT_TEST(TestActiveSubDime);
        CPPUNIT_TEST(TestPassiveSubDime);
        CPPUNIT_TEST(TestLargeActiveSubDollar);
        CPPUNIT_TEST(TestLargePassiveSubDollar);
        CPPUNIT_TEST(TestLargeActiveSubDime);
        CPPUNIT_TEST(TestLargePassiveSubDime);
        CPPUNIT_TEST(TestHiddenActiveSubDollar);
        CPPUNIT_TEST(TestHiddenPassiveSubDollar);
        CPPUNIT_TEST(TestHiddenActiveSubDime);
        CPPUNIT_TEST(TestHiddenPassiveSubDime);
        CPPUNIT_TEST(TestActiveOddLotSubDollar);
        CPPUNIT_TEST(TestPassiveOddLotSubDollar);
        CPPUNIT_TEST(TestActiveOddLotSubDime);
        CPPUNIT_TEST(TestPassiveOddLotSubDime);
        CPPUNIT_TEST(TestHiddenOddLot);
        CPPUNIT_TEST(TestUnknownLiquidityFlag);
        CPPUNIT_TEST(TestEmptyLiquidityFlag);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
