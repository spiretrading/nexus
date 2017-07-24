#ifndef NEXUS_CHARTINGSERVLETTESTER_HPP
#define NEXUS_CHARTINGSERVLETTESTER_HPP
#include <Beam/ServicesTests/TestServices.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/ChartingService/ChartingServlet.hpp"
#include "Nexus/ChartingServiceTests/ChartingServiceTests.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

namespace Nexus {
namespace ChartingService {
namespace Tests {

  /*! \class ChartingServletTester
      \brief Tests the ChartingServlet class.
   */
  class ChartingServletTester: public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceProtocolServer.
      using ServletContainer =
        Beam::Services::Tests::TestServiceProtocolServletContainer<
        MetaChartingServlet<MarketDataService::VirtualMarketDataClient*>>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests loading a time/price series for a Security.
      void TestLoadSecurityTimePriceSeries();

    private:
      boost::optional<TestEnvironment> m_environment;
      boost::optional<TestServiceClients> m_serviceClients;
      boost::optional<ServletContainer> m_container;
      boost::optional<Beam::Services::Tests::TestServiceProtocolClient>
        m_clientProtocol;

      CPPUNIT_TEST_SUITE(ChartingServletTester);
        CPPUNIT_TEST(TestLoadSecurityTimePriceSeries);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
