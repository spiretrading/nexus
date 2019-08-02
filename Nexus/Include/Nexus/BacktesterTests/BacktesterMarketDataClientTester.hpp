#ifndef NEXUS_BACKTESTER_MARKET_DATA_CLIENT_TESTER_HPP
#define NEXUS_BACKTESTER_MARKET_DATA_CLIENT_TESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/BacktesterTests/BacktesterTests.hpp"

namespace Nexus::Tests {

  /** Tests the BacktesterMarketDataClient class. */
  class BacktesterMarketDataClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests submitting a query for realtime data.
      void TestRealTimeQuery();

      //! Tests submitting a query for history market data from before the
      //! backtester start's time.
      void TestHistoricalQuery();

    private:
      CPPUNIT_TEST_SUITE(BacktesterMarketDataClientTester);
        CPPUNIT_TEST(TestRealTimeQuery);
        CPPUNIT_TEST(TestHistoricalQuery);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}

#endif
