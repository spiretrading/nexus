#ifndef NEXUS_CHARTINGCLIENTTESTER_HPP
#define NEXUS_CHARTINGCLIENTTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolServer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
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

      //! The type of ServerConnection.
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServiceProtocolServer = Beam::Services::ServiceProtocolServer<
        ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      //! The type used to build sessions.
      using ServiceProtocolClientBuilder =
        Beam::Services::ServiceProtocolClientBuilder<
        Beam::Services::MessageProtocol<std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      //! The type of ChartingClient.
      using TestChartingClient = ChartingClient<ServiceProtocolClientBuilder>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests loading a Security's price/time series.
      void TestLoadSecurityPriceTimeSeries();

    private:
      Beam::DelayPtr<ServerConnection> m_serverConnection;
      Beam::DelayPtr<ServiceProtocolServer> m_server;
      Beam::DelayPtr<TestChartingClient> m_client;

      CPPUNIT_TEST_SUITE(ChartingClientTester);
        CPPUNIT_TEST(TestLoadSecurityPriceTimeSeries);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
