#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/TelemetryService/LocalTelemetryDataStore.hpp"
#include "Nexus/TelemetryService/TelemetryServlet.hpp"

using namespace Beam;
using namespace Beam::Queries;
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

    Client MakeClient(std::string name, bool isAdministrator = true) {
      auto account = m_serviceLocatorEnvironment.GetRoot().MakeAccount(
        name, "1234", DirectoryEntry::GetStarDirectory());
      if(isAdministrator) {
        m_administrationEnvironment.MakeAdministrator(account);
      }
      auto client =
        Client(m_serviceLocatorEnvironment.MakeClient(name, "1234"));
      client.m_telemetryClient = std::make_unique<TestServiceProtocolClient>(
        Initialize("test", *m_serverConnection), Initialize());
      Nexus::Queries::RegisterQueryTypes(
        Store(client.m_telemetryClient->GetSlots().GetRegistry()));
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
    auto account = DirectoryEntry::MakeAccount(221, "user");
    m_dataStore.Store(SequencedValue(IndexedValue(TelemetryEvent("abcd",
      "spire.blotter.size", time_from_string("2021-10-23 13:01:12"), {}),
      account), Beam::Queries::Sequence(111)));
    m_dataStore.Store(SequencedValue(IndexedValue(TelemetryEvent("efgh",
      "spire.book_view.size", time_from_string("2021-10-23 13:01:13"), {}),
      account), Beam::Queries::Sequence(112)));
    auto adminClient = MakeClient("simba");
    auto query = AccountQuery();
    query.SetIndex(account);
    query.SetRange(Range::Historical());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto adminSnapshot = adminClient.m_telemetryClient->SendRequest<
      QueryAccountTelemetryService>(query);
    REQUIRE(adminSnapshot.m_snapshot.size() == 2);
    auto userClient = MakeClient("simba2", false);
    auto userSnapshot = userClient.m_telemetryClient->SendRequest<
      QueryAccountTelemetryService>(query);
    REQUIRE(userSnapshot.m_snapshot.empty());
  }
}
