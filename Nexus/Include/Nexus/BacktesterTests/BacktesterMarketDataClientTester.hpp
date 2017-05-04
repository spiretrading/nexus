#ifndef NEXUS_BACKTESTERMARKETDATACLIENTTESTER_HPP
#define NEXUS_BACKTESTERMARKETDATACLIENTTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterMarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus {
namespace Tests {

  /*! \class BacktesterMarketDataClientTester
      \brief Tests the BacktesterMarketDataClient class.
   */
  class BacktesterMarketDataClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of BacktesterEnvironment used for testing.
      using BacktesterEnvironment = ::Nexus::BacktesterEnvironment<
        MarketDataService::VirtualMarketDataClient*>;

      //! Tests submitting a query for real time data.
      void TestRealTimeQuery();

    private:
      CPPUNIT_TEST_SUITE(BacktesterMarketDataClientTester);
        CPPUNIT_TEST(TestRealTimeQuery);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}

#endif
