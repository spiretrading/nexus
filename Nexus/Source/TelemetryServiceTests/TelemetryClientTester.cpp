#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/TelemetryService/TelemetryClient.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace Nexus;
using namespace Nexus::TelemetryService;

namespace {
  struct Fixture {
    using TestTelemetryClient =
      TelemetryClient<TestServiceProtocolClientBuilder, TimeClientBox>;
    optional<TestServiceProtocolServer> m_server;
    optional<TestTelemetryClient> m_client;

    Fixture() {
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_server.emplace(serverConnection,
        factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
      RegisterTelemetryServices(Store(m_server->GetSlots()));
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
            "test", *serverConnection);
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_client.emplace(builder, std::make_unique<LocalTimeClient>());
    }
  };
}

TEST_SUITE("TelemetryClient") {
  TEST_CASE_FIXTURE(Fixture, "store") {
  }
}
