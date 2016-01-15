#ifndef NEXUS_MARKETDATAFEEDCLIENTTESTER_HPP
#define NEXUS_MARKETDATAFEEDCLIENTTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Tests {

  /*! \class MarketDataFeedClientTester
      \brief Tests the MarketDataFeedClient class.
   */
  class MarketDataFeedClientTester : public CPPUNIT_NS::TestFixture {
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

      //! The type of MarketDataFeedClient.
      using TestMarketDataFeedClient = MarketDataFeedClient<std::string,
        Beam::Threading::TriggerTimer*,
        Beam::Services::MessageProtocol<ClientChannel,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests setting the BBO.
      void TestSettingBbo();

      //! Tests publishing a market's order imbalance.
      void TestPublishMarketOrderImbalance();

    private:
      Beam::DelayPtr<ServerConnection> m_serverConnection;
      Beam::DelayPtr<ServiceProtocolServer> m_server;
      Beam::DelayPtr<Beam::Threading::TriggerTimer> m_samplingTimer;
      Beam::DelayPtr<TestMarketDataFeedClient> m_client;

      CPPUNIT_TEST_SUITE(MarketDataFeedClientTester);
        CPPUNIT_TEST(TestSettingBbo);
        CPPUNIT_TEST(TestPublishMarketOrderImbalance);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
