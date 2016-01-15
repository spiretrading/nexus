#ifndef NEXUS_MARKETDATAREGISTRYTESTER_HPP
#define NEXUS_MARKETDATAREGISTRYTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/MarketDataServiceTests/MarketDataServiceTests.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Tests {

  /*  \class MarketDataRegistryTester
      \brief Tests the MarketDataRegistry class.
   */
  class MarketDataRegistryTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests publishing a BboQuote.
      void TestPublishBboQuote();

    private:
      CPPUNIT_TEST_SUITE(MarketDataRegistryTester);
        CPPUNIT_TEST(TestPublishBboQuote);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
