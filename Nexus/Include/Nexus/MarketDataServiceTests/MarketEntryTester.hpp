#ifndef NEXUS_MARKETENTRYTESTER_HPP
#define NEXUS_MARKETENTRYTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/MarketDataServiceTests/MarketDataServiceTests.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Tests {

  /*  \class MarketEntryTester
      \brief Tests the MarketEntry class.
   */
  class MarketEntryTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests publishing an OrderImbalance.
      void TestPublishOrderImbalance();

    private:
      CPPUNIT_TEST_SUITE(MarketEntryTester);
        CPPUNIT_TEST(TestPublishOrderImbalance);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
