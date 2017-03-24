#ifndef NEXUS_MARKETDATAREGISTRYSERVLETTESTER_HPP
#define NEXUS_MARKETDATAREGISTRYSERVLETTESTER_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Pointers/NativePointerPolicy.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTests.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Tests {

  /*  \class MarketDataRegistryServletTester
      \brief Tests the MarketDataRegistryServlet class.
   */
  class MarketDataRegistryServletTester :
      public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServerConnection.
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServiceProtocolServer =
        Beam::Services::ServiceProtocolServer<ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      //! The type of ServiceLocatorClient.
      using TestServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      //! The type of ServiceProtocolServer.
      using ServletContainer = Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
          MetaMarketDataRegistryServlet<MarketDataRegistry*,
          LocalHistoricalDataStore, TestServiceLocatorClient>,
          TestServiceLocatorClient*, Beam::NativePointerPolicy>,
        ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>,
        Beam::NativePointerPolicy>;

      //! The type of ServiceProtocol on the client side.
      using ClientServiceProtocolClient =
        Beam::Services::ServiceProtocolClient<
        Beam::Services::MessageProtocol<ClientChannel,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      virtual void setUp();

      virtual void tearDown();

      void TestMarketAndSourceEntitlement();

    private:
      Beam::DelayPtr<Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment>
        m_serviceLocatorEnvironment;
      std::unique_ptr<TestServiceLocatorClient> m_servletServiceLocatorClient;
      std::unique_ptr<TestServiceLocatorClient> m_clientServiceLocatorClient;
      Beam::DelayPtr<EntitlementDatabase> m_entitlements;
      Beam::DelayPtr<ServerConnection> m_serverConnection;
      Beam::DelayPtr<MarketDataRegistry> m_registry;
      Beam::DelayPtr<ServletContainer::Servlet::Servlet> m_registryServlet;
      Beam::DelayPtr<ServletContainer::Servlet> m_servlet;
      Beam::DelayPtr<ServletContainer> m_container;
      Beam::DelayPtr<ClientServiceProtocolClient> m_clientProtocol;

      CPPUNIT_TEST_SUITE(MarketDataRegistryServletTester);
        CPPUNIT_TEST(TestMarketAndSourceEntitlement);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
