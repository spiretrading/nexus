#ifndef NEXUS_PORTFOLIOMONITORTESTER_HPP
#define NEXUS_PORTFOLIOMONITORTESTER_HPP
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/AccountingTests/AccountingTests.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

namespace Nexus {
namespace Accounting {
namespace Tests {

  /*  \class PortfolioMonitorTester
      \brief Tests the PortfolioMonitor class.
   */
  class PortfolioMonitorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of PortfolioMonitor to test.
      using PortfolioMonitor = Accounting::PortfolioMonitor<Portfolio<
        TrueAverageBookkeeper<Inventory<Position<Security>>>>,
        MarketDataService::VirtualMarketDataClient*>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests handling ExecutionReports out of order.
      void TestOutOfOrderExecutionReports();

    private:
      boost::optional<TestEnvironment> m_environment;
      boost::optional<TestServiceClients> m_serviceClients;

      CPPUNIT_TEST_SUITE(PortfolioMonitorTester);
        CPPUNIT_TEST(TestOutOfOrderExecutionReports);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
