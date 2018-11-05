#ifndef NEXUS_CHICFEEHANDLINGTESTER_HPP
#define NEXUS_CHICFEEHANDLINGTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/FeeHandlingTests/FeeHandlingTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class ChicFeeHandlingTester
      \brief Tests fee handling for CHIC.
   */
  class ChicFeeHandlingTester : public CPPUNIT_NS::TestFixture {
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

      //! Tests a sub dollar active execution.
      void TestSubDollarActive();

      //! Tests a sub dollar passive execution.
      void TestSubDollarPassive();

      //! Tests a sub dime active execution.
      void TestSubDimeActive();

      //! Tests a sub dime passive execution.
      void TestSubDimePassive();

      //! Tests a sub dollar active execution.
      void TestSubDollarHiddenActive();

      //! Tests a sub dollar passive execution.
      void TestSubDollarHiddenPassive();

      //! Tests a sub dime active execution.
      void TestSubDimeHiddenActive();

      //! Tests a sub dime passive execution.
      void TestSubDimeHiddenPassive();

      //! Tests an interlisted active execution.
      void TestInterlistedActive();

      //! Tests an interlisted passive execution.
      void TestInterlistedPassive();

      //! Tests an interlisted hidden passive execution.
      void TestInterlistedHiddenPassive();

      //! Tests an interlisted hidden active execution.
      void TestInterlistedHiddenActive();

      //! Tests a sub-dollar interlisted active execution.
      void TestSubDollarInterlistedActive();

      //! Tests a sub-dime interlisted active execution.
      void TestSubDimeInterlistedActive();

      //! Tests an ETF active execution.
      void TestEtfActive();

      //! Tests an ETF passive execution.
      void TestEtfPassive();

      //! Tests an ETF hidden passive execution.
      void TestEtfHiddenPassive();

      //! Tests an ETF hidden active execution.
      void TestEtfHiddenActive();

      //! Tests a sub-dollar ETF active execution.
      void TestSubDollarEtfActive();

      //! Tests a sub-dime ETF active execution.
      void TestSubDimeEtfActive();

      //! Tests calculating fees with an unknown liquidity flag.
      void TestUnknownLiquidityFlag();

      //! Tests calculating fees with an empty liquidity flag.
      void TestEmptyLiquidityFlag();

    private:
      CPPUNIT_TEST_SUITE(ChicFeeHandlingTester);
        CPPUNIT_TEST(TestFeeTableCalculations);
        CPPUNIT_TEST(TestZeroQuantity);
        CPPUNIT_TEST(TestDefaultActive);
        CPPUNIT_TEST(TestDefaultPassive);
        CPPUNIT_TEST(TestDefaultHiddenPassive);
        CPPUNIT_TEST(TestDefaultHiddenActive);
        CPPUNIT_TEST(TestSubDollarActive);
        CPPUNIT_TEST(TestSubDollarPassive);
        CPPUNIT_TEST(TestSubDimeActive);
        CPPUNIT_TEST(TestSubDimePassive);
        CPPUNIT_TEST(TestSubDollarHiddenActive);
        CPPUNIT_TEST(TestSubDollarHiddenPassive);
        CPPUNIT_TEST(TestSubDimeHiddenActive);
        CPPUNIT_TEST(TestSubDimeHiddenPassive);
        CPPUNIT_TEST(TestInterlistedActive);
        CPPUNIT_TEST(TestInterlistedPassive);
        CPPUNIT_TEST(TestInterlistedHiddenPassive);
        CPPUNIT_TEST(TestInterlistedHiddenActive);
        CPPUNIT_TEST(TestSubDollarInterlistedActive);
        CPPUNIT_TEST(TestSubDimeInterlistedActive);
        CPPUNIT_TEST(TestEtfActive);
        CPPUNIT_TEST(TestEtfPassive);
        CPPUNIT_TEST(TestEtfHiddenPassive);
        CPPUNIT_TEST(TestEtfHiddenActive);
        CPPUNIT_TEST(TestSubDollarEtfActive);
        CPPUNIT_TEST(TestSubDimeEtfActive);
        CPPUNIT_TEST(TestUnknownLiquidityFlag);
        CPPUNIT_TEST(TestEmptyLiquidityFlag);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
