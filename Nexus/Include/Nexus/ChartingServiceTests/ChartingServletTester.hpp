#ifndef NEXUS_CHARTINGSERVLETTESTER_HPP
#define NEXUS_CHARTINGSERVLETTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/optional/optional.hpp>
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

      //! The type of ServerConnection.
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServletContainer = Beam::Services::ServiceProtocolServletContainer<
        MetaChartingServlet<MarketDataService::VirtualMarketDataClient*>,
        std::shared_ptr<ServerConnection>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      //! The type of ServiceProtocol on the client side.
      using ClientServiceProtocolClient =
        Beam::Services::ServiceProtocolClient<Beam::Services::MessageProtocol<
        ClientChannel,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests loading a time/price series for a Security.
      void TestLoadSecurityTimePriceSeries();

    private:
      boost::optional<TestEnvironment> m_environment;
      boost::optional<TestServiceClients> m_serviceClients;
      std::shared_ptr<ServerConnection> m_serverConnection;
      boost::optional<ServletContainer> m_container;
      boost::optional<ClientServiceProtocolClient> m_clientProtocol;

      CPPUNIT_TEST_SUITE(ChartingServletTester);
        CPPUNIT_TEST(TestLoadSecurityTimePriceSeries);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
