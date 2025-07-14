#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;

namespace {
  struct Fixture {
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaMarketDataRegistryServlet<MarketDataRegistry*,
        LocalHistoricalDataStore*, AdministrationClient>>;
    FixedTimeClient m_time_client;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    optional<ServiceLocatorClientBox> m_servlet_service_locator_client;
    optional<AdministrationClient> m_servlet_administration_client;
    MarketDataRegistry m_registry;
    LocalHistoricalDataStore m_data_store;
    std::shared_ptr<TestServerConnection> m_server_connection;
    optional<ServletContainer> m_container;
    DirectoryEntry m_client_account;
    std::unique_ptr<TestServiceProtocolClient> m_client;

    Fixture()
        : m_time_client(time_from_string("2024-07-04 12:00:00")),
          m_server_connection(std::make_shared<TestServerConnection>()),
          m_administration_environment(
            make_administration_service_test_environment(
              m_service_locator_environment)),
          m_registry(DEFAULT_VENUES, get_default_time_zone_database()) {
      auto servlet_account = make_account(
        "market_data_service", DirectoryEntry::GetStarDirectory());
      m_service_locator_environment.GetRoot().StorePermissions(
        servlet_account, DirectoryEntry::GetStarDirectory(), Permissions(~0));
      m_servlet_service_locator_client =
        m_service_locator_environment.MakeClient(servlet_account.m_name, "");
      m_servlet_administration_client =
        m_administration_environment.make_client(
          *m_servlet_service_locator_client);
      m_container.emplace(Initialize(*m_servlet_service_locator_client,
        Initialize(*m_servlet_administration_client, &m_registry,
          &m_data_store)), m_server_connection,
        factory<std::unique_ptr<TriggerTimer>>());
      make_account("client", DirectoryEntry::GetStarDirectory());
      std::tie(m_client_account, m_client) = make_client("client");
    }

    auto make_account(const std::string& name, const DirectoryEntry& parent) {
      return m_service_locator_environment.GetRoot().MakeAccount(
        name, "", parent);
    }

    auto make_client(const std::string& name) {
      auto service_locator_client =
        m_service_locator_environment.MakeClient(name, "");
      auto authenticator = SessionAuthenticator(service_locator_client);
      auto protocol_client = std::make_unique<TestServiceProtocolClient>(
        Initialize(name, *m_server_connection), Initialize());
      RegisterMarketDataRegistryServices(Store(protocol_client->GetSlots()));
      RegisterMarketDataRegistryMessages(Store(protocol_client->GetSlots()));
      authenticator(*protocol_client);
      return std::tuple(
        service_locator_client.GetAccount(), std::move(protocol_client));
    }
  };
}

TEST_SUITE("MarketDataRegistryServlet") {
  TEST_CASE("query_security_info") {
    auto fixture = Fixture();
    auto security = Security("A", TSX);
    auto info = SecurityInfo(security, "SECURITY A", "", 100);
    fixture.m_registry.add(info);
    auto query = SecurityInfoQuery();
    query.SetIndex(security);
    auto result =
      fixture.m_client->SendRequest<QuerySecurityInfoService>(query);
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == info);
  }
}
