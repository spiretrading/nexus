#ifndef NEXUS_MATNFEEHANDLINGTESTER_HPP
#define NEXUS_MATNFEEHANDLINGTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class MatnFeeHandlingTester
      \brief Tests fee handling for MATN.
   */
  class MatnFeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests fee calculations across the entire fee table.
      void TestFeeTableCalculations();

      //! Tests a fee calculation with a quantity of 0.
      void TestZeroQuantity();

      //! Tests an active trade for over five dollars.
      void TestActiveDefaultTrade();

      //! Tests a passive trade for over five dollars.
      void TestPassiveDefaultTrade();

      //! Tests an active trade for one dollar.
      void TestActiveDollarTrade();

      //! Tests a passive trade for one dollar.
      void TestPassiveDollarTrade();

      //! Tests an active trade for under one dollar.
      void TestActiveSubDollarTrade();

      //! Tests a passive trade for under one dollar.
      void TestPassiveSubDollarTrade();

      //! Tests an active ETF trade.
      void TestActiveEtfTrade();

      //! Tests a passive ETF trade.
      void TestPassiveEtfTrade();

      //! Tests an active odd-lot trade.
      void TestActiveOddLotTrade();

      //! Tests a passive odd-lot trade.
      void TestPassiveOddLotTrade();

      //! Tests an active odd-lot ETF.
      void TestActiveOddLotEtf();

      //! Tests a passive odd-lot ETF.
      void TestPassiveOddLotEtf();

      //! Tests calculating fees with an unknown liquidity flag.
      void TestUnknownLiquidityFlag();

      //! Tests calculating fees with an empty liquidity flag.
      void TestEmptyLiquidityFlag();

    private:
      CPPUNIT_TEST_SUITE(MatnFeeHandlingTester);
        CPPUNIT_TEST(TestFeeTableCalculations);
        CPPUNIT_TEST(TestZeroQuantity);
        CPPUNIT_TEST(TestActiveDefaultTrade);
        CPPUNIT_TEST(TestPassiveDefaultTrade);
        CPPUNIT_TEST(TestActiveDollarTrade);
        CPPUNIT_TEST(TestPassiveDollarTrade);
        CPPUNIT_TEST(TestActiveSubDollarTrade);
        CPPUNIT_TEST(TestPassiveSubDollarTrade);
        CPPUNIT_TEST(TestActiveEtfTrade);
        CPPUNIT_TEST(TestPassiveEtfTrade);
        CPPUNIT_TEST(TestActiveOddLotTrade);
        CPPUNIT_TEST(TestPassiveOddLotTrade);
        CPPUNIT_TEST(TestActiveOddLotEtf);
        CPPUNIT_TEST(TestPassiveOddLotEtf);
        CPPUNIT_TEST(TestUnknownLiquidityFlag);
        CPPUNIT_TEST(TestEmptyLiquidityFlag);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
