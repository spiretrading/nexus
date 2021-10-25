#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/TelemetryService/LocalTelemetryDataStore.hpp"
#include "Nexus/TelemetryService/TelemetryServlet.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::TelemetryService;

namespace {
  using TestServletContainer = TestAuthenticatedServiceProtocolServletContainer<
    MetaTelemetryServlet<std::shared_ptr<FixedTimeClient>,
      AdministrationClientBox, LocalTelemetryDataStore*>>;

  struct Client {
    ServiceLocatorClientBox m_serviceLocatorClient;
    std::unique_ptr<TestServiceProtocolClient> m_telemetryClient;

    Client(ServiceLocatorClientBox serviceLocatorClient)
      : m_serviceLocatorClient(std::move(serviceLocatorClient)) {}
  };

  struct Fixture {
    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    AdministrationServiceTestEnvironment m_administrationEnvironment;
    LocalTelemetryDataStore m_dataStore;
    std::shared_ptr<TestServerConnection> m_serverConnection;
    optional<TestServletContainer> m_container;

    Fixture()
        : m_administrationEnvironment(MakeAdministrationServiceTestEnvironment(
            m_serviceLocatorEnvironment)),
          m_serverConnection(std::make_shared<TestServerConnection>()) {
      m_administrationEnvironment.MakeAdministrator(
        m_serviceLocatorEnvironment.GetRoot().GetAccount());
      m_container.emplace(Initialize(m_serviceLocatorEnvironment.GetRoot(),
        Initialize(std::make_shared<FixedTimeClient>(
          time_from_string("2021-10-23 13:01:12")),
          m_administrationEnvironment.GetClient(), &m_dataStore)),
        m_serverConnection, factory<std::unique_ptr<TriggerTimer>>());
    }

    Client MakeClient(std::string name) {
      auto account = m_serviceLocatorEnvironment.GetRoot().MakeAccount(
        name, "1234", DirectoryEntry::GetStarDirectory());
      auto client =
        Client(m_serviceLocatorEnvironment.MakeClient(name, "1234"));
      client.m_telemetryClient = std::make_unique<TestServiceProtocolClient>(
        Initialize("test", *m_serverConnection), Initialize());
      RegisterTelemetryServices(Store(client.m_telemetryClient->GetSlots()));
      RegisterTelemetryMessages(Store(client.m_telemetryClient->GetSlots()));
      auto authenticator = SessionAuthenticator(client.m_serviceLocatorClient);
      authenticator(*client.m_telemetryClient);
      return client;
    }
  };
}

TEST_SUITE("TelemetryServlet") {
  TEST_CASE_FIXTURE(Fixture, "query_events") {
    auto client = MakeClient("simba");
    m_administrationEnvironment.MakeAdministrator(
      client.m_serviceLocatorClient.GetAccount());
  }
}
