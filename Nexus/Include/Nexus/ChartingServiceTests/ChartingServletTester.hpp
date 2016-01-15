#ifndef NEXUS_CHARTINGSERVLETTESTER_HPP
#define NEXUS_CHARTINGSERVLETTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestInstance.hpp"
#include "Nexus/ChartingService/ChartingServlet.hpp"
#include "Nexus/ChartingServiceTests/ChartingServiceTests.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestInstance.hpp"

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
        MetaChartingServlet<std::unique_ptr<MarketDataService::Tests::
        MarketDataServiceTestInstance::MarketDataClient>>,
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
      Beam::DelayPtr<Beam::ServiceLocator::Tests::ServiceLocatorTestInstance>
        m_serviceLocatorInstance;
      Beam::DelayPtr<
        AdministrationService::Tests::AdministrationServiceTestInstance>
        m_administrationInstance;
      std::unique_ptr<Beam::ServiceLocator::Tests::ServiceLocatorTestInstance::
        ServiceLocatorClient> m_serviceLocatorClient;
      Beam::DelayPtr<MarketDataService::Tests::MarketDataServiceTestInstance>
        m_marketDataServiceInstance;
      std::shared_ptr<ServerConnection> m_serverConnection;
      Beam::DelayPtr<ServletContainer> m_container;
      Beam::DelayPtr<ClientServiceProtocolClient> m_clientProtocol;

      CPPUNIT_TEST_SUITE(ChartingServletTester);
        CPPUNIT_TEST(TestLoadSecurityTimePriceSeries);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
