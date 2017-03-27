#ifndef NEXUS_BOARDLOTCHECKTESTER_HPP
#define NEXUS_BOARDLOTCHECKTESTER_HPP
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/OrderExecutionService/BoardLotCheck.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Tests {

  /*! \class BoardLotCheckTester
      \brief Tests the BoardLotCheck class.
   */
  class BoardLotCheckTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of BoardLotCheck to test.
      using BoardLotCheck = OrderExecutionService::BoardLotCheck<
        MarketDataService::VirtualMarketDataClient*>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests no BboQuote available.
      void TestUnavailableBboQuote();

    private:
      boost::optional<TestEnvironment> m_environment;
      boost::optional<TestServiceClients> m_serviceClients;
      boost::optional<BoardLotCheck> m_check;

      CPPUNIT_TEST_SUITE(BoardLotCheckTester);
        CPPUNIT_TEST(TestUnavailableBboQuote);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
