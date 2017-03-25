#ifndef NEXUS_BOARDLOTCHECKTESTER_HPP
#define NEXUS_BOARDLOTCHECKTESTER_HPP
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionService/BoardLotCheck.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Tests {

  /*! \class BoardLotCheckTester
      \brief Tests the BoardLotCheck class.
   */
  class BoardLotCheckTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceLocatorClient.
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      //! The type of MarketDataClient.
      using MarketDataClient = MarketDataService::VirtualMarketDataClient;

      //! The type of BoardLotCheck to test.
      using BoardLotCheck = OrderExecutionService::BoardLotCheck<
        std::unique_ptr<MarketDataClient>>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests no BboQuote available.
      void TestUnavailableBboQuote();

    private:
      boost::optional<
        Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment>
        m_serviceLocatorEnvironment;
      boost::optional<
        MarketDataService::Tests::MarketDataServiceTestEnvironment>
        m_marketDataServiceEnvironment;
      std::unique_ptr<ServiceLocatorClient> m_serviceLocatorClient;
      boost::optional<BoardLotCheck> m_check;

      CPPUNIT_TEST_SUITE(BoardLotCheckTester);
        CPPUNIT_TEST(TestUnavailableBboQuote);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
