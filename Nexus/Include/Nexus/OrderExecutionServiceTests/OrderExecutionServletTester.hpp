#ifndef NEXUS_ORDEREXECUTIONSERVLETTESTER_HPP
#define NEXUS_ORDEREXECUTIONSERVLETTESTER_HPP
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
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestInstance.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestInstance.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServlet.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Tests {

  /*! \class OrderExecutionServletTester
      \brief Tests the OrderExecutionServlet class.
   */
  class OrderExecutionServletTester: public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceLocatorClient used.
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      //! The type of UidClient used.
      using UidClient = Beam::UidService::VirtualUidClient;

      //! The type of AdministrationClient used.
      using AdministrationClient =
        AdministrationService::VirtualAdministrationClient;

      //! The type of ServerConnection.
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServletContainer = Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
          MetaOrderExecutionServlet<Beam::TimeService::IncrementalTimeClient,
          ServiceLocatorClient*, std::unique_ptr<UidClient>,
          std::unique_ptr<AdministrationClient>, MockOrderExecutionDriver*,
          LocalOrderExecutionDataStore*>, ServiceLocatorClient*,
          Beam::NativePointerPolicy>, ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      //! The type of ServiceProtocol on the client side.
      using ClientServiceProtocolClient = Beam::Services::ServiceProtocolClient<
        Beam::Services::MessageProtocol<ClientChannel,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests a NewOrderSingleMessage.
      void TestNewOrderSingle();

    private:
      Beam::DelayPtr<Beam::ServiceLocator::Tests::ServiceLocatorTestInstance>
        m_serviceLocatorInstance;
      Beam::DelayPtr<Beam::UidService::Tests::UidServiceTestInstance>
        m_uidServiceInstance;
      Beam::DelayPtr<
        AdministrationService::Tests::AdministrationServiceTestInstance>
        m_administrationServiceInstance;
      std::unique_ptr<ServiceLocatorClient> m_servletServiceLocatorClient;
      std::unique_ptr<ServiceLocatorClient> m_clientServiceLocatorClient;
      Beam::DelayPtr<ServerConnection> m_serverConnection;
      Beam::DelayPtr<MockOrderExecutionDriver> m_driver;
      Beam::DelayPtr<LocalOrderExecutionDataStore> m_dataStore;
      Beam::DelayPtr<ServletContainer::Servlet::Servlet> m_servlet;
      Beam::DelayPtr<ServletContainer> m_container;
      Beam::DelayPtr<ClientServiceProtocolClient> m_clientProtocol;

      CPPUNIT_TEST_SUITE(OrderExecutionServletTester);
        CPPUNIT_TEST(TestNewOrderSingle);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
