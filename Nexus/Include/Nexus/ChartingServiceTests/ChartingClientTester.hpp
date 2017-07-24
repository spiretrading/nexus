#ifndef NEXUS_CHARTINGCLIENTTESTER_HPP
#define NEXUS_CHARTINGCLIENTTESTER_HPP
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/ChartingService/ChartingClient.hpp"
#include "Nexus/ChartingServiceTests/ChartingServiceTests.hpp"

namespace Nexus {
namespace ChartingService {
namespace Tests {

  /*! \class ChartingClientTester
      \brief Tests the ChartingClient class.
   */
  class ChartingClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ChartingClient.
      using TestChartingClient =
        ChartingClient<Beam::Services::Tests::TestServiceProtocolClientBuilder>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests loading a Security's price/time series.
      void TestLoadSecurityPriceTimeSeries();

    private:
      boost::optional<Beam::Services::Tests::TestServiceProtocolServer>
        m_server;
      boost::optional<TestChartingClient> m_client;

      CPPUNIT_TEST_SUITE(ChartingClientTester);
        CPPUNIT_TEST(TestLoadSecurityPriceTimeSeries);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
