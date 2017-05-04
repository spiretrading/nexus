#ifndef NEXUS_BACKTESTERMARKETDATACLIENTTESTER_HPP
#define NEXUS_BACKTESTERMARKETDATACLIENTTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/BacktesterTests/BacktesterTests.hpp"

namespace Nexus {
namespace Tests {

  /*! \class BacktesterMarketDataClientTester
      \brief Tests the BacktesterMarketDataClient class.
   */
  class BacktesterMarketDataClientTester : public CPPUNIT_NS::TestFixture {
    public:

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
