#include <Beam/Queues/Queue.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/ServiceAdministrationClient.hpp"
#include "Nexus/AdministrationServiceTests/TestAdministrationClient.hpp"

using namespace Beam;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;

namespace {
  using OperationQueue =
    Queue<std::shared_ptr<TestAdministrationClient::Operation>>;
  using TestServiceAdministrationClient =
    ServiceAdministrationClient<TestServiceProtocolClientBuilder>;

  struct Fixture {
    std::shared_ptr<TestServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    std::shared_ptr<OperationQueue> m_operations;

    Fixture()
      : m_server_connection(std::make_shared<TestServerConnection>()),
        m_server(m_server_connection,
          factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot()),
        m_operations(std::make_shared<OperationQueue>()) {
      RegisterAdministrationServices(Store(m_server.GetSlots()));
      RegisterAdministrationMessages(Store(m_server.GetSlots()));
    }

    template<typename T>
    using Request =
      RequestToken<TestServiceProtocolServer::ServiceProtocolClient, T>;

    std::unique_ptr<TestServiceAdministrationClient> make_client() {
      auto builder = TestServiceProtocolClientBuilder([&] {
        return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection);
      }, factory<std::unique_ptr<
        TestServiceProtocolClientBuilder::Timer>>());
      return std::make_unique<TestServiceAdministrationClient>(builder);
    }
  };
}

TEST_SUITE("ServiceAdministrationClient") {
}
