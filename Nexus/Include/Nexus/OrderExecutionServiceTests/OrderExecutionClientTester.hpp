#ifndef NEXUS_ORDEREXECUTIONCLIENTTESTER_HPP
#define NEXUS_ORDEREXECUTIONCLIENTTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolServer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Tests {

  /*! \class OrderExecutionClientTester
      \brief Tests the OrderExecutionClient class.
   */
  class OrderExecutionClientTester : public CPPUNIT_NS::TestFixture {
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

      //! The type used to build sessions.
      using ServiceProtocolClientBuilder =
        Beam::Services::ServiceProtocolClientBuilder<
        Beam::Services::MessageProtocol<std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      //! The type of OrderExecutionClient.
      using TestOrderExecutionClient =
        OrderExecutionClient<ServiceProtocolClientBuilder>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests submitting an Order.
      void TestSubmitOrder();

    private:
      boost::optional<ServerConnection> m_serverConnection;
      boost::optional<ServiceProtocolServer> m_server;
      boost::optional<TestOrderExecutionClient> m_client;

      CPPUNIT_TEST_SUITE(OrderExecutionClientTester);
        CPPUNIT_TEST(TestSubmitOrder);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
