#ifndef NEXUS_REGIONMAPTESTER_HPP
#define NEXUS_REGIONMAPTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/DefinitionsTests/DefinitionsTests.hpp"

namespace Nexus {
namespace Tests {

  /*  \class RegionMapTester
      \brief Tests the RegionMap class.
   */
  class RegionMapTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Test a Market Region that's a subset of a Country Region.
      void TestMarketRegionSubsetOfCountryRegion();

      //! Test setting a Country, then a Security, then a Market.
      void TestSetCountrySecurityMarket();

      //! Test iterating over the RegionMap.
      void TestRegionMapIterator();

    private:
      CPPUNIT_TEST_SUITE(RegionMapTester);
        CPPUNIT_TEST(TestMarketRegionSubsetOfCountryRegion);
        CPPUNIT_TEST(TestSetCountrySecurityMarket);
        CPPUNIT_TEST(TestRegionMapIterator);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
