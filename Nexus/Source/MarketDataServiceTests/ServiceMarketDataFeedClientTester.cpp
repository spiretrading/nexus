#include <memory>
#include <Beam/Codecs/NullEncoder.hpp>
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/ServiceMarketDataFeedClient.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;

namespace {
  struct Fixture {
    using TestServiceMarketDataFeedClient =
      ServiceMarketDataFeedClient<std::string, std::shared_ptr<TriggerTimer>,
      MessageProtocol<std::unique_ptr<TestClientChannel>,
      BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
    std::shared_ptr<TestServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    std::unique_ptr<TestServiceMarketDataFeedClient> m_client;
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_handlers;

    Fixture()
        : m_server_connection(std::make_shared<TestServerConnection>()),
          m_server(m_server_connection,
            factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot()) {
      RegisterMarketDataFeedMessages(Store(m_server.GetSlots()));
/*
      auto builder = TestServiceProtocolClientBuilder([=] {
        return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection);
      }, factory<std::unique_ptr<
        TestServiceProtocolClientBuilder::Timer>>());
*/
//      m_client = std::make_unique<TestServiceMarketDataFeedClient>(builder);
    }
  };
}

TEST_SUITE("ServiceMarketDataFeedClient") {
  TEST_CASE("foo") {
  }
}
