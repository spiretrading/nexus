#ifndef NEXUS_OMGAFEEHANDLINGTESTER_HPP
#define NEXUS_OMGAFEEHANDLINGTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class OmgaFeeHandlingTester
      \brief Tests fee handling for OMGA.
   */
  class OmgaFeeHandlingTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests fee calculations across the entire fee table.
      void TestFeeTableCalculations();

      //! Tests a fee calculation with a quantity of 0.
      void TestZeroQuantity();

      //! Tests a default active execution.
      void TestDefaultActive();

      //! Tests a default passive execution.
      void TestDefaultPassive();

      //! Tests a hidden passive execution.
      void TestDefaultHiddenPassive();

      //! Tests a hidden active execution.
      void TestDefaultHiddenActive();

      //! Tests an ETF active execution.
      void TestEtfActive();

      //! Tests an ETF passive execution.
      void TestEtfPassive();

      //! Tests odd-lot active execution.
      void TestOddLotActive();

      //! Tests odd-lot passive execution.
      void TestOddLotPassive();

      //! Tests a sub-dollar active execution.
      void TestSubDollarActive();

      //! Tests a sub-dollar passive execution.
      void TestSubDollarPassive();

      //! Tests a sub-dollar hidden passive execution.
      void TestSubDollarHiddenPassive();

      //! Tests a sub-dollar hidden active execution.
      void TestSubDollarHiddenActive();

      //! Tests a sub-dollar ETF active execution.
      void TestSubDollarEtfActive();

      //! Tests a sub-dollar ETF passive execution.
      void TestSubDollarEtfPassive();

      //! Tests a sub-dollar odd-lot active execution.
      void TestSubDollarOddLotActive();

      //! Tests a sub-dollar odd-lot passive execution.
      void TestSubDollarOddLotPassive();

      //! Tests an active hidden ETF.
      void TestHiddenEtfActive();

      //! Tests a passive hidden ETF.
      void TestHiddenEtfPassive();

      //! Tests an active odd lot ETF.
      void TestOddLotEtfActive();

      //! Tests a passive odd lot ETF.
      void TestOddLotEtfPassive();

      //! Tests an active odd lot hidden ETF.
      void TestOddLotHiddenEtfActive();

      //! Tests a passive odd lot hidden ETF.
      void TestOddLotHiddenEtfPassive();

      //! Tests calculating fees with an unknown liquidity flag.
      void TestUnknownLiquidityFlag();

      //! Tests calculating fees with an empty liquidity flag.
      void TestEmptyLiquidityFlag();

    private:
      CPPUNIT_TEST_SUITE(OmgaFeeHandlingTester);
        CPPUNIT_TEST(TestFeeTableCalculations);
        CPPUNIT_TEST(TestZeroQuantity);
        CPPUNIT_TEST(TestDefaultActive);
        CPPUNIT_TEST(TestDefaultPassive);
        CPPUNIT_TEST(TestDefaultHiddenPassive);
        CPPUNIT_TEST(TestDefaultHiddenActive);
        CPPUNIT_TEST(TestEtfActive);
        CPPUNIT_TEST(TestEtfPassive);
        CPPUNIT_TEST(TestOddLotActive);
        CPPUNIT_TEST(TestOddLotPassive);
        CPPUNIT_TEST(TestSubDollarActive);
        CPPUNIT_TEST(TestSubDollarPassive);
        CPPUNIT_TEST(TestSubDollarHiddenPassive);
        CPPUNIT_TEST(TestSubDollarHiddenActive);
        CPPUNIT_TEST(TestSubDollarEtfActive);
        CPPUNIT_TEST(TestSubDollarEtfPassive);
        CPPUNIT_TEST(TestSubDollarOddLotActive);
        CPPUNIT_TEST(TestSubDollarOddLotPassive);
        CPPUNIT_TEST(TestHiddenEtfActive);
        CPPUNIT_TEST(TestHiddenEtfPassive);
        CPPUNIT_TEST(TestOddLotEtfActive);
        CPPUNIT_TEST(TestOddLotEtfPassive);
        CPPUNIT_TEST(TestOddLotHiddenEtfActive);
        CPPUNIT_TEST(TestOddLotHiddenEtfPassive);
        CPPUNIT_TEST(TestUnknownLiquidityFlag);
        CPPUNIT_TEST(TestEmptyLiquidityFlag);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
