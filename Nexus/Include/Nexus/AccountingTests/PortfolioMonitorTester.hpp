#ifndef NEXUS_PORTFOLIOMONITORTESTER_HPP
#define NEXUS_PORTFOLIOMONITORTESTER_HPP
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/AccountingTests/AccountingTests.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestInstance.hpp"

namespace Nexus {
namespace Accounting {
namespace Tests {

  /*  \class PortfolioMonitorTester
      \brief Tests the PortfolioMonitor class.
   */
  class PortfolioMonitorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceLocatorClient.
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      //! The type of MarketDataClient.
      using MarketDataClient = MarketDataService::VirtualMarketDataClient;

      //! The type of PortfolioMonitor to test.
      typedef Accounting::PortfolioMonitor<Portfolio<TrueAverageBookkeeper<
        Inventory<Position<Security>>>>, std::unique_ptr<MarketDataClient>>
        PortfolioMonitor;

      virtual void setUp();

      virtual void tearDown();

      //! Tests handling ExecutionReports out of order.
      void TestOutOfOrderExecutionReports();

    private:
      Beam::DelayPtr<Beam::ServiceLocator::Tests::ServiceLocatorTestInstance>
        m_serviceLocatorInstance;
      std::unique_ptr<ServiceLocatorClient> m_serviceLocatorClient;
      Beam::DelayPtr<MarketDataService::Tests::MarketDataServiceTestInstance>
        m_marketDataServiceInstance;

      CPPUNIT_TEST_SUITE(PortfolioMonitorTester);
        CPPUNIT_TEST(TestOutOfOrderExecutionReports);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
