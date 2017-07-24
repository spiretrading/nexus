#ifndef NEXUS_MARKETDATAFEEDCLIENTTESTER_HPP
#define NEXUS_MARKETDATAFEEDCLIENTTESTER_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServer.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Tests {

  /*! \class MarketDataFeedClientTester
      \brief Tests the MarketDataFeedClient class.
   */
  class MarketDataFeedClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceProtocolServer.
      using ServiceProtocolServer =
        Beam::Services::ServiceProtocolServer<
        std::shared_ptr<Beam::Services::Tests::TestServerConnection>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::unique_ptr<Beam::Threading::TriggerTimer>>;

      //! The type of MarketDataFeedClient.
      using TestMarketDataFeedClient = MarketDataFeedClient<std::string,
        Beam::Threading::TriggerTimer*, Beam::Services::MessageProtocol<
        Beam::Services::Tests::TestClientChannel,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests setting the BBO.
      void TestSettingBbo();

      //! Tests publishing a market's order imbalance.
      void TestPublishMarketOrderImbalance();

    private:
      boost::optional<ServiceProtocolServer> m_server;
      boost::optional<Beam::Threading::TriggerTimer> m_samplingTimer;
      boost::optional<TestMarketDataFeedClient> m_client;

      CPPUNIT_TEST_SUITE(MarketDataFeedClientTester);
        CPPUNIT_TEST(TestSettingBbo);
        CPPUNIT_TEST(TestPublishMarketOrderImbalance);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
