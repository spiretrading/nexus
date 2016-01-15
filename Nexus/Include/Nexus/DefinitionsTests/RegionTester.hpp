#ifndef NEXUS_REGIONTESTER_HPP
#define NEXUS_REGIONTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/DefinitionsTests/DefinitionsTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class RegionTester
      \brief Tests the Region class.
   */
  class RegionTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Test a Market Region that's a subset of a Country Region.
      void TestMarketRegionSubsetOfCountryRegion();

      //! Test a Security Region that's a subset of a Market Region.
      void TestSecurityRegionSubsetOfMarketRegion();

      //! Test a Security Region that's a subset of a Country Region.
      void TestSecurityRegionSubsetOfCountryRegion();

      //! Test distinct Country Regions.
      void TestDistinctCountryRegions();

    private:
      CPPUNIT_TEST_SUITE(RegionTester);
        CPPUNIT_TEST(TestMarketRegionSubsetOfCountryRegion);
        CPPUNIT_TEST(TestSecurityRegionSubsetOfMarketRegion);
        CPPUNIT_TEST(TestSecurityRegionSubsetOfCountryRegion);
        CPPUNIT_TEST(TestDistinctCountryRegions);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
